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

void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 has restarted.");
  Serial.print("Reset reason: ");
  Serial.println(ESP.getResetReason());
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

  // Initialize NTP Client
  timeClient.begin();
  
  // Initialize time
  setSyncProvider(getNtpTime);
  setSyncInterval(600); // Sync every 10 minutes

  // Set alarms
  setAlarms();

  // Blink SOS to prove I'm alive
   blinkSOS();
}

void loop() {
  // Serial.println("Loop start");
  timeClient.update();
  // Break the 30000 milliseconds delay into smaller increments
  for (int i = 0; i < 30; i++) {
    Alarm.delay(1000); // Wait for 1 second
    ESP.wdtFeed(); // Explicitly feed the watchdog timer
  }
  // Print the current time every second
  printCurrentTime();
  // blinkSOS();
  // Serial.println("Loop end");
}

void setAlarms() {
  Serial.println("Setting alarms...");
  for (int i = 0; i < sizeof(alarmSettings) / sizeof(alarmSettings[0]); i++) {
    for (int j = 0; j < alarmSettings[i].numDays; j++) {
      timeDayOfWeek_t dayOfWeek = intToDayOfWeek(alarmSettings[i].days[j]);
      if (dayOfWeek != dowInvalid) {
        Alarm.alarmRepeat(dayOfWeek, alarmSettings[i].hour, alarmSettings[i].minute, 0, triggerRelay);
        Serial.printf("Alarm set: %02d:%02d %s\n", alarmSettings[i].hour, alarmSettings[i].minute, dayStr(dayOfWeek));
      }
    }
  }
  Serial.println("Alarms have been set.");
}

time_t getNtpTime() {
  // Serial.print("getNtpTime_Start");
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
  // Serial.print("getNtpTime_End");
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

void triggerRelay() {
  Serial.println("Relay triggered");
  digitalWrite(relayPin, LOW); // Set GPIO pin low to activate relay
  delay(2000); // Keep the relay on for 2000 ms
  digitalWrite(relayPin, HIGH); // Set GPIO pin high to deactivate relay

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
