#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

// WiFi credentials
const char* ssid = "Troy and Abed in the Modem";
const char* password = "";

// Define the NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = -4 * 3600; // Adjust for your timezone (e.g., -5 hours for EST)
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000); // Update every minute

const int relayPin = D1; // GPIO pin to be used as relay control
const int ledPin = LED_BUILTIN; // Built-in LED pin for debugging
const int powerStatePin = A0; // Analog pin to read the LED voltage

// Thresholds for detecting the power state
const int onThreshold = 500; // Example threshold for ON state
const int offThreshold = 200; // Example threshold for OFF state

// Maximum number of days per alarm
const int MAX_DAYS = 7;

// Alarm settings
struct AlarmSetting {
  int hour;
  int minute;
  int days[MAX_DAYS];
  int numDays;
  bool isTurnOn; // true for turning on, false for turning off
};

// Define your alarms
AlarmSetting alarmSettings[] = {
  {7, 0, {1, 2, 3, 4, 5}, 5, true}, // Turn on, 7:00 AM on Weekdays
  {9, 0, {6, 7}, 2, true}, // Turn on, 9:00 AM on Weekends
  {9, 30, {1, 2, 3, 4, 5}, 5, false}, // Turn off, 9:30 AM on Weekdays
  {16, 0, {1, 2, 3, 4, 5}, 5, true}, // Turn on, 4:00 PM on Weekdays
  {23, 0, {1, 2, 3, 4, 5, 6, 7}, 7, false}, // Turn off, 11:00 PM Everyday
};

// Function to convert int to timeDayOfWeek_t
timeDayOfWeek_t intToDayOfWeek(int day) {
  switch(day) {
    case 1: return dowMonday;
    case 2: return dowTuesday;
    case 3: return dowWednesday;
    case 4: return dowThursday;
    case 5: return dowFriday;
    case 6: return dowSaturday;
    case 7: return dowSunday;
    default: return dowInvalid;
  }
}

// Function to read the power state over a period of time
bool isDeviceOn() {
  unsigned long start = millis();
  unsigned long duration = 1000; // Measure for 1 second
  bool sawOffState = false;

  while (millis() - start < duration) {
    int value = analogRead(powerStatePin);
    // Serial.print("Analog value: ");
    // Serial.println(value);
    if (value < offThreshold) {
      sawOffState = true;
    }
    delay(10); // Sample every 10ms
  }

  return !sawOffState;
}

// Function to trigger the relay to turn the device ON or OFF
void triggerRelay(bool turnOn) {
  bool currentState = isDeviceOn();
  if ((turnOn && !currentState) || (!turnOn && currentState)) {
    Serial.println("Toggling relay");
    digitalWrite(relayPin, HIGH); // Set GPIO pin low to activate relay
    delay(1000); // Keep the relay on for 1000 ms
    digitalWrite(relayPin, LOW); // Set GPIO pin high to deactivate relay
    delay(2000); // Wait for the device to change state
  } else {
    Serial.println("Device already in desired state");
  }
}

// Function to trigger the relay to turn the device ON
void turnOnDevice() {
  Serial.println("Turn ON command received");
  triggerRelay(true);
}

// Function to trigger the relay to turn the device OFF
void turnOffDevice() {
  Serial.println("Turn OFF command received");
  triggerRelay(false);
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 has restarted.");
  Serial.print("Reset reason: ");
  Serial.println(ESP.getResetReason());
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Initially set the pin to high (relay off)

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // LED is active LOW on ESP8266

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println("Number of alarms supported: " + String(dtNBR_ALARMS)); // prints the max quantity of alarms supported by the TimeAlarms.h library. Default is 12. Edit this file if more than 12 are needed.

  // Initialize NTP Client
  timeClient.begin();

  // Initialize time
  setSyncProvider(getNtpTime);
  setSyncInterval(3600); // Sync every 1 hour

  // Set alarms
  setAlarms();

  // Blink SOS to prove I'm alive
  blinkSOS();
}

