#define BLYNK_PRINT SwSerial
#include <Wire.h>
#include <SoftwareSerial.h>
SoftwareSerial SwSerial(10, 11); // RX, TX

#include <BlynkSimpleStream.h>

char auth[] = "3abc00e4e9b24264beec6e24a1d62665";

BlynkTimer timer;

const int PB_PIN = 2;
const int TRANSIENT_PERIOD = 10;
boolean transientPeriodStarted = false;
boolean condition = false;
boolean bPressAccepted = false;
unsigned long timeRef = 0;


int gasPin1 = A0; 
int gasPin2 = A1; 

int BUZZER_PIN = 8;
int led = 13;

const byte MY_ADDRESS = 42; // Gas Master's address

void setup() {
  Wire.begin(MY_ADDRESS);
  Wire.onRequest(requestEvent);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(gasPin1, INPUT);
  pinMode(gasPin2, INPUT);
  digitalWrite(led, HIGH);

  pinMode(PB_PIN, INPUT_PULLUP);  // Enables the internal pull-up resistor

  timer.setInterval(500L, myTimerEvent);
  SwSerial.begin(9600);
  Serial.begin(9600);
  Blynk.begin(Serial, auth);
}

void loop() {
    if (digitalRead(PB_PIN) == LOW){
      
      if (!transientPeriodStarted){
        transientPeriodStarted = true;
        timeRef = millis();
      }
      
      else if (!bPressAccepted && (unsigned long)(millis() - timeRef) > TRANSIENT_PERIOD){
        condition = !condition;
        bPressAccepted = true;
      }
    }
    else{
      transientPeriodStarted = false;
      bPressAccepted = false;
    }
    Blynk.run();
    timer.run();
}

void requestEvent() {
  if (!condition){
    //Serial.println("Received request");
    digitalWrite(led, HIGH);
    Wire.write(1);
  }
  else {
    digitalWrite(led, LOW);
    while(true);
  }
}

void myTimerEvent() {
  int val_1 = analogRead(gasPin1);
  val_1 = map(val_1, 0, 1023, 0, 255);
  int val_2 = analogRead(gasPin2);
  val_2 = map(val_2, 0, 1023, 0, 255);
  
  boolean isDanger_1 = val_1 > 25;
  boolean isDanger_2 = val_2 > 50;
  
  Blynk.virtualWrite(V0, isDanger_1?"BAHAYA":"AMAN");
  Blynk.virtualWrite(V1, val_1);
  Blynk.virtualWrite(V2, isDanger_2?"BAHAYA":"AMAN");
  Blynk.virtualWrite(V3, val_2);
  Blynk.virtualWrite(V4, "MASTER");

  if (isDanger_1 || isDanger_2) {
    digitalWrite(BUZZER_PIN , HIGH);
    Blynk.notify("Terjadi kebocoran gas berbahaya!!");
  }
  else{
    digitalWrite(BUZZER_PIN , LOW);
  }
}
