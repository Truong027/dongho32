#ifndef SENSOR_APPS_H
#define SENSOR_APPS_H

#include "config.h"
#include "app_functions.h" 

// --- BẢNG MÀU RIÊNG CHO SENSOR APPS ---
#define SA_BG           0x0000 
#define SA_HEADER       0x1082 
#define SA_LINE         0x07FF 
#define SA_TEXT_MAIN    0xFFFF 
#define SA_TEXT_ACCENT  0xFA60 // Màu cam
#define SA_TEXT_OK      0x07E0 // Màu xanh lá
#define SA_TEXT_WARN    0xF800 // Màu đỏ

// --- HÀM HỖ TRỢ CHỜ THẢ NÚT ---
void waitForReleaseSensor(int pin) {
    while (touchRead(pin) < TOUCH_THRESHOLD) {
        delay(10);
    }
}

// --- HÀM VẼ TIÊU ĐỀ CHUNG ---
void drawSensorHeader(String title) {
    tft.fillScreen(SA_BG);
    tft.fillRect(0, 0, 160, 22, SA_HEADER);
    tft.drawFastHLine(0, 22, 160, SA_LINE);
    tft.setFont(NULL);
    tft.setTextSize(1);
    tft.setTextColor(SA_TEXT_MAIN);
    // Căn giữa tiêu đề
    int xPos = (160 - (title.length() * 6)) / 2;
    tft.setCursor(xPos, 7); 
    tft.print(title);
}

// --- HÀM ĐỌC CẢM BIẾN ---
int getToolDistance() {
    digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 25000); // Timeout 25ms (~4m)
    if (duration == 0) return 999;
    return duration * 0.034 / 2;
}

// ======================================================
// 1. THƯỚC ĐO ĐIỆN TỬ (DIGITAL RULER)
// ======================================================
void runDigitalRuler() {
    drawSensorHeader("THUOC DO DIEN TU");
    
    // Nhãn đơn vị tĩnh
    tft.setFont(NULL);
    tft.setTextColor(0x9CD3); // Màu xám xanh
    tft.setCursor(110, 85); tft.print("CM");

    while (true) {
        if (touchRead(TOUCH_MENU) < TOUCH_THRESHOLD) { waitForReleaseSensor(TOUCH_MENU); return; }

        int dist = getToolDistance();
        String distStr = (dist > 400) ? "---" : String(dist);

        // [FIX VỆT CHỮ] Xóa vùng số cũ trước khi vẽ
        tft.fillRect(20, 50, 90, 40, SA_BG); 

        tft.setFont(&FreeSansBold18pt7b);
        tft.setTextColor(SA_TEXT_ACCENT);
        
        // Căn lề phải cho số
        int textW = distStr.length() * 18; 
        tft.setCursor(100 - textW, 85); 
        tft.print(distStr);

        // Thanh Bar trực quan phía dưới
        int barW = map(constrain(dist, 0, 100), 0, 100, 0, 140);
        tft.fillRect(10, 100, 140, 6, 0x2124); // Nền bar
        tft.fillRect(10, 100, barW, 6, SA_TEXT_OK); // Bar giá trị

        delay(150);
    }
}

// ======================================================
// 2. HỖ TRỢ ĐỖ XE (PARKING ASSIST)
// ======================================================
void runParkingAssist() {
    drawSensorHeader("HO TRO DO XE");
    unsigned long lastBeep = 0;

    while (true) {
        if (touchRead(TOUCH_MENU) < TOUCH_THRESHOLD) { 
            ledcWriteTone(BUZZER_PIN, 0); 
            waitForReleaseSensor(TOUCH_MENU); return; 
        }

        int dist = getToolDistance();
        if (dist > 150) dist = 150;

        // Màu sắc cảnh báo
        uint16_t color = SA_TEXT_OK;
        if (dist < 50) color = SA_TEXT_ACCENT; // Vàng/Cam
        if (dist < 20) color = SA_TEXT_WARN;   // Đỏ

        // [FIX VỆT CHỮ] Hiển thị số lớn
        tft.fillRect(60, 40, 90, 40, SA_BG);
        tft.setFont(&FreeSansBold18pt7b);
        tft.setTextColor(color);
        tft.setCursor(70, 75); tft.print(dist);

        // Vẽ đồ thị cột mô phỏng khoảng cách
        int barH = map(dist, 0, 150, 80, 0); // Gần thì cột cao
        tft.fillRect(20, 35, 30, 80, 0x2124); // Xóa cột cũ
        tft.fillRect(20, 35 + (80 - barH), 30, barH, color);

        // Âm thanh Beep
        unsigned long now = millis();
        int beepInterval = dist * 8; 
        if (dist < 10) {
            ledcWriteTone(BUZZER_PIN, 2000); // Kêu liên tục
        } else {
            if (now - lastBeep > beepInterval) {
                ledcWriteTone(BUZZER_PIN, 2000);
                delay(50);
                ledcWriteTone(BUZZER_PIN, 0);
                lastBeep = now;
            } else {
                ledcWriteTone(BUZZER_PIN, 0);
            }
        }
        delay(50);
    }
}

