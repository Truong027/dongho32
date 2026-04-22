#ifndef APP_FUNCTIONS_H
#define APP_FUNCTIONS_H

#include "config.h"

// --- KHAI BÁO BIẾN TOÀN CỤC (EXTERN) ---
extern bool relay1State;
extern bool relay2State;
extern char dev_relay1[];
extern char dev_relay2[];
extern char dev_all[];
extern bool rainAlertEnabled;
extern String weatherDesc;
extern unsigned long lastRainBlink;
extern String weatherLat;
extern String weatherLon;
extern String openWeatherMapApiKey;
extern float currentTemp;
extern int currentHum;
extern int currentAQI;
extern vn_lunar vl;
extern String lunarDateStr;
extern int touchSensitivity;
extern int brightnessLevel;
extern bool isScreenOff;
extern unsigned long lastTouchTime;
extern bool aiEnabled; 

// Biến báo thức
extern int alarmHour;
extern int alarmMinute;
extern bool alarmEnabled;
extern bool isAlarmRinging;
// [NÂNG CẤP] Biến theo dõi thời gian báo thức để tăng tốc độ chuông
unsigned long alarmStartTime = 0;

// [MỚI] Biến giờ đêm từ dongho32.ino
extern bool nightModeEnabled;
extern int nightStartHour;
extern int nightEndHour;

// --- [NÂNG CẤP 1] HỆ THỐNG ÂM THANH UI 8-BIT ---
// Các nốt nhạc cơ bản (Thêm A5, B5 cho nhạc Startup)
#define NOTE_C5 523
#define NOTE_E5 659
#define NOTE_G5 784
#define NOTE_A5 880
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_ERR 150

// 1. HÀM BEEP CHUẨN (Fix cho Core 3.0 - Thay digitalWrite bằng ledcWriteTone)
void beep() {
    ledcWriteTone(BUZZER_PIN, 1000); 
    delay(60);                       
    ledcWriteTone(BUZZER_PIN, 0);    
}

// 2. HÀM BEEP RELAY (Tiếng "Tách" cơ học)
void relayBeep() {
    ledcWriteTone(BUZZER_PIN, 1000); 
    delay(20);                       
    ledcWriteTone(BUZZER_PIN, 0);    
}

// [MỚI] Hàm phát âm thanh hệ thống (Blocking - dùng cho sự kiện ngắn)
// type 0: Success (Tăng dần - Dùng khi kết nối WiFi/Lưu cài đặt)
// type 1: Error (Trầm đục - Dùng khi lỗi mạng/Sai thao tác)
// type 2: Action (Thanh thoát - Dùng khi bật/tắt thiết bị)
// type 3: Startup (Vui tươi - Dùng khi khởi động)
void playSystemSound(int type) {
    if (type == 0) { // Success
        ledcWriteTone(BUZZER_PIN, NOTE_E5); delay(80);
        ledcWriteTone(BUZZER_PIN, NOTE_G5); delay(80);
        ledcWriteTone(BUZZER_PIN, NOTE_C6); delay(100);
        ledcWriteTone(BUZZER_PIN, 0);
    } 
    else if (type == 1) { // Error
        ledcWriteTone(BUZZER_PIN, NOTE_ERR); delay(150);
        ledcWriteTone(BUZZER_PIN, 0); delay(50);
        ledcWriteTone(BUZZER_PIN, NOTE_ERR); delay(150);
        ledcWriteTone(BUZZER_PIN, 0);
    }
    else if (type == 2) { // Action
        ledcWriteTone(BUZZER_PIN, NOTE_C6); delay(50);
        ledcWriteTone(BUZZER_PIN, 0);
    }
    else if (type == 3) { // [MỚI] Startup (Arpeggio đi lên)
        ledcWriteTone(BUZZER_PIN, NOTE_E5); delay(100);
        ledcWriteTone(BUZZER_PIN, NOTE_G5); delay(100);
        ledcWriteTone(BUZZER_PIN, NOTE_B5); delay(100);
        ledcWriteTone(BUZZER_PIN, NOTE_C6); delay(200);
        ledcWriteTone(BUZZER_PIN, 0);
    }
}

// 3. Xử lý Relay & SinricPro
bool onPowerState(const String &deviceId, bool &state) {
  bool handled = false;
  int relaySignal = state ? LOW : HIGH; // Relay module thường kích LOW

  if (deviceId == String(dev_relay1)) {
    relay1State = state;
    digitalWrite(RELAY_1_PIN, relaySignal);
    handled = true;
  } 
  else if (deviceId == String(dev_relay2)) {
    relay2State = state;
    digitalWrite(RELAY_2_PIN, relaySignal);
    handled = true;
  }
  else if (deviceId == String(dev_all)) {
    relay1State = state;
    relay2State = state;
    digitalWrite(RELAY_1_PIN, relaySignal);
    digitalWrite(RELAY_2_PIN, relaySignal);
    SinricProSwitch& s1 = SinricPro[dev_relay1]; s1.sendPowerStateEvent(state);
    SinricProSwitch& s2 = SinricPro[dev_relay2]; s2.sendPowerStateEvent(state);
    handled = true;
  }
  
  // Gọi hàm beep ngắn khi điều khiển xong
  if (handled) {
      relayBeep(); 
  }
  return true;
}

