#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>
#include <DHT.h>
#include <Adafruit_ST7735.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <TimeLib.h> // [QUAN TRỌNG] Giữ lại để xử lý thời gian cho AI & Clock

// --- CẤU HÌNH PHẦN CỨNG ---
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     17  // Đã fix chân DC
#define TFT_BL     26  // Chân Backlight

#define RELAY_1_PIN 33 // LED
#define RELAY_2_PIN 32 // AC
#define CONTROL_ALL_PIN 25

// [MỚI] Chân Buzzer (Còi chip)
// Chân 18 là SCK của màn hình, nên đổi sang 21
#define BUZZER_PIN 21 

// [MỚI - SIÊU VIỆT] Cảm biến siêu âm HC-SR04
// Lưu ý: Tránh chân 17 (đã dùng cho màn hình)
#define TRIG_PIN  16  // Chân phát sóng (Output) - Nối vào Trig của cảm biến
#define ECHO_PIN  35  // Chân thu sóng (Input Only) - Nối vào Echo của cảm biến

#define DHTPIN 27
#define DHTTYPE DHT11

// --- TOUCH PINS ---
#define TOUCH_MENU 12
#define TOUCH_UP   13
#define TOUCH_DOWN 14
#define TOUCH_THRESHOLD 420 

// --- MÀN HÌNH ---
#define C_BG           0x0000 
#define C_FLIP_BG      0x2124 
#define C_TEXT         0xFFFF 
#define C_ACCENT       0xF800 
#define C_HIGHLIGHT    0x07FF 
#define C_LINE         0x0000 

// --- BIẾN TOÀN CỤC ---
extern Adafruit_ST7735 tft;
extern DHT dht;
extern Preferences preferences;

// Trạng thái thiết bị
extern bool relay1State;
extern bool relay2State;
extern float currentTemp;
extern int currentHum;
extern int currentAQI; // [QUAN TRỌNG] Giữ lại biến AQI để hiển thị không khí

// Thời tiết & Lịch
extern String weatherDesc;
extern String lunarDateStr;
extern String lunarYearStr;
extern int weatherIconId; 

// Key SinricPro 
extern char app_key[50];
extern char app_secret[100];
extern char dev_relay1[30];
extern char dev_relay2[30];
extern char dev_all[30];

// Cấu hình OpenWeather (Dùng tọa độ chính xác)
extern String openWeatherMapApiKey;
extern String weatherLat; // Vĩ độ
extern String weatherLon; // Kinh độ

// [MỚI] Biến Báo Thức
extern int alarmHour;
extern int alarmMinute;
extern bool alarmEnabled;
extern bool isAlarmRinging;

#endif