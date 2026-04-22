#ifndef UI_DISPLAY_H
#define UI_DISPLAY_H

#include "config.h"
#include <WiFi.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h> 

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// --- MODERN UI PALETTE ---
#define C_BG           0x0000 
#define C_HEADER_BG    0x1082 
#define C_HEADER_ERR   0xC000 
#define C_CARD_BG      0x18E3 
#define C_CARD_SHADOW  0x0861 
#define C_ACCENT_CYAN  0x07FF 
#define C_ACCENT_ORG   0xFA60 
#define C_ACTIVE       0x05E0 
#define C_INACTIVE     0x2965 
#define C_TEXT_MAIN    0xFFFF 
#define C_TEXT_SUB     0x9CD3 
#define C_BORDER       0x4A69 

int old_h = -1, old_m = -1;
char old_ram[10] = ""; 
int old_wifi_bars = -1;
bool old_r1 = false, old_r2 = false;

void initDisplay() {
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(C_BG);
    ledcAttach(TFT_BL, 5000, 8); 
    ledcWrite(TFT_BL, 255);
}

// [MỚI] Màn hình khởi động (Boot Screen)
void drawBootScreen(String status) {
    // Chỉ vẽ nền 1 lần nếu cần (hoặc người dùng tự clear trước khi gọi)
    // Ở đây ta giả sử người dùng gọi 1 lần hoặc update trạng thái
    
    // Logo / Tiêu đề
    tft.setFont(&FreeSansBold12pt7b);
    tft.setTextColor(C_ACCENT_CYAN);
    int wTitle = 110; // Ước lượng chiều rộng "SMART CLOCK"
    tft.setCursor((160 - wTitle)/2, 50);
    tft.print("SMART");
    
    tft.setTextColor(C_ACCENT_ORG);
    tft.setCursor((160 - 80)/2, 75); // "CLOCK"
    tft.print("CLOCK");

    // Thanh loading giả lập
    tft.drawRect(30, 90, 100, 10, C_BORDER);
    
    // Vẽ trạng thái text
    tft.fillRect(0, 105, 160, 20, C_BG); // Xóa dòng status cũ
    tft.setFont(NULL);
    tft.setTextColor(C_TEXT_SUB);
    
    // Căn giữa text status
    int wStatus = status.length() * 6;
    tft.setCursor((160 - wStatus)/2, 110);
    tft.print(status);
}

// [MỚI] Hàm cập nhật thanh loading (để tạo hiệu ứng chạy)
void updateBootProgress(int percent) {
    int w = map(percent, 0, 100, 0, 96);
    tft.fillRect(32, 92, w, 6, C_ACCENT_CYAN);
}

// 1. THANH TRẠNG THÁI
void drawSystemBar(bool wifi, uint32_t freeHeap) {
    int bars = 0;
    if (wifi) {
        long rssi = WiFi.RSSI();
        if (rssi > -55) bars = 4;
        else if (rssi > -65) bars = 3;
        else if (rssi > -75) bars = 2;
        else bars = 1;
    }

    char ramBuffer[10];
    snprintf(ramBuffer, sizeof(ramBuffer), "%dKB", freeHeap / 1024);
    
    if (strcmp(ramBuffer, old_ram) != 0 || bars != old_wifi_bars) {
        strcpy(old_ram, ramBuffer);
        old_wifi_bars = bars;

        uint16_t headerColor = (bars == 0 && wifi) ? C_HEADER_ERR : C_HEADER_BG;
        tft.fillRect(0, 0, 160, 20, headerColor); 
        
        tft.setFont(NULL);
        tft.setTextColor(C_TEXT_SUB);
        tft.setCursor(4, 6); tft.print(F("MEM:")); 
        tft.setTextColor(C_ACCENT_CYAN); 
        tft.print(ramBuffer);

        int xBar = 135;
        int yBarBase = 16; 
        
        if (bars > 0) {
            for (int i = 0; i < 4; i++) {
                int h = (i + 1) * 3;
                uint16_t color = C_INACTIVE;
                if (i < bars) color = (i < 2) ? C_ACCENT_ORG : C_ACCENT_CYAN;
                tft.fillRoundRect(xBar + (i * 5), yBarBase - h, 3, h, 1, color);
            }
        } else {
            tft.setTextColor(0xFFFF); 
            tft.setCursor(xBar - 10, 6);
            tft.print(F("NoWifi"));
        }
    }
}

