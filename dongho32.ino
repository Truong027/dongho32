#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <vn_lunar.h> 
#include "config.h"
#include "ui_display.h"
#include "weather_app.h" 

// --- 1. KHAI BÁO TOÀN BỘ BIẾN CẤU HÌNH (GLOBAL VARIABLES) ---
// Biến Cài đặt Chung
int brightnessLevel = 2; // Cao
bool rainAlertEnabled = true;
int menuCursor = 0;
int currentAQI = 1; 

// Biến tính năng bổ sung
bool autoScrollEnabled = true; 
bool isFlipped = false;        
int timeFormat = 0;            // 0: 24h, 1: 12h
int screenTimeoutMode = 0;     // 0: Off, 1:15s, 2:30s, 3:60s
bool isScreenOff = false;      // Trạng thái màn hình hiện tại

int pendingAction = 0;         // 0:None, 1:WiFi, 2:Reboot...

// Biến Pro Settings
int touchSensitivity = 1;      // 0:Thấp, 1:Vừa, 2:Cao
int themeIndex = 0;            // 0: Cyan
uint16_t themeColor = 0x07FF;  // Màu chủ đề mặc định

// Biến AI
bool aiEnabled = true;         // Mặc định bật

// [MỚI] Biến tính năng mới (Sound & AI Personality)
bool soundEnabled = true;
int aiTalkMode = 1;            // 0: Im lặng, 1: Vừa, 2: Nói nhiều

// Biến Báo thức
int alarmHour = 6;             
int alarmMinute = 0;
bool alarmEnabled = false;
bool isAlarmRinging = false;

// Biến Chế độ Ban Đêm (Night Mode)
bool nightModeEnabled = true; 
int nightStartHour = 23;      // 23h đêm
int nightEndHour = 6;         // 6h sáng

// Biến đo hiệu năng
unsigned long lastLoopTime = 0;

// Biến Hệ thống & IoT
Preferences preferences;
DHT dht(DHTPIN, DHTTYPE);
vn_lunar vl;

// Config SinricPro
char app_key[50]      = "235433d6-8f38-452a-bdd8-761c1f58c026";
char app_secret[100]  = "7465c855-d4e4-49c9-9a3b-f889c6b4f257-1805b16b-c249-4bc1-bc11-c448f59711cc";
char dev_relay1[30]   = "68d7d137cf74abf8c94a2885"; 
char dev_relay2[30]   = "68d7d187cf74abf8c94a28c1"; 
char dev_all[30]      = "68d7d1b25009c4f120fb7f1e"; 

// Trạng thái Relay & Cảm biến
bool relay1State = false;
bool relay2State = false;
float currentTemp = 0.0; 
int currentHum = 0;      
String weatherDesc = "";
String lunarDateStr = "";
String openWeatherMapApiKey = "";

// Tọa độ
String weatherLat = "16.0078";
String weatherLon = "108.1899";

// Bộ đếm thời gian (Timers)
unsigned long lastWeatherUpdate = 0;
unsigned long lastTouchTime = 0;
unsigned long lastGraphUpdate = 0;
unsigned long lastRealtimeUpdate = 0; 
const unsigned long GRAPH_INTERVAL = 900000; 

// Điều khiển Carousel
unsigned long lastCarouselSwitch = 0;
const unsigned long CAROUSEL_DURATION = 15000; 
int currentScreen = 0; // 0: Main, 1: Weather, 2: Settings
int prevScreen = -1;   

int lastDay = -1;
float lastValidInTemp = 25.0;
int lastValidInHum = 60;
unsigned long lastRainBlink = 0;
int rainBlinkCount = 0;

// --- 2. INCLUDE MODULES (Thứ tự quan trọng) ---
// Settings App: Chứa menu logic
#include "settings_app.h"

// App Functions: Chứa hàm hệ thống (beep, applyBrightness...) -> Cần include SAU khi khai báo biến
#include "app_functions.h"

// Game App: Chứa Flashlight -> Cần applyBrightness từ app_functions
#include "game_app.h" 

// Creative Apps: Chứa 6 Games mới
#include "creative_apps.h"

