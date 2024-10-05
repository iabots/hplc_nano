#include <HX711_ADC.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define COOL_STATE 0
#define DOOR_OPEN_STATE 1
#define DOOR_ALARM_ON_STATE 2
#define DOOR_ALARM_OFF_STATE 3
#define ALMOST_FULL_ON_STATE 4
#define ALMOST_FULL_OFF_STATE 5
#define FULL_ON_STATE 6
#define FULL_OFF_STATE 7
#define TARE_STATE 8
#define TARE_SUCCESS_STATE 9

#define PILOT 6
#define BUZZER 7
#define DOOR 9
#define TOGGLE 10

const unsigned long time_open = 40000;
const unsigned long time_alarm = 500;
const unsigned long time_emergency = 250;
const unsigned long time_tare = 3000;

unsigned int state = COOL_STATE;

unsigned long initial_time = 0;
unsigned long actual_time = 0;
unsigned long trel = 0;

const unsigned long time_refresh_rate = 20;
unsigned long display_time = 0;

const int FULL_LOAD_VALUE = 90;
const int ALMOST_FULL_LOAD_VALUE = 80;
const int MIDDLE_LOAD_VALUE = 50;

HX711_ADC LoadCell(3, 2);
LiquidCrystal_I2C lcd(0x27, 20, 4);

