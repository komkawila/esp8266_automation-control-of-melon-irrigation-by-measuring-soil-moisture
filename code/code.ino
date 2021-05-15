#include <ESP8266WiFi.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* ssid = "PAELECTRONIC_2.4GHz";             // SSID is set
const char* password = "12345678";     // Password is set

char ntp_server1[20] = "pool.ntp.org";
char ntp_server2[20] = "time.nist.gov";
char ntp_server3[20] = "time.uni.net.th";

int timezone = 7 * 3600;
int dst = 0;
String datetime = "";

#define OUTPUT1 14 // D5
#define BUTTON 13 // D7
int count = 1;
bool state = false;
int soil = 0;

int startHR = 0;
int startMN = 0;

unsigned long pre = 0;
unsigned long post = 0;

bool bt = false;
bool old_bt = false;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  pinMode(OUTPUT1, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  digitalWrite(OUTPUT1, 1);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Connecting to ");
  lcd.print(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
  Serial.println("");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("WiFi connected");
  delay(1000);
  configTime(timezone, dst, ntp_server1, ntp_server2, ntp_server3);
  Serial.println("\nWaiting for time");
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Waiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
}

void loop() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  bt = !digitalRead(BUTTON);
  if (bt && !old_bt) {

    state = !state;
    if (state) {
      startHR = p_tm->tm_hour;
      startMN = p_tm->tm_min;
    }
    count = 1;
    digitalWrite(OUTPUT1, 1);
  }
  old_bt = bt;



  pre = millis();
  if (pre - post >= 1000) {
    if ((p_tm->tm_year + 1900) > 2020) {
      datetime = "";
      datetime += String(p_tm->tm_mday);
      datetime += "/";
      datetime += String(p_tm->tm_mon + 1);
      datetime += "/";
      datetime += String(p_tm->tm_year + 1900);
      datetime += " ";
      if (p_tm->tm_hour < 10)
        datetime += "0";
      datetime += String(p_tm->tm_hour);
      datetime += ":";
      if (p_tm->tm_min < 10)
        datetime += "0";
      datetime += String(p_tm->tm_min);
      datetime += ":";
      if (p_tm->tm_sec < 10)
        datetime += "0";
      datetime += String(p_tm->tm_sec);

      soil = readSoil();
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Soil Automation");
      lcd.setCursor(1, 1);
      lcd.print(datetime);
      lcd.setCursor(0, 2);
      lcd.print("Soil Moisture: ");
      lcd.print(soil);
      lcd.print("%");
      lcd.setCursor(0, 3);
      if (!state) {
        lcd.print("STATUS: STOP");
        digitalWrite(OUTPUT1, 1);
      } else {

        Serial.print("startMN = ");
        Serial.print(startMN);
        Serial.print("   p_tm->tm_min = ");
        Serial.print(p_tm->tm_min);
        Serial.print("   p_tm->tm_sec = ");
        Serial.println(p_tm->tm_sec);
        if (startHR == p_tm->tm_hour && startMN == p_tm->tm_min && (p_tm->tm_sec == 0 || p_tm->tm_sec == 1)) {
          count++;
          if (count == 5) {
            count = 1;
            state = false;
          }
          delay(2000);
        }
        if (count == 1) {
          if (soil <= 30) {
            digitalWrite(OUTPUT1, 0);
          } else {
            digitalWrite(OUTPUT1, 1);
          }
        } else if (count == 2) {
          if (soil <= 40) {
            digitalWrite(OUTPUT1, 0);
          } else {
            digitalWrite(OUTPUT1, 1);
          }
        } else if (count == 3) {
          if (soil <= 60) {
            digitalWrite(OUTPUT1, 0);
          } else {
            digitalWrite(OUTPUT1, 1);
          }
        } else if (count == 4) {
          if (soil <= 80) {
            digitalWrite(OUTPUT1, 0);
          } else {
            digitalWrite(OUTPUT1, 1);
          }
        } else {
          digitalWrite(OUTPUT1, 1);
        }
        lcd.print("STATUS: START DAY: ");
        lcd.print(count);
      }

      Serial.println(datetime);
    }
    post = pre;
  }
}

int readSoil() {
  int sensor = 0;
  for (int i = 0; i < 20; i++) {
    sensor += analogRead(A0);
    delay(10);
  }
  return map((sensor / 20), 0, 1023, 100, 0);
}