// [MỚI] Các tiện ích cảm biến (Thước đo, Đỗ xe, Chống trộm)
#include "sensor_apps.h"

// AI Extensions: Chứa Shop và Vật phẩm AI
#include "ai_extensions.h"

// [FIX] QUAN TRỌNG: Include AI Assistant để code trong loop() nhìn thấy các biến AI
#include "ai_assistant.h"

// --- 3. HÀM LOCAL ---
// Hàm kiểm tra báo giờ (Hourly Chime)
void checkHourlyChime() {
    static int lastChimeHour = -1;
    int h = hour();
    int m = minute();
    
    if (m == 0 && h != lastChimeHour) {
        lastChimeHour = h;
        if (h >= 7 && h <= 22) {
             triggerAiReaction(AI_EVENT_HOURLY);
             // Chỉ AI kêu nhạc, không beep hệ thống
        }
    }
}

// Wrapper beep an toàn (có kiểm tra soundEnabled)
void safeBeep() {
    if (soundEnabled) beep();
}

// --- 4. SETUP ---
void setup() {
  Serial.begin(115200);
  pinMode(RELAY_1_PIN, OUTPUT); pinMode(RELAY_2_PIN, OUTPUT);
  digitalWrite(RELAY_1_PIN, HIGH); digitalWrite(RELAY_2_PIN, HIGH);

  pinMode(BUZZER_PIN, OUTPUT);
  
  // [MỚI] Cấu hình chân cảm biến siêu âm
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // [FIX] Cập nhật cho ESP32 Core 3.0+
  ledcAttach(BUZZER_PIN, 2000, 8);
  
  // Tiếng beep chuẩn khi khởi động
  safeBeep();

  initDisplay();
  tft.setRotation(1); 
  applyBrightness(); 
  
  drawBootScreen("Starting AI...");
  updateBootProgress(10);
  
  dht.begin();
  WiFiManager wm;
  
  drawBootScreen("Connect WiFi...");
  updateBootProgress(30);
  
  if (!wm.autoConnect("SmartClock_Pro")) {
    drawBootScreen("WiFi Failed!");
    delay(2000);
    ESP.restart();
  }

  drawBootScreen("Sync Time...");
  updateBootProgress(60);
  
  configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com");
  
  int retry = 0;
  while(time(nullptr) < 100000 && retry < 10) {
      delay(500);
      retry++;
  }

  drawBootScreen("Start IoT...");
  updateBootProgress(80);

  SinricProSwitch& s1 = SinricPro[dev_relay1]; s1.onPowerState(onPowerState);
  SinricProSwitch& s2 = SinricPro[dev_relay2]; s2.onPowerState(onPowerState);
  SinricProSwitch& sAll = SinricPro[dev_all]; sAll.onPowerState(onPowerState);
  SinricPro.begin(app_key, app_secret);
  SinricPro.restoreDeviceStates(true);

  drawBootScreen("Wake Up AI...");
  updateBootProgress(90);

  initAiShop(); // Khởi tạo Shop và tải trang bị AI

  fetchWeather();
  updateLunar(); 
  
  float t_in_start = dht.readTemperature();
  if (isnan(t_in_start)) t_in_start = 25; else lastValidInTemp = t_in_start;
  float t_out_start = currentTemp;
  if (t_out_start == 0) t_out_start = t_in_start;
  
  initGraphData((int)t_out_start, (int)t_in_start);
  
  updateBootProgress(100);
  delay(500); 
  tft.fillScreen(0); 
  
  lastCarouselSwitch = millis();
  
  triggerAiReaction(AI_EVENT_WAKEUP);
  
  // [NÂNG CẤP] Phát nhạc Startup hoành tráng thay vì tiếng beep đơn điệu
  if (soundEnabled) {
      playSystemSound(3); 
  } else {
      safeBeep();
  }
}