// 4. Cảnh báo mưa (Nháy Relay 1)
void checkRainWarning() {
    if (!rainAlertEnabled) return;
    String w = weatherDesc;
    w.toLowerCase();
    
    if (w.indexOf("rain") >= 0 || w.indexOf("thunder") >= 0) {
        if (!relay1State && millis() - lastRainBlink > 900000) { 
            for(int i=0; i<3; i++) {
                digitalWrite(RELAY_1_PIN, LOW); delay(200);
                digitalWrite(RELAY_1_PIN, HIGH); delay(200);
            }
            lastRainBlink = millis();
        }
    }
}

// --- [NÂNG CẤP 2] PHÂN TÍCH TIỆN NGHI MÔI TRƯỜNG ---
String getComfortStatus() {
    if (currentTemp == 0) return "Dang cap nhat...";
    
    // Logic ưu tiên cảnh báo cực đoan trước
    if (currentTemp > 33) return "Oi buc! (Hot)";
    if (currentTemp < 16) return "Lanh gia (Cold)";
    
    // Logic độ ẩm
    if (currentHum < 35) return "Kho hanh (Dry)";
    if (currentHum > 80) return "Am uot (Humid)";
    
    // Vùng dễ chịu: Temp 20-28, Hum 40-70
    if (currentTemp >= 20 && currentTemp <= 28 && currentHum >= 40 && currentHum <= 70) {
        return "De chiu (Good)";
    }
    
    return "Binh thuong";
}

// 5. Lấy thời tiết từ OpenWeatherMap
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + weatherLat + "&lon=" + weatherLon + "&appid=" + openWeatherMapApiKey + "&units=metric";
  http.begin(url);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(768); 
    deserializeJson(doc, payload);
    currentTemp = doc["main"]["temp"];
    currentHum = doc["main"]["humidity"];
    weatherDesc = doc["weather"][0]["description"].as<String>();
  }
  http.end();
  
  String aqiUrl = "http://api.openweathermap.org/data/2.5/air_pollution?lat=" + weatherLat + "&lon=" + weatherLon + "&appid=" + openWeatherMapApiKey;
  http.begin(aqiUrl);
  code = http.GET();
  if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      DynamicJsonDocument doc(512); 
      deserializeJson(doc, payload);
      currentAQI = doc["list"][0]["main"]["aqi"]; 
  }
  http.end();
  
  checkRainWarning(); 
}

// 6. Cập nhật Âm lịch
void updateLunar() {
  time_t now = ::now();
  int d = day(now);
  int m = month(now);
  int y = year(now);
  
  if (y > 2020) {
      vl.convertSolar2Lunar(d, m, y);
      int ld = vl.get_lunar_dd();
      int lm = vl.get_lunar_mm();
      lunarDateStr = "AL:" + String(ld) + "/" + String(lm);
  }
}

// 7. Xử lý chạm cảm ứng (kèm độ nhạy)
bool isTouchedSafe(int pin) {
    int threshold = TOUCH_THRESHOLD;
    if (touchSensitivity == 0) threshold -= 30; 
    else if (touchSensitivity == 2) threshold += 30;

    if (touchRead(pin) >= threshold) return false;
    // Debounce kỹ hơn
    for (int i = 0; i < 3; i++) {
        delay(15); 
        if (touchRead(pin) >= threshold) {
            return false; 
        }
    }
    return true; 
}

// 8. Điều khiển độ sáng màn hình
void applyBrightness() {
    if (isScreenOff) {
        ledcWrite(TFT_BL, 0); // Tắt hẳn
    } else {
        int duty = 255;
        if (brightnessLevel == 0) duty = 50;  
        else if (brightnessLevel == 1) duty = 150; 
        ledcWrite(TFT_BL, duty);
    }
}

// 9. Đánh thức màn hình
void wakeScreen() {
    if (isScreenOff) {
        isScreenOff = false;
        applyBrightness();
        lastTouchTime = millis(); // Reset thời gian chờ
    }
}

