#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

// WiFi credentials
const char* ssid = "Troy and Abed in the Modem";
const char* password = "12345678";

// Define the NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = -4 * 3600; // Adjust for your timezone (e.g., -5 hours for EST)
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000); // Update every minute

const int switchPin = D1; // Change this to your desired pin
const int ledPin = LED_BUILTIN; // Built-in LED pin

// Maximum number of days per alarm
const int MAX_DAYS = 7;

// Alarm settings
struct AlarmSetting {
  int hour;
  int minute;
  int days[MAX_DAYS];
  int numDays;
};

// Define your alarms
AlarmSetting alarmSettings[] = {
  {7, 0, {1, 2, 3, 4, 5}, 5}, // Turn on, 7:00 AM on Weekdays
  {9, 0, {6, 7}, 2}, // Turn on, 9:00 AM on Weekends
  {9, 30, {1, 2, 3, 4, 5}, 5}, // Turn off, 9:30 AM on Weekdays
  {16, 0, {1, 2, 3, 4, 5}, 5}, // Turn on, 4:00 PM on Weekdays
  {23, 0, {1, 2, 3, 4, 5, 6, 7}, 7}, // Turn off, 11:00 PM Everyday
  {19, 55, {1}, 1}
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

void setup() {
  Serial.begin(115200);
  pinMode(switchPin, OUTPUT);
  digitalWrite(switchPin, LOW);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // LED is active LOW on ESP8266

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize NTP Client
  timeClient.begin();
  
  // Initialize time
  setSyncProvider(getNtpTime);
  setSyncInterval(600); // Sync every 10 minutes

  // Set alarms
  setAlarms();
}

void loop() {
  timeClient.update();
  Alarm.delay(1000); // Wait for alarms to be triggered
}

void setAlarms() {
  for (int i = 0; i < sizeof(alarmSettings) / sizeof(alarmSettings[0]); i++) {
    for (int j = 0; j < alarmSettings[i].numDays; j++) {
      timeDayOfWeek_t dayOfWeek = intToDayOfWeek(alarmSettings[i].days[j]);
      if (dayOfWeek != dowInvalid) {
        Alarm.alarmRepeat(dayOfWeek, alarmSettings[i].hour, alarmSettings[i].minute, 0, triggerSwitch);
      }
    }
  }
}

time_t getNtpTime() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  return timeClient.getEpochTime();
}

void triggerSwitch() {
  Serial.println("Switch triggered");
  digitalWrite(switchPin, HIGH); // Turn on the transistor to close the connection
  delay(500); // Keep the switch on for 500 ms
  digitalWrite(switchPin, LOW); // Turn off the transistor to open the connection

  // SOS pattern: dot-dot-dot, dash-dash-dash, dot-dot-dot
  blinkSOS();
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
