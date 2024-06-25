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

const int relayPin = D1; // GPIO pin to be used as relay control
const int ledPin = LED_BUILTIN; // Built-in LED pin for debugging

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

// Function to trigger the relay
void triggerRelay() {
  Serial.println("Relay triggered");
  digitalWrite(relayPin, LOW); // Set GPIO pin low to activate relay
  delay(2000); // Keep the relay on for 500 ms
  digitalWrite(relayPin, HIGH); // Set GPIO pin high to deactivate relay

  // SOS pattern: dot-dot-dot, dash-dash-dash, dot-dot-dot
  blinkSOS();
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Initially set the pin to high (relay off)

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // LED is active LOW on ESP8266

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // Initialize NTP Client
  timeClient.begin();
  timeClient.update(); // Get initial time
  setTime(timeClient.getEpochTime());
  setSyncProvider(getNtpTime);
  setSyncInterval(3600); // Sync every hour

  // Set alarms
  setAlarms();

  // Blink SOS to prove I'm alive
  blinkSOS();
}

void loop() {
  Alarm.delay(1000); // Wait for alarms to be triggered

  // Print the current time every second
 // printCurrentTime();
}

void setAlarms() {
  Serial.println("Setting alarms...");
  for (int i = 0; i < sizeof(alarmSettings) / sizeof(alarmSettings[0]); i++) {
    for (int j = 0; j < alarmSettings[i].numDays; j++) {
      timeDayOfWeek_t dayOfWeek = intToDayOfWeek(alarmSettings[i].days[j]);
      if (dayOfWeek == dowInvalid) {
        Serial.printf("Invalid day for alarm %d: %d\n", i, alarmSettings[i].days[j]);
        continue; // Skip invalid days
      }
      if (alarmSettings[i].hour < 0 || alarmSettings[i].hour > 23 || alarmSettings[i].minute < 0 || alarmSettings[i].minute > 59) {
        Serial.printf("Invalid time for alarm %d: %02d:%02d\n", i, alarmSettings[i].hour, alarmSettings[i].minute);
        continue; // Skip invalid times
      }
      Alarm.alarmRepeat(dayOfWeek, alarmSettings[i].hour, alarmSettings[i].minute, 0, triggerRelay);
      Serial.printf("Alarm set: %02d:%02d %s\n", alarmSettings[i].hour, alarmSettings[i].minute, dayStr(dayOfWeek));
    }
  }
  Serial.println("Alarms have been set.");
}

time_t getNtpTime() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  return timeClient.getEpochTime();
}
 /*
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
*/
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
