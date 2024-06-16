#include <ESP8266WiFi.h>

// Define the GPIO pin
const int switchPin = D1; // GPIO pin to be used as switch

void setup() {
  Serial.begin(115200);
  pinMode(switchPin, OUTPUT);
  digitalWrite(switchPin, LOW); // Initially set the pin to low
}

void loop() {
  // Set GPIO pin high
  digitalWrite(switchPin, HIGH);
  Serial.println("Pin HIGH");
  delay(1000); // Wait for 1 second

  // Set GPIO pin low
  digitalWrite(switchPin, LOW);
  Serial.println("Pin LOW");
  delay(1000); // Wait for 1 second
}
