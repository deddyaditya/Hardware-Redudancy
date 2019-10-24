#include <I2C.h>

#define BLYNK_PRINT SwSerial
#include <SoftwareSerial.h>
SoftwareSerial SwSerial(10, 11); // RX, TX

#include <BlynkSimpleStream.h>

char auth[] = "3abc00e4e9b24264beec6e24a1d62665";

BlynkTimer timer;

int gasPin1 = A0; 
int gasPin2 = A1; 

const int BUZZER_PIN = 8;
int relay = 2;
int led = 13;

const int MASTER_ADDRESS = 42;

void setup() {
  I2c.begin ();  
  I2c.timeOut(100);  // in milliseconds
  
  SwSerial.begin(9600);
  Serial.begin(9600);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(gasPin1, INPUT);
  pinMode(gasPin2, INPUT);
  digitalWrite(relay, LOW);

  Blynk.begin(Serial, auth);
  timer.setInterval(500L, myTimerEvent);
}

uint8_t oldStatus = 0, newStatus = 0;

void loop() {
  Blynk.run();  
  uint8_t masterStatus;
  static unsigned long last = millis();
  
  unsigned long delayDowntime = millis();
  if (millis() - last > 100){
    last = millis();  
    if (I2c.read(MASTER_ADDRESS, sizeof masterStatus, &masterStatus) == 0) {
      if (masterStatus){
        //Serial.println("Master Status : ONLINE");
        digitalWrite(led, LOW);
        digitalWrite(relay, LOW);
      }
      else {
        timer.run();
        digitalWrite(relay, HIGH);
        digitalWrite(led, HIGH);
        //Serial.println("Master Status : OFFLINE");
      }
    } else {
      masterStatus = 0;
      timer.run();
      digitalWrite(relay, HIGH);
      digitalWrite(led, HIGH);
      //Serial.println("Master Status : OFFLINE");
      }
  }

  newStatus = masterStatus;
  if(newStatus != oldStatus){
    if(newStatus == 0) {
//      String msg = "Master is currently down [" + (millis()-delayDowntime);
//      msg += " ms]";
//      Blynk.notify((millis() - delayDowntime));
      Blynk.notify("Master is currently down and needed to be repaired manually");
    }
    else if (newStatus == 1){
      Blynk.notify("Master is back online");
    }
    oldStatus = newStatus;
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
  Blynk.virtualWrite(V4, "SLAVE");
  
  if (isDanger_1 || isDanger_2) {
    digitalWrite(BUZZER_PIN , HIGH);
    Blynk.notify("Terjadi kebocoran gas berbahaya!!");
  }
  else{
    digitalWrite(BUZZER_PIN , LOW);
  }
}