// 10. Tự động chế độ Ban Đêm (Smart Night Mode)
void checkSmartNightMode() {
    int h = hour();
    static bool isNightMode = false;

    // Kiểm tra xem có BẬT tính năng này không
    if (!nightModeEnabled) {
        // Nếu tính năng bị TẮT nhưng hệ thống đang ở chế độ đêm -> Phải thoát ngay
        if (isNightMode) {
            if (brightnessLevel == 0) { 
                brightnessLevel = 2; // Khôi phục độ sáng Cao
                applyBrightness();
            }
            if (!aiEnabled) {
                aiEnabled = true; // Bật lại AI
            }
            isNightMode = false;
        }
        return; // Thoát hàm, không làm gì thêm
    }

    // Logic linh hoạt theo biến cài đặt nightStartHour / nightEndHour
    bool inNightTime = false;
    if (nightStartHour > nightEndHour) {
        // Ví dụ: 23h đến 6h (Qua đêm)
        if (h >= nightStartHour || h < nightEndHour) inNightTime = true;
    } else {
        // Ví dụ: 0h đến 5h (Cùng ngày - ít dùng nhưng support)
        if (h >= nightStartHour && h < nightEndHour) inNightTime = true;
    }

    if (inNightTime) {
        if (!isNightMode) {
            // Chuyển sang chế độ đêm
            if (brightnessLevel != 0) { 
                brightnessLevel = 0; // Giảm sáng
                applyBrightness();
            }
            if (aiEnabled) {
                aiEnabled = false; // Tắt AI để không làm phiền
            }
            isNightMode = true;
        }
    } 
    // Kiểm tra giờ hồi phục (Ngay sau khi hết giờ đêm)
    else if (h >= nightEndHour && h < (nightEndHour + 1)) {
        if (isNightMode) {
            // Chuyển sang chế độ ngày
            if (brightnessLevel == 0) {
                brightnessLevel = 2; // Tự động sáng lại mức Cao
                applyBrightness();
            }
            if (!aiEnabled) {
                aiEnabled = true; // Bật lại AI
            }
            isNightMode = false;
        }
    }
}

// 11. Logic Kiểm tra Báo thức
void checkAlarm() {
    if (!alarmEnabled) return;
    
    // Nếu đang kêu rồi thì không cần kiểm tra lại
    if (isAlarmRinging) return;

    int h = hour();
    int m = minute();
    int s = second();

    // Nếu đúng giờ và phút, và giây là 0
    if (h == alarmHour && m == alarmMinute && s == 0) {
        isAlarmRinging = true;
        alarmStartTime = millis(); // [MỚI] Ghi lại thời điểm bắt đầu kêu
        wakeScreen(); // Bật màn hình
    }
}

// 12. [QUAN TRỌNG] HÀM QUẢN LÝ TIẾNG KÊU BÁO THỨC (SMART ALARM)
// Nâng cấp: Tự động tăng mức độ dồn dập nếu không tắt sau 30 giây
void loopAlarmSound() {
    static int lastSoundState = 0; 

    if (isAlarmRinging) {
        unsigned long currentMs = millis();
        // Tính thời gian đã kêu
        unsigned long elapsed = currentMs - alarmStartTime;
        
        // [LOGIC URGENCY] Nếu kêu quá 30 giây, chuyển sang chế độ "Gấp gáp"
        // Chế độ thường: Chu kỳ 1000ms (Bíp chậm)
        // Chế độ gấp: Chu kỳ 300ms (Bíp nhanh liên tục)
        int cycleDuration = (elapsed > 30000) ? 300 : 1000;
        
        unsigned long cycle = currentMs % cycleDuration; 
        int targetState = 0; 

        if (elapsed > 30000) {
             // --- CHẾ ĐỘ GẤP GÁP (Sau 30s) ---
             // Kêu 150ms, nghỉ 150ms
             if (cycle < 150) targetState = 1;
             else targetState = 0;
        } else {
             // --- CHẾ ĐỘ THƯỜNG (Trong trẻo) ---
             // Bíp - Nghỉ - Bíp - Nghỉ dài
             if (cycle < 100) targetState = 1; 
             else if (cycle >= 100 && cycle < 200) targetState = 0;
             else if (cycle >= 200 && cycle < 300) targetState = 1;
             else targetState = 0; 
        }

        // CHỈ GỬI LỆNH KHI TRẠNG THÁI THAY ĐỔI
        if (targetState != lastSoundState) {
            if (targetState == 1) {
                // Nếu gấp gáp, tăng tần số lên 2500Hz cho chói tai hơn chút
                int freq = (elapsed > 30000) ? 2500 : 2048;
                ledcWriteTone(BUZZER_PIN, freq); 
            } else {
                ledcWriteTone(BUZZER_PIN, 0);
            }
            lastSoundState = targetState;
        }

    } else {
        if (lastSoundState != 0) {
            ledcWriteTone(BUZZER_PIN, 0);
            lastSoundState = 0;
        }
    }
}

#endif