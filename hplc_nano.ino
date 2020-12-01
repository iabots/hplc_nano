#include <HX711_ADC.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define ECOOL 0
#define EDOORO 1
#define EDOORA 2
#define EDOORAOFF 3
#define EFULL 4
#define EFULLOFF 5
#define ETARE 6
#define ETARESUCC 7

#define PILOT 6
#define BUZZER 7
#define DOOR 9
#define TOGGLE 10

const unsigned long topen = 40000;
const unsigned long talarm = 500;
const unsigned long temerg = 250;
const unsigned long ttare = 3000;

unsigned int estado = ECOOL;

unsigned long tini = 0;
unsigned long tactual = 0;
unsigned long trel = 0;

const unsigned long trate = 20;
unsigned long tdisp = 0;

HX711_ADC LoadCell(3, 2);
LiquidCrystal_I2C lcd(0x27,20,4);

float i;
int val;
void setup() {
  // put your setup code here, to run once:
  pinMode(PILOT, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(DOOR, INPUT);
  pinMode(TOGGLE, INPUT);
  digitalWrite(PILOT,LOW);
  digitalWrite(BUZZER,LOW);

  Serial.begin(9600);
  Serial.println("Wait...");
  LoadCell.begin();
  long stabilisingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  LoadCell.setCalFactor(105.3); // user set calibration factor (float)
  Serial.println("Startup + tare is complete");

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Blacksmith!!!");
  delay(5000);
  lcd.clear();
}

void loop() {
  LoadCell.update();
  tactual = millis();
  
  if (millis() > tdisp + trate) {
    i = LoadCell.getData();
    val = int(i /100);
    //lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tanque: ");
    lcd.print(val);
    lcd.print("%   ");
    tdisp = millis();

    switch(estado){
        case ECOOL:
          lcd.setCursor(0,1);
          lcd.print("                ");
          break;
        case EDOORO:
          lcd.setCursor(0,1);
          lcd.print("Puerta abierta! ");
          break;
        case EDOORA:
          lcd.setCursor(0,1);
          lcd.print("Cerrar puerta!  ");
          break;
        case EDOORAOFF:
          lcd.setCursor(0,1);
          lcd.print("Cerrar puerta!  ");
          break;
        case EFULL:
          lcd.setCursor(0,1);
          lcd.print("Tanque lleno!   ");
          break;
        case EFULLOFF:
          lcd.setCursor(0,1);
          lcd.print("Tanque lleno!   ");
          break;
        case ETARE:
          lcd.setCursor(0,1);
          lcd.print("Tarando...      ");
          break;
        case ETARESUCC:
          lcd.setCursor(0,1);
          lcd.print("Tarado!         ");
          break;
      }
    }

  switch(estado){
      case ECOOL:
        digitalWrite(PILOT,LOW);
        digitalWrite(BUZZER,LOW);
        if(val >= 90){
          estado = EFULL;
          }
        else if(digitalRead(DOOR) == 0){
          estado = EDOORO;
          tini = millis();
          }
        else{
          estado = ECOOL;
          }
        break;
      case EDOORO:
        digitalWrite(PILOT,LOW);
        digitalWrite(BUZZER,LOW);

        trel = tactual - tini;

        if(val >= 90){
          estado = EFULL;
          }
        else if(digitalRead(DOOR) == 1){
          estado = ECOOL;
          }
        else if(trel >= topen){
          estado = EDOORA;
          tini = millis();
          }
        else if(digitalRead(TOGGLE)){
          estado = ETARE;
          tini = millis();
          }
        else{
          estado = EDOORO;
          }
        break;

      case EDOORA:
        digitalWrite(PILOT,HIGH);
        digitalWrite(BUZZER,HIGH);

        trel = tactual - tini;

        if(val >= 90){
          estado = EFULL;
          }
        else if(digitalRead(DOOR) == 1){
          estado = ECOOL;
          }
        else if(trel >= talarm){
          estado = EDOORAOFF;
          tini = millis();
          }
        break;

      case EDOORAOFF:
        digitalWrite(PILOT,LOW);
        digitalWrite(BUZZER,LOW);

        trel = tactual - tini;

        if(val >= 90){
          estado = EFULL;
          }
        else if(digitalRead(DOOR) == 1){
          estado = ECOOL;
          }
        else if(trel >= talarm){
          estado = EDOORA;
          tini = millis();
          }
        break;

      case EFULL:
        digitalWrite(PILOT,HIGH);
        digitalWrite(BUZZER,HIGH);

        trel = tactual - tini;

        if(val <= 50){
          estado = EDOORO;
          tini = millis();
          }
        else if(trel >= temerg){
            estado = EFULLOFF;
            tini = millis();
          }
        break;

      case EFULLOFF:
        digitalWrite(PILOT,LOW);
        digitalWrite(BUZZER,LOW);

        trel = tactual - tini;

        if(val <= 50){
          estado = EDOORO;
          tini = millis();
          }
        else if(trel >= temerg){
            estado = EFULL;
            tini = millis();
          }
        break;

      case ETARE:
        digitalWrite(PILOT,LOW);
        digitalWrite(BUZZER,LOW);

        trel = tactual - tini;
        
        if(val >= 90){
          estado = EFULL;
          }
        else if(digitalRead(DOOR) == 1){
            estado = ECOOL;
          }
        else if(digitalRead(TOGGLE) == 0){
            estado = EDOORO;
            tini = millis();
          }
        else if(trel >= ttare){
            estado = ETARESUCC;
            LoadCell.tareNoDelay();
            tini = millis();
          }
        break;
        
      case ETARESUCC:
        digitalWrite(PILOT,LOW);
        digitalWrite(BUZZER,LOW);
        
        if(val >= 90){
          estado = EFULL;
          }
        else if(digitalRead(DOOR) == 1){
            estado = ECOOL;
          }
        else if(digitalRead(TOGGLE) == 0){
            estado = EDOORO;
            tini = millis();
          }
        break;
    }

}
