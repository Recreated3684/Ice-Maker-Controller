#include <ESP8266WiFi.h>

const int powerStatePin = A0; // Analog pin to read the LED voltage

void setup() {
  Serial.begin(115200);
  Serial.println("Starting analog read of A0 pin...");
}

void loop() {
  int value = analogRead(powerStatePin);
  Serial.print("Analog value: ");
  Serial.println(value);
  delay(500); // Read and print every 500ms
}