// ======================================================
// 3. BÁO ĐỘNG CHỐNG TRỘM (SECURITY)
// ======================================================
void runSecurityAlarm() {
    drawSensorHeader("CANH BAO AN NINH");
    
    tft.setFont(NULL); tft.setTextSize(1);
    tft.setTextColor(SA_TEXT_WARN);
    tft.setCursor(30, 50); tft.print("KICH HOAT SAU:");
    
    // Đếm ngược đẹp
    for(int i=5; i>0; i--) {
        tft.fillRect(70, 70, 30, 30, SA_BG);
        tft.setFont(&FreeSansBold18pt7b);
        tft.setTextColor(SA_TEXT_MAIN);
        tft.setCursor(70, 100); tft.print(i);
        
        ledcWriteTone(BUZZER_PIN, 1500); delay(100); ledcWriteTone(BUZZER_PIN, 0);
        delay(900);
    }
    
    tft.fillScreen(0x0000);
    tft.setFont(NULL);
    tft.setTextColor(SA_TEXT_OK);
    tft.setCursor(45, 60); tft.print("DA KICH HOAT");
    tft.setTextColor(0x9CD3);
    tft.setCursor(35, 80); tft.print("(Man hinh se tat)");
    delay(1500);
    
    ledcWrite(TFT_BL, 0); // Tắt màn hình
    
    int baseDist = getToolDistance(); 
    if (baseDist > 200) baseDist = 200; 
    bool alarmTriggered = false;

    while(true) {
        if (touchRead(TOUCH_MENU) < TOUCH_THRESHOLD) {
            ledcWrite(TFT_BL, 255);
            ledcWriteTone(BUZZER_PIN, 0);
            waitForReleaseSensor(TOUCH_MENU); return;
        }

        int currentDist = getToolDistance();
        // Phát hiện thay đổi > 30cm hoặc vật cản < 50cm
        if (abs(currentDist - baseDist) > 30 || currentDist < 50) {
            alarmTriggered = true;
        }
        
        if (alarmTriggered) {
            ledcWrite(TFT_BL, 255); // Bật lại màn hình
            
            // Hiệu ứng chớp tắt
            tft.fillScreen(SA_TEXT_WARN); 
            tft.setTextColor(SA_TEXT_MAIN);
            tft.setFont(&FreeSansBold9pt7b);
            tft.setCursor(15, 70); tft.print("PHAT HIEN!");
            
            // Còi hú
            for(int f=800; f<2500; f+=100) { ledcWriteTone(BUZZER_PIN, f); delay(5); }
            
            tft.fillScreen(SA_BG);
            for(int f=2500; f>800; f-=100) { ledcWriteTone(BUZZER_PIN, f); delay(5); }
        }
        delay(100);
    }
}