void loop() {
  timeClient.update();
  // Break the 30000 milliseconds delay into smaller increments
  for (int i = 0; i < 30; i++) {
    Alarm.delay(1000); // Wait for 1 second
    ESP.wdtFeed(); // Explicitly feed the watchdog timer
  }
}

void setAlarms() {
  Serial.println("Setting alarms...");
  for (int i = 0; i < sizeof(alarmSettings) / sizeof(alarmSettings[0]); i++) {
    for (int j = 0; j < alarmSettings[i].numDays; j++) {
      timeDayOfWeek_t dayOfWeek = intToDayOfWeek(alarmSettings[i].days[j]);
      if (dayOfWeek != dowInvalid) {
        if (alarmSettings[i].isTurnOn) {
          Alarm.alarmRepeat(dayOfWeek, alarmSettings[i].hour, alarmSettings[i].minute, 0, turnOnDevice);
        } else {
          Alarm.alarmRepeat(dayOfWeek, alarmSettings[i].hour, alarmSettings[i].minute, 0, turnOffDevice);
        }
        Serial.printf("Alarm set: %02d:%02d %s (%s)\n", alarmSettings[i].hour, alarmSettings[i].minute, dayStr(dayOfWeek), alarmSettings[i].isTurnOn ? "ON" : "OFF");
      }
    }
  }
  Alarm.alarmRepeat(dowWednesday, 10, 46, 0, turnOffDevice); // An explicit alarm call to see if the error is in the handling of days or with the library.
  Serial.println("Alarms have been set.");
}

time_t getNtpTime() {
  unsigned long start = millis();
  unsigned long timeout = 5000; // 5 seconds timeout
  while (!timeClient.update()) {
    timeClient.forceUpdate();
    if (millis() - start >= timeout) {
      Serial.println("NTP update timed out");
      return 0; // Return 0 if NTP update fails
    }
    yield(); // Reset the watchdog timer
  }
  return timeClient.getEpochTime();
}

void printCurrentTime() {
  Serial.print("Current time: ");
  Serial.print(hour());
  Serial.print(":");
  if (minute() < 10) Serial.print("0"); // Leading zero for single digit minutes
  Serial.print(minute());
  Serial.print(":");
  if (second() < 10) Serial.print("0"); // Leading zero for single digit seconds
  Serial.print(second());
  Serial.print(" ");
  Serial.print(dayStr(weekday()));
  Serial.print(", ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(monthStr(month()));
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void blinkSOS() {
  // Morse code: S (dot-dot-dot) O (dash-dash-dash) S (dot-dot-dot)
  // dot duration is 200 ms, dash duration is 600 ms, space between symbols is 200 ms, space between letters is 600 ms

  int dotDuration = 200;
  int dashDuration = 600;
  int symbolSpace = 200;
  int letterSpace = 600;

  // S: dot-dot-dot
  blinkDot(dotDuration);
  delay(symbolSpace);
  blinkDot(dotDuration);
  delay(symbolSpace);
  blinkDot(dotDuration);
  delay(letterSpace);

  // O: dash-dash-dash
  blinkDash(dashDuration);
  delay(symbolSpace);
  blinkDash(dashDuration);
  delay(symbolSpace);
  blinkDash(dashDuration);
  delay(letterSpace);

  // S: dot-dot-dot
  blinkDot(dotDuration);
  delay(symbolSpace);
  blinkDot(dotDuration);
  delay(symbolSpace);
  blinkDot(dotDuration);
  delay(letterSpace);
}

void blinkDot(int duration) {
  digitalWrite(ledPin, LOW); // LED on
  delay(duration);
  digitalWrite(ledPin, HIGH); // LED off
  delay(duration);
}

void blinkDash(int duration) {
  digitalWrite(ledPin, LOW); // LED on
  delay(duration);
  digitalWrite(ledPin, HIGH); // LED off
  delay(duration);
}
