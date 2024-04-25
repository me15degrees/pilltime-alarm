#include <ESP32Servo.h>

Servo servoUm;
Servo servoDois;
int servoUmPin = 5;
int servoDoisPin = 22;
void setup() {
  servoUm.attach(servoUmPin);
  servoDois.attach(servoDoisPin);
  Serial.begin(115200);
}

void loop() {
  if(Serial.available()){
    int angle = Serial.parseInt();
    servoUm.write(angle);
    servoDois.write(angle);
  }
  delay(20);
}
