#include <Arduino.h>
#include "DateTimeModule.h"

DateTimeModule dateTime;

void setup() {
    Serial.begin(115200);

    Serial.print("Data: ");
    Serial.print(dateTime.getDia());
    Serial.print("/");
    Serial.print(dateTime.getMes());
    Serial.print("/");
    Serial.println(dateTime.getAno());

    Serial.print("Hora: ");
    Serial.print(dateTime.getHora());
    Serial.print(":");
    Serial.print(dateTime.getMinuto());
    Serial.print(":");
    Serial.println(dateTime.getSegundo());
}

void loop() {
    // O loop principal
}
