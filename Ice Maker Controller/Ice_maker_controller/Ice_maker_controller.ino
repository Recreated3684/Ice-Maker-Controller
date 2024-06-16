#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Define the NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Update every minute

const int switchPin = D1; // Change this to your desired pin

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
  {7, 0, {1, 3, 5}, 3}, // 7:00 AM on Monday, Wednesday, and Friday
  {23, 0, {5}, 1}, // 11:00 PM on Friday
  {7, 0, {7}, 1}, // 7:00 AM on Sunday
  {23, 0, {7}, 1} // 11:00 PM on Sunday
};

void setup() {
  Serial.begin(115200);
  pinMode(switchPin, OUTPUT);
  digitalWrite(switchPin, LOW);

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
      Alarm.alarmRepeat(alarmSettings[i].days[j], alarmSettings[i].hour, alarmSettings[i].minute, 0, triggerSwitch);
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
  digitalWrite(switchPin, HIGH);
  delay(1000); // Keep the switch on for 1 second
  digitalWrite(switchPin, LOW);
}