// ======================================================
// 4. ĐO CHIỀU CAO (HEIGHT MEASURE)
// ======================================================
void runHeightMeasure() {
    drawSensorHeader("DO CHIEU CAO");
    
    // Bước 1: Setup
    int sensorHeight = 180; // Mặc định 1m80
    
    tft.setFont(NULL);
    tft.setTextColor(SA_TEXT_MAIN);
    tft.setCursor(10, 40); tft.print("Chinh do cao Sensor:");
    tft.setCursor(10, 110); tft.print("UP/DN:Chinh  MENU:Chot");

    while(true) {
        // [FIX VỆT CHỮ]
        tft.fillRect(40, 60, 80, 40, SA_BG);
        tft.setFont(&FreeSansBold18pt7b);
        tft.setTextColor(SA_TEXT_ACCENT);
        tft.setCursor(45, 90); tft.print(sensorHeight);
        
        // Điều khiển
        if (touchRead(TOUCH_UP) < TOUCH_THRESHOLD) { sensorHeight++; delay(80); }
        if (touchRead(TOUCH_DOWN) < TOUCH_THRESHOLD) { sensorHeight--; delay(80); }
        if (touchRead(TOUCH_MENU) < TOUCH_THRESHOLD) { waitForReleaseSensor(TOUCH_MENU); break; }
        delay(50);
    }
    
    // Bước 2: Đo
    drawSensorHeader("KET QUA DO");
    tft.setFont(NULL);
    tft.setCursor(35, 40); tft.print("Dung duoi Sensor");

    while(true) {
        if (touchRead(TOUCH_MENU) < TOUCH_THRESHOLD) { waitForReleaseSensor(TOUCH_MENU); return; }
        
        int distHead = getToolDistance();
        int humanHeight = sensorHeight - distHead;
        if (humanHeight < 0) humanHeight = 0;
        
        // [FIX VỆT CHỮ]
        tft.fillRect(20, 60, 120, 50, SA_BG);
        
        tft.setFont(&FreeSansBold18pt7b);
        tft.setTextColor(SA_TEXT_OK);
        
        // Căn giữa số
        String hStr = String(humanHeight);
        int xPos = (160 - (hStr.length() * 20)) / 2;
        tft.setCursor(xPos, 95); 
        tft.print(hStr);
        
        tft.setFont(NULL);
        tft.setTextColor(0x9CD3);
        tft.setCursor(120, 95); tft.print("cm");
        
        delay(300);
    }
}

// ======================================================
// 5. ĐẾM NGƯỜI (PEOPLE COUNTER)
// ======================================================
void runPeopleCounter() {
    drawSensorHeader("BO DEM NGUOI");
    int count = 0;
    bool hasObject = false;
    
    tft.setFont(NULL);
    tft.setTextColor(0x9CD3);
    tft.setCursor(30, 110); tft.print("DN: Reset  MENU: Thoat");

    // Vẽ box số
    tft.drawRoundRect(30, 40, 100, 60, 4, SA_LINE);

    while(true) {
        if (touchRead(TOUCH_MENU) < TOUCH_THRESHOLD) { waitForReleaseSensor(TOUCH_MENU); return; }
        
        // Reset
        if (touchRead(TOUCH_DOWN) < TOUCH_THRESHOLD) { 
            count = 0; 
            tft.fillRect(32, 42, 96, 56, SA_BG); // Xóa số cũ
            waitForReleaseSensor(TOUCH_DOWN); 
        }

        int dist = getToolDistance();
        // Ngưỡng phát hiện: < 80cm
        if (dist < 80 && dist > 0) {
            if (!hasObject) {
                count++;
                hasObject = true;
                // Hiệu ứng khi đếm
                tft.fillRoundRect(30, 40, 100, 60, 4, 0x2124); // Nháy nền nhẹ
                ledcWriteTone(BUZZER_PIN, 2000); delay(50); ledcWriteTone(BUZZER_PIN, 0);
            }
        } else {
            hasObject = false;
            tft.drawRoundRect(30, 40, 100, 60, 4, SA_LINE); // Trả nền đen
        }
        
        // [FIX VỆT CHỮ]
        // Chỉ vẽ lại khi cần thiết hoặc dùng fillRect vùng nhỏ bên trong
        // Ở đây ta vẽ liên tục nhưng có fillRect
        tft.fillRect(35, 50, 90, 40, SA_BG); // Xóa vùng chữ bên trong khung
        
        tft.setFont(&FreeSansBold18pt7b);
        tft.setTextColor(SA_TEXT_MAIN);
        String cStr = String(count);
        int xPos = (160 - (cStr.length() * 20)) / 2;
        tft.setCursor(xPos, 85); tft.print(cStr);
        
        delay(100);
    }
}

