#include "ArduinoBLEClient.h"
#include <Arduino.h>

const int ONBOARD_LED = 2;

ArduinoBLEClient client;

void setup() {
    pinMode(ONBOARD_LED, OUTPUT);

    Serial.begin(115200);
    client.setup();
    // put your setup code here, to run once:
}

void loop() {
    static bool led = false;

    if (led)
        digitalWrite(ONBOARD_LED, HIGH);
    else
        digitalWrite(ONBOARD_LED, LOW);
    led = !led;
    delay(500);

    client.monitor();
    //  put your main code here, to run repeatedly:
}