// --- 5. LOOP ---
void loop() {
  unsigned long startLoop = micros(); 

  // [MỚI] GỌI HÀM PHÁT TIẾNG BÁO THỨC (QUAN TRỌNG)
  loopAlarmSound();

  SinricPro.handle();

  // [FIX QUAN TRỌNG] Kiểm tra cảm biến siêu âm NGAY CẢ KHI MÀN HÌNH TẮT
  // Phải đặt trước dòng "if (isScreenOff) return;" để tính năng Wakeup hoạt động
  if (aiEnabled) {
      checkProximitySensor(); // Hàm này nằm trong ai_assistant.h
  }
  
  static unsigned long lastSyncTime = 0;
  if (millis() - lastSyncTime > 1000) {
      struct tm timeinfo;
      if(getLocalTime(&timeinfo)){
         setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      }
      lastSyncTime = millis();
      
      checkSmartNightMode();
      checkHourlyChime();
      checkAlarm();
  }
  
  if (isAlarmRinging) {
      triggerAiReaction(AI_EVENT_ALARM);
  }

  if (day() != lastDay) {
      updateLunar();
      lastDay = day();
  }

  static wl_status_t lastWifiStatus = WL_CONNECTED;
  if (WiFi.status() != WL_CONNECTED && lastWifiStatus == WL_CONNECTED) {
      triggerAiReaction(AI_EVENT_WIFI_LOST); 
  }
  lastWifiStatus = WiFi.status();

  if (screenTimeoutMode > 0 && !isScreenOff) {
      unsigned long timeoutMs = 0;
      if (screenTimeoutMode == 1) timeoutMs = 15000;
      else if (screenTimeoutMode == 2) timeoutMs = 30000;
      else if (screenTimeoutMode == 3) timeoutMs = 60000;

      if (millis() - lastTouchTime > timeoutMs) {
          isScreenOff = true;
          applyBrightness(); 
      }
  }

  if (millis() - lastTouchTime > 300) { 
      bool touched = false;

      // 1. NÚT MENU
      if (isTouchedSafe(TOUCH_MENU)) {
          safeBeep(); 
          wakeScreen();
          
          if (isAlarmRinging) {
              isAlarmRinging = false;
              triggerAiReaction(AI_EVENT_WAKEUP);
          } 
          else if (!isScreenOff) { 
              if (pendingAction > 0) {
                  // [SỬA LỖI] Cho phép thoát khỏi mọi màn hình Popup (kể cả 5 - Thông tin)
                  pendingAction = 0; 
                  prevScreen = -1;
              }
              else { 
                  currentScreen++;
                  if (currentScreen > 2) currentScreen = 0; 
                  lastCarouselSwitch = millis();  
              }
          }
          touched = true;
      }
      
      // 2. NÚT UP (Vào Menu chọn, tăng giá trị)
      if (isTouchedSafe(TOUCH_UP)) {
          wakeScreen();
          
          if (isAlarmRinging) {
              isAlarmRinging = false;
              triggerAiReaction(AI_EVENT_WAKEUP);
              safeBeep();
          }
          else if (!isScreenOff) {
              if (currentScreen == 2) { 
                  // Trong Menu Cài đặt
                  safeBeep();
                  if (pendingAction > 0) {
                      // [SỬA LỖI] Bấm nút nào cũng thoát Popup
                      pendingAction = 0; 
                      prevScreen = -1; 
                  } else {
                      switch (menuCursor) {
                          case 0: brightnessLevel = (brightnessLevel + 1) % 3; applyBrightness(); break;
                          case 1: themeIndex = (themeIndex + 1) % 4; themeColor = THEME_COLORS[themeIndex]; break;
                          case 2: rainAlertEnabled = !rainAlertEnabled; break;
                          case 3: autoScrollEnabled = !autoScrollEnabled; break;
                          case 4: isFlipped = !isFlipped; tft.setRotation(isFlipped ? 3 : 1); tft.fillScreen(0); prevScreen = -1; break;
                          case 5: timeFormat = !timeFormat; break;
                          case 6: touchSensitivity = (touchSensitivity + 1) % 3; break;
                          case 7: screenTimeoutMode = (screenTimeoutMode + 1) % 4; break;
                          
                          // HỆ THỐNG
                          case 8: pendingAction = 1; break; // Wifi
                          case 9: pendingAction = 4; break; // Chan Doan
                          case 10: 
                              aiEnabled = !aiEnabled; 
                              if(aiEnabled) wakeUpAi(); 
                              else {
                                  tft.fillRect((int)currentAiX - 15, AI_BASE_Y - 15, 30, 30, 0x0000);
                                  tft.fillRect(0, AI_BASE_Y - 50, 160, 45, 0x0000); 
                              }
                              break; 
                          case 11: pendingAction = 2; break; // Reboot
                          
                          // Mục Báo thức
                          case 12: alarmHour = (alarmHour + 1) % 24; break;
                          case 13: alarmMinute = (alarmMinute + 5) % 60; break; 
                          case 14: alarmEnabled = !alarmEnabled; break;

                          // Mục Chế độ Ban Đêm
                          case 15: nightModeEnabled = !nightModeEnabled; break;
                          case 16: nightStartHour = (nightStartHour + 1) % 24; break;
                          case 17: nightEndHour = (nightEndHour + 1) % 24; break;
                          
                          // [MỚI] TÍNH NĂNG NÂNG CAO
                          case 18: soundEnabled = !soundEnabled; break;
                          case 19: aiTalkMode = (aiTalkMode + 1) % 3; break;
                          
                          // [MỚI] THAY ĐỔI SKIN AI
                          // [FIX] Thêm case 20 để thay đổi Skin. TOTAL_SKINS = 5 (0-4)
                          case 20: currentSkin = (currentSkin + 1) % 5; break;

                          // KHO GAME & ỨNG DỤNG (Cập nhật số thứ tự)
                          case 21: // CUU SOS (Flashlight)
                              runFlashlightTool();
                              tft.fillScreen(0); prevScreen = -1;
                              break;
                          case 22: // GAME CENTER
                              runCreativeMenu();
                              tft.fillScreen(0); prevScreen = -1;
                              break;
                          case 23: // TIEN ICH CAM BIEN
                              runSensorMenu();
                              tft.fillScreen(0); prevScreen = -1;
                              break;
                          case 24: // THONG TIN
                              pendingAction = 5; 
                              break;
                      }
                  }
                  lastCarouselSwitch = millis();
              } else {
                  // MÀN HÌNH CHÍNH: Điều khiển Relay 1
                  bool newState = !relay1State;
                  onPowerState(String(dev_relay1), newState); 
                  SinricProSwitch& s = SinricPro[dev_relay1]; s.sendPowerStateEvent(newState);
                  triggerAiReaction(newState ? AI_EVENT_RELAY_ON : AI_EVENT_RELAY_OFF);
              }
          }
          touched = true;
      }
      
      // 3. NÚT DOWN (Di chuyển Menu)
      if (isTouchedSafe(TOUCH_DOWN)) {
          wakeScreen();
          
          if (isAlarmRinging) {
              isAlarmRinging = false;
              triggerAiReaction(AI_EVENT_WAKEUP);
              safeBeep(); 
          }
          else if (!isScreenOff) {
              if (currentScreen == 2) {
                  // Trong Menu Cài đặt
                  safeBeep();
                  lastCarouselSwitch = millis();
                  if (pendingAction > 0) {
                      if (pendingAction == 1) { // RESET WIFI
                          // [NÂNG CẤP] Âm thanh xác nhận
                          if (soundEnabled) playSystemSound(0);
                          
                          tft.fillScreen(0);
                          tft.setCursor(20, 60); tft.print("Dang xoa WiFi...");
                          WiFiManager wm; wm.resetSettings(); delay(1000); ESP.restart();
                      }
                      else if (pendingAction == 2) { // REBOOT
                          // [NÂNG CẤP] Âm thanh hành động
                          if (soundEnabled) playSystemSound(2);

                          tft.fillScreen(0);
                          tft.setCursor(20, 60); tft.print("Dang khoi dong...");
                          delay(1000); ESP.restart();
                      }
                      else {
                          // [SỬA LỖI] Bấm nút nào cũng thoát Popup
                          pendingAction = 0; prevScreen = -1;
                      }
                  } 
                  else {
                      // Menu chính: Di chuyển xuống
                      // [FIX] CẬP NHẬT TỔNG SỐ MỤC LÊN 25 (0 đến 24) ĐỂ HIỂN THỊ HẾT
                      menuCursor = (menuCursor + 1) % 25; 
                  }
              } else {
                  // MÀN HÌNH CHÍNH: Điều khiển Relay 2
                  bool newState = !relay2State;
                  onPowerState(String(dev_relay2), newState); 
                  SinricProSwitch& s = SinricPro[dev_relay2]; s.sendPowerStateEvent(newState); 
                  triggerAiReaction(newState ? AI_EVENT_RELAY_ON : AI_EVENT_RELAY_OFF);
              }
          }
          touched = true;
      }

      if (touched) lastTouchTime = millis();
  }

  if (!isScreenOff && autoScrollEnabled && currentScreen != 2 && pendingAction == 0) {
      if (millis() - lastCarouselSwitch > CAROUSEL_DURATION) {
          if (currentScreen == 0) currentScreen = 1;
          else if (currentScreen == 1) currentScreen = 0;
          lastCarouselSwitch = millis();
      }
  }

  if (isScreenOff) return;

  drawSystemBar(WiFi.status() == WL_CONNECTED, ESP.getFreeHeap());
  drawProgressBar(millis() - lastCarouselSwitch, CAROUSEL_DURATION);

  bool screenChanged = (currentScreen != prevScreen);
  if (screenChanged) {
      tft.fillRect(0, 20, 160, 108, 0x0000); 
      prevScreen = currentScreen;
  }

  if (currentScreen == 0) {
      int h = hour(); int m = minute();
      if (timeFormat == 1) { if (h == 0) h = 12; else if (h > 12) h -= 12; }

      static char dayOfWeek[5]; static char dateStr[10];  const char* weekDays[] = {"CN", "T2", "T3", "T4", "T5", "T6", "T7"};
      if (year() < 2024) { strcpy(dayOfWeek, "ERR"); strcpy(dateStr, "SYNC.."); h=0; m=0; } 
      else { strcpy(dayOfWeek, weekDays[weekday()-1]); snprintf(dateStr, sizeof(dateStr), "%02d/%02d", day(), month()); }
      drawMainScreen(h, m, dayOfWeek, dateStr, lunarDateStr.c_str(), relay1State, relay2State, screenChanged);
  } 
  else if (currentScreen == 1) {
      if (screenChanged) {
          float t_read = dht.readTemperature(); int h_read = dht.readHumidity();
          if (!isnan(t_read)) lastValidInTemp = t_read; if (!isnan(h_read)) lastValidInHum = h_read;
          float showT_Out = (currentTemp != 0) ? currentTemp : lastValidInTemp;
          drawWeatherScreen(showT_Out, currentHum, lastValidInTemp, lastValidInHum, weatherDesc, currentAQI);
      }
      if (millis() - lastRealtimeUpdate > 3000) {
          float t_read = dht.readTemperature(); int h_read = dht.readHumidity();
          if (!isnan(t_read) && !isnan(h_read)) {
              lastValidInTemp = t_read; lastValidInHum = h_read;
              float showT_Out = (currentTemp != 0) ? currentTemp : lastValidInTemp;
              updateWeatherReadings(showT_Out, currentHum, lastValidInTemp, lastValidInHum);
          }
          lastRealtimeUpdate = millis();
      }
  }
  else if (currentScreen == 2) {
      drawSettingsScreen(screenChanged);
  }

  // AI Loop chạy ngầm (Kiểm tra Todo, Wellbeing)
  if (ai_loop()) {
      prevScreen = -1; 
  }

  if (millis() - lastWeatherUpdate > 600000) { fetchWeather(); lastWeatherUpdate = millis(); }

  if (millis() - lastGraphUpdate > GRAPH_INTERVAL) {
      float t_in_read = dht.readTemperature(); if (isnan(t_in_read)) t_in_read = lastValidInTemp; else lastValidInTemp = t_in_read;
      float t_out_read = currentTemp; if (t_out_read == 0) t_out_read = -99; 
      pushGraphData((int)t_out_read, (int)t_in_read);
      if (currentScreen == 1) prevScreen = -1; 
      lastGraphUpdate = millis();
  }

  lastLoopTime = micros() - startLoop;
}