// ======================================================
// 6. CẢNH BÁO TƯ THẾ (POSTURE ALERT)
// ======================================================
void runPostureAlert() {
    drawSensorHeader("CANH BAO TU THE");
    tft.setFont(NULL);
    tft.setCursor(25, 40); tft.print("Dat tren ban...");
    tft.setCursor(15, 55); tft.print("Canh bao neu < 45cm");
    delay(2000);
    
    tft.fillScreen(SA_BG);

    while(true) {
        if (touchRead(TOUCH_MENU) < TOUCH_THRESHOLD) { 
            ledcWriteTone(BUZZER_PIN, 0);
            waitForReleaseSensor(TOUCH_MENU); return; 
        }
        
        int dist = getToolDistance();
        
        // Hiển thị khoảng cách hiện tại nhỏ ở góc
        tft.fillRect(0, 0, 60, 20, SA_BG);
        tft.setFont(NULL); tft.setTextColor(0x9CD3);
        tft.setCursor(5, 10); tft.print("Dist: " + String(dist));

        if (dist < 45 && dist > 2) { // Quá gần
            // Cảnh báo đỏ
            tft.fillRect(0, 30, 160, 98, SA_TEXT_WARN);
            tft.setFont(&FreeSansBold9pt7b);
            tft.setTextColor(SA_TEXT_MAIN);
            tft.setCursor(25, 75); tft.print("NGOI XA RA!");
            tft.setCursor(45, 95); tft.print("HAI MAT");
            
            ledcWriteTone(BUZZER_PIN, 500); delay(200); ledcWriteTone(BUZZER_PIN, 0);
        } else {
            // An toàn xanh
            tft.fillRect(0, 30, 160, 98, SA_TEXT_OK);
            tft.setFont(&FreeSansBold9pt7b);
            tft.setTextColor(SA_BG); // Chữ đen trên nền xanh
            tft.setCursor(35, 85); tft.print("TOT LAM");
        }
        delay(300);
    }
}

// ======================================================
// MENU CHÍNH (GIAO DIỆN SCROLL)
// ======================================================
void runSensorMenu() {
    const char* items[] = {
        "THUOC DO", 
        "HO TRO DO XE", 
        "CHONG TROM", 
        "DO CHIEU CAO", 
        "DEM NGUOI", 
        "CANH BAO TU THE"
    };
    int count = 6;
    int cursor = 0;
    bool needRedraw = true;
    
    while(true) {
        if (needRedraw) {
            drawSensorHeader("TIEN ICH CAM BIEN");
            
            // Logic cuộn: Hiển thị 4 mục
            int startIdx = 0;
            if (cursor >= 3) startIdx = cursor - 3;
            if (startIdx > count - 4) startIdx = count - 4;
            if (startIdx < 0) startIdx = 0;

            for(int i=0; i<4; i++) {
                if (startIdx + i >= count) break;
                
                int idx = startIdx + i;
                int y = 35 + i*22; // Vị trí Y
                
                if (idx == cursor) {
                    tft.fillRoundRect(10, y, 140, 20, 4, SA_LINE); // Highlight
                    tft.setTextColor(SA_BG);
                } else {
                    tft.drawRoundRect(10, y, 140, 20, 4, 0x2124); // Khung mờ
                    tft.setTextColor(SA_TEXT_MAIN);
                }
                
                tft.setFont(NULL);
                tft.setCursor(20, y+6); 
                tft.print(items[idx]);
                
                // Vẽ thanh cuộn bên phải
                int barH = 80;
                int scrollY = 35 + map(idx, 0, count-1, 0, barH-10);
                if (i==0) tft.fillRect(156, 35, 2, barH, 0x2124); // Xóa thanh cuộn cũ
                if (idx == cursor) tft.fillRect(156, scrollY, 2, 10, SA_TEXT_ACCENT);
            }
            needRedraw = false;
        }
        
        delay(50); 
        
        if (touchRead(TOUCH_MENU) < TOUCH_THRESHOLD) {
            waitForReleaseSensor(TOUCH_MENU);
            
            // [FIX] Reset biến trạng thái UI để vẽ lại System Bar (RAM, WiFi) khi thoát
            // Sử dụng extern để truy cập biến từ ui_display.h
            extern int old_wifi_bars;
            extern char old_ram[];
            
            old_wifi_bars = -1;  // Buộc vẽ lại WiFi
            old_ram[0] = '\0';   // Buộc vẽ lại RAM (chuỗi rỗng khác với chuỗi hiện tại)
            
            return; 
        }

        if (touchRead(TOUCH_DOWN) < TOUCH_THRESHOLD) {
            cursor = (cursor + 1) % count;
            needRedraw = true;
            beep();
            waitForReleaseSensor(TOUCH_DOWN);
        }

        if (touchRead(TOUCH_UP) < TOUCH_THRESHOLD) { 
            beep();
            waitForReleaseSensor(TOUCH_UP);
            
            // Chạy ứng dụng tương ứng
            switch(cursor) {
                case 0: runDigitalRuler(); break;
                case 1: runParkingAssist(); break;
                case 2: runSecurityAlarm(); break;
                case 3: runHeightMeasure(); break;
                case 4: runPeopleCounter(); break;
                case 5: runPostureAlert(); break;
            }
            
            // Khi thoát app, vẽ lại menu
            needRedraw = true; 
        }
    }
}

#endif