int val;
void setup() {
    // put your setup code here, to run once:
    pinMode(PILOT, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(DOOR, INPUT);
    pinMode(TOGGLE, INPUT);
    digitalWrite(PILOT, LOW);
    digitalWrite(BUZZER, LOW);

    Serial.begin(9600);
    Serial.println("Wait...");
    LoadCell.begin();
    // tare precision can be improved by adding a few seconds of stabilising time
    long stabilising_time = 2000;
    LoadCell.start(stabilising_time);
    LoadCell.setCalFactor(105.3);  // user set calibration factor (float)
    Serial.println("Startup + tare is complete");

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Blacksmith   ");
    delay(5000);
    lcd.clear();
}

void loop() {
    LoadCell.update();
    actual_time = millis();

    if (millis() > display_time + time_refresh_rate) {
        // Convert to percentage and adjust to 8kg instead of 10kg
        float i = (LoadCell.getData() / 100.0) * 1.25;
        val = int(i);
        // lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tanque: ");
        lcd.print(val);
        lcd.print("%   ");
        display_time = millis();

        switch (state) {
            case COOL_STATE:
                lcd.setCursor(0, 1);
                lcd.print("                ");
                break;
            case DOOR_OPEN_STATE:
                lcd.setCursor(0, 1);
                lcd.print("Puerta abierta! ");
                break;
            case DOOR_ALARM_ON_STATE:
                lcd.setCursor(0, 1);
                lcd.print("Cerrar puerta!  ");
                break;
            case DOOR_ALARM_OFF_STATE:
                lcd.setCursor(0, 1);
                lcd.print("Cerrar puerta!  ");
                break;
            case ALMOST_FULL_ON_STATE:
                lcd.setCursor(0, 1);
                lcd.print("Precaucion");
                break;
            case ALMOST_FULL_OFF_STATE:
                lcd.setCursor(0, 1);
                lcd.print("Precaucion      ");
                break;
            case FULL_ON_STATE:
                lcd.setCursor(0, 1);
                lcd.print("Tanque lleno!   ");
                break;
            case FULL_OFF_STATE:
                lcd.setCursor(0, 1);
                lcd.print("Tanque lleno!   ");
                break;
            case TARE_STATE:
                lcd.setCursor(0, 1);
                lcd.print("Tarando...      ");
                break;
            case TARE_SUCCESS_STATE:
                lcd.setCursor(0, 1);
                lcd.print("Tarado!         ");
                break;
        }
    }

    switch (state) {
        case COOL_STATE:
            digitalWrite(PILOT, LOW);
            digitalWrite(BUZZER, LOW);
            if (val >= FULL_LOAD_VALUE) {
                state = FULL_ON_STATE;
                initial_time = millis();
            } else if (val >= ALMOST_FULL_LOAD_VALUE && val < FULL_LOAD_VALUE) {
                state = ALMOST_FULL_ON_STATE;
                initial_time = millis();
            } else if (digitalRead(DOOR) == 0) {
                state = DOOR_OPEN_STATE;
                initial_time = millis();
            } else {
                state = COOL_STATE;
            }
            break;
        case DOOR_OPEN_STATE:
            digitalWrite(PILOT, LOW);
            digitalWrite(BUZZER, LOW);

            trel = actual_time - initial_time;

            if (val >= FULL_LOAD_VALUE) {
                state = FULL_ON_STATE;
                initial_time = millis();
            } else if (val >= ALMOST_FULL_LOAD_VALUE && val < FULL_LOAD_VALUE) {
                state = ALMOST_FULL_ON_STATE;
                initial_time = millis();
            } else if (digitalRead(DOOR) == 1) {
                state = COOL_STATE;
            } else if (trel >= time_open) {
                state = DOOR_ALARM_ON_STATE;
                initial_time = millis();
            } else if (digitalRead(TOGGLE)) {
                state = TARE_STATE;
                initial_time = millis();
            } else {
                state = DOOR_OPEN_STATE;
            }
            break;

        case DOOR_ALARM_ON_STATE:
            digitalWrite(PILOT, HIGH);
            digitalWrite(BUZZER, HIGH);

            trel = actual_time - initial_time;

            if (val >= FULL_LOAD_VALUE) {
                state = FULL_ON_STATE;
                initial_time = millis();
            } else if (val >= ALMOST_FULL_LOAD_VALUE && val < FULL_LOAD_VALUE) {
                state = ALMOST_FULL_ON_STATE;
                initial_time = millis();
            } else if (digitalRead(DOOR) == 1) {
                state = COOL_STATE;
            } else if (trel >= time_alarm) {
                state = DOOR_ALARM_OFF_STATE;
                initial_time = millis();
            }
            break;

        case DOOR_ALARM_OFF_STATE:
            digitalWrite(PILOT, LOW);
            digitalWrite(BUZZER, LOW);

            trel = actual_time - initial_time;

            if (val >= FULL_LOAD_VALUE) {
                state = FULL_ON_STATE;
                initial_time = millis();
            } else if (val >= ALMOST_FULL_LOAD_VALUE && val < FULL_LOAD_VALUE) {
                state = ALMOST_FULL_ON_STATE;
                initial_time = millis();
            } else if (digitalRead(DOOR) == 1) {
                state = COOL_STATE;
            } else if (trel >= time_alarm) {
                state = DOOR_ALARM_ON_STATE;
                initial_time = millis();
            }
            break;

        case ALMOST_FULL_ON_STATE:
            digitalWrite(PILOT, HIGH);
            digitalWrite(BUZZER, LOW);

            trel = actual_time - initial_time;

            if (val >= FULL_LOAD_VALUE) {
                state = FULL_ON_STATE;
                initial_time = millis();
            } else if (val <= MIDDLE_LOAD_VALUE) {
                state = DOOR_OPEN_STATE;
                initial_time = millis();
            } else if (trel >= time_emergency) {
                state = ALMOST_FULL_OFF_STATE;
                initial_time = millis();
            }
            break;

        case ALMOST_FULL_OFF_STATE:
            digitalWrite(PILOT, LOW);
            digitalWrite(BUZZER, LOW);

            trel = actual_time - initial_time;

            if (val >= FULL_LOAD_VALUE) {
                state = FULL_ON_STATE;
                initial_time = millis();
            } else if (val <= MIDDLE_LOAD_VALUE) {
                state = DOOR_OPEN_STATE;
                initial_time = millis();
            } else if (trel >= time_emergency) {
                state = ALMOST_FULL_ON_STATE;
                initial_time = millis();
            }
            break;

        case FULL_ON_STATE:
            digitalWrite(PILOT, HIGH);
            digitalWrite(BUZZER, HIGH);

            trel = actual_time - initial_time;

            if (val <= MIDDLE_LOAD_VALUE) {
                state = DOOR_OPEN_STATE;
                initial_time = millis();
            } else if (trel >= time_emergency) {
                state = FULL_OFF_STATE;
                initial_time = millis();
            }
            break;

        case FULL_OFF_STATE:
            digitalWrite(PILOT, LOW);
            digitalWrite(BUZZER, LOW);

            trel = actual_time - initial_time;

            if (val <= MIDDLE_LOAD_VALUE) {
                state = DOOR_OPEN_STATE;
                initial_time = millis();
            } else if (trel >= time_emergency) {
                state = FULL_ON_STATE;
                initial_time = millis();
            }
            break;

        case TARE_STATE:
            digitalWrite(PILOT, LOW);
            digitalWrite(BUZZER, LOW);

            trel = actual_time - initial_time;

            if (val >= FULL_LOAD_VALUE) {
                state = FULL_ON_STATE;
                initial_time = millis();
            } else if (val >= ALMOST_FULL_LOAD_VALUE && val < FULL_LOAD_VALUE) {
                state = ALMOST_FULL_ON_STATE;
                initial_time = millis();
            } else if (digitalRead(DOOR) == 1) {
                state = COOL_STATE;
            } else if (digitalRead(TOGGLE) == 0) {
                state = DOOR_OPEN_STATE;
                initial_time = millis();
            } else if (trel >= time_tare) {
                state = TARE_SUCCESS_STATE;
                LoadCell.tareNoDelay();
                initial_time = millis();
            }
            break;

        case TARE_SUCCESS_STATE:
            digitalWrite(PILOT, LOW);
            digitalWrite(BUZZER, LOW);

            if (val >= FULL_LOAD_VALUE) {
                state = FULL_ON_STATE;
                initial_time = millis();
            } else if (val >= ALMOST_FULL_LOAD_VALUE && val < FULL_LOAD_VALUE) {
                state = ALMOST_FULL_ON_STATE;
                initial_time = millis();
            } else if (digitalRead(DOOR) == 1) {
                state = COOL_STATE;
            } else if (digitalRead(TOGGLE) == 0) {
                state = DOOR_OPEN_STATE;
                initial_time = millis();
            }
            break;
    }
}