// 2. ĐỒNG HỒ
void drawFlipDigit(int x, int y, int num, bool refresh) {
    char strNum[3];
    snprintf(strNum, sizeof(strNum), "%02d", num);
    
    if (refresh) {
        tft.fillRoundRect(x, y, 68, 58, 6, C_BG); 
        delay(20); 
    }

    tft.fillRoundRect(x, y, 68, 58, 6, C_CARD_BG);
    tft.drawRoundRect(x, y, 68, 58, 6, C_BORDER); 

    tft.fillRect(x+2, y+2, 64, 26, 0x2145); 
    tft.fillRect(x+2, y+28, 64, 2, C_BG);

    tft.setFont(&FreeSansBold18pt7b);
    tft.setTextSize(1);
    
    int16_t x1, y1; uint16_t tw, th;
    tft.getTextBounds(strNum, 0, 0, &x1, &y1, &tw, &th);
    
    tft.setTextColor(C_CARD_SHADOW); 
    tft.setCursor(x + (68 - tw)/2 + 2, y + 47); tft.print(strNum); 
    tft.setTextColor(C_TEXT_MAIN);
    tft.setCursor(x + (68 - tw)/2, y + 45); tft.print(strNum); 
}

void drawClock(int h, int m, bool force) {
    int yPos = 22; 
    if (h != old_h || force) {
        drawFlipDigit(8, yPos, h, (h != old_h) && !force); 
        old_h = h;
    }
    
    static bool lastTick = false;
    bool currentTick = (millis() / 1000) % 2 == 0; 
    
    if (lastTick != currentTick || force) {
        uint16_t col = currentTick ? C_TEXT_MAIN : 0x2124; 
        tft.fillCircle(82, yPos + 20, 3, col);
        tft.fillCircle(82, yPos + 40, 3, col);
        lastTick = currentTick;
    }

    if (m != old_m || force) {
        drawFlipDigit(88, yPos, m, (m != old_m) && !force);
        old_m = m;
    }
}

// 3. THANH NGÀY THÁNG
void drawDateBar(int y, const char* dayOfWeek, const char* dayDate, const char* lunarStr) {
    tft.fillRect(0, y, 160, 24, C_BG); 

    tft.fillRoundRect(2, y + 2, 32, 20, 6, C_ACCENT_ORG);
    tft.setFont(NULL);
    tft.setTextColor(0x0000); 
    
    int len = strlen(dayOfWeek);
    int xOffset = (len > 2) ? 4 : 9; 
    tft.setCursor(2 + xOffset, y + 6);
    tft.print(dayOfWeek);

    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(C_TEXT_MAIN);
    tft.setCursor(40, y + 17); 
    tft.print(dayDate);

    if (lunarStr != NULL && strlen(lunarStr) > 0) {
        tft.setFont(NULL); 
        int w2 = strlen(lunarStr) * 6 + 10; 
        int xLunar = 160 - w2 - 2;
        
        tft.fillRoundRect(xLunar, y + 2, w2, 20, 6, C_ACCENT_CYAN); 
        tft.setTextColor(0x0000); 
        tft.setCursor(xLunar + 5, y + 6); 
        tft.print(lunarStr);
    }
}

// 4. NÚT BẤM RELAY
void drawRelays(bool r1, bool r2, bool force) {
    if (r1 == old_r1 && r2 == old_r2 && !force) return;
    int yPos = 106; 
    int hBtn = 16;
    
    tft.fillRect(0, yPos - 2, 160, hBtn + 4, C_BG); 

    uint16_t c1 = r1 ? C_ACTIVE : C_INACTIVE;
    uint16_t tc1 = r1 ? 0xFFFF : C_TEXT_SUB;
    tft.fillRoundRect(4, yPos, 74, hBtn, 8, c1); 
    if (r1) tft.fillCircle(12, yPos + hBtn/2, 3, 0xFFFF);
    else tft.drawCircle(12, yPos + hBtn/2, 3, C_TEXT_SUB);
    tft.setFont(NULL); tft.setTextColor(tc1);
    tft.setCursor(25, yPos + 4); tft.print(F("LED"));

    uint16_t c2 = r2 ? C_ACCENT_CYAN : C_INACTIVE; 
    uint16_t tc2 = r2 ? 0x0000 : C_TEXT_SUB;
    tft.fillRoundRect(82, yPos, 74, hBtn, 8, c2);
    if (r2) tft.fillCircle(90, yPos + hBtn/2, 3, 0x0000);
    else tft.drawCircle(90, yPos + hBtn/2, 3, C_TEXT_SUB);
    tft.setTextColor(tc2);
    tft.setCursor(105, yPos + 4); tft.print(F("AC"));

    old_r1 = r1; old_r2 = r2;
}

// 5. TỔNG HỢP MÀN HÌNH CHÍNH
void drawMainScreen(int h, int m, const char* dayOfWeek, const char* dateStr, const char* lunarStr, bool r1, bool r2, bool force) {
    drawClock(h, m, force);

    if (force || m != old_m) { 
         drawDateBar(82, dayOfWeek, dateStr, lunarStr);
    }

    drawRelays(r1, r2, force);
}

void drawProgressBar(unsigned long elapsed, unsigned long duration) {
    if (elapsed > duration) elapsed = duration;
    int width = map(elapsed, 0, duration, 0, 160);
    tft.drawFastHLine(0, 127, width, C_ACCENT_CYAN);
    if (width < 160) tft.drawFastHLine(width, 127, 160 - width, C_BG);
}

#endif