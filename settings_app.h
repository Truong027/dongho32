#ifndef SETTINGS_APP_H
#define SETTINGS_APP_H

#include "config.h"
#include "ai_assistant.h"

// --- MÀU SẮC SETTINGS ---
#define S_BG           0x0000
extern uint16_t themeColor; 
#define S_TEXT_NORM    0xFFFF
#define S_TEXT_SEL     0x0000 
#define S_ITEM_BG      0x2124
#define S_POPUP_BG     0x1082 
#define S_POPUP_BORDER 0xFFFF 

// --- BIẾN TOÀN CỤC ---
extern int brightnessLevel; 
extern bool rainAlertEnabled;
extern bool autoScrollEnabled;
extern bool isFlipped;
extern int timeFormat;        
extern int screenTimeoutMode; 
extern int menuCursor; 
extern int pendingAction; 

// Biến Pro
extern int touchSensitivity;
extern int themeIndex; 
extern unsigned long lastLoopTime; 
extern bool aiEnabled; 
// [MỚI] Biến tính năng mới (Cần khai báo trong main)
extern bool soundEnabled; 
extern int aiTalkMode; // 0: Silent, 1: Normal, 2: Chatty

// Biến Báo thức
extern int alarmHour;
extern int alarmMinute;
extern bool alarmEnabled;

// [MỚI] Biến tùy chỉnh giờ ban đêm
extern bool nightModeEnabled; // Biến bật/tắt
extern int nightStartHour; 
extern int nightEndHour;   

// Danh sách tên màu chủ đề
const uint16_t THEME_COLORS[] = {0x07FF, 0xFA60, 0x07E0, 0xF81F};
const String THEME_NAMES[]    = {"CYAN", "CAM", "LUC", "TIM"};

// [MỚI] Danh sách tên Skin AI - Thêm Pokemon & Anime (Đảm bảo khớp với ai_core.h)
const String SKIN_NAMES[]     = {"PACMAN", "GHOST", "ROBOT", "POKEMON", "ANIME"};

// --- [UI] HÀM VẼ 1 DÒNG MENU ---
void drawSettingItem(int y, String label, String value, bool isSelected) {
    uint16_t color = THEME_COLORS[themeIndex]; 
    if (isSelected) {
        tft.fillRoundRect(2, y, 156, 18, 4, color);
        tft.setTextColor(S_TEXT_SEL);
    } else {
        tft.fillRoundRect(2, y, 156, 18, 4, S_ITEM_BG);
        tft.setTextColor(S_TEXT_NORM);
    }
    tft.setFont(NULL);
    tft.setTextSize(1);
    tft.setCursor(6, y + 5);
    tft.print(label);
    if (value.length() > 10) value = value.substring(0, 9) + ".";
    int valWidth = value.length() * 6;
    tft.setCursor(150 - valWidth, y + 5);
    tft.print(value);
}

// --- [UI] VẼ MÀN HÌNH SETTINGS CHÍNH ---
void drawSettingsScreen(bool force) {
    static int oldCursor = -1;
    static int oldBright = -1;
    static int oldTheme = -1;
    static bool oldRain = false;
    static bool oldAuto = false;
    static bool oldFlip = false;
    static int oldTimeFmt = -1;
    static int oldSens = -1;
    static int oldTimeout = -1;
    static int oldPending = -1;
    static bool oldAi = false;
    
    // Biến kiểm tra báo thức
    static int oldAH = -1;
    static int oldAM = -1;
    static bool oldAEn = false;

    // Biến kiểm tra giờ đêm
    static bool oldNME = false;
    static int oldNSH = -1;
    static int oldNEH = -1;

    // [MỚI] Biến kiểm tra tính năng mới
    static bool oldSound = true;
    static int oldAiMode = -1;
    static int oldSkin = -1; // [MỚI] Check thay đổi Skin

    bool changed = false;
    if (menuCursor != oldCursor) changed = true;
    if (brightnessLevel != oldBright) changed = true;
    if (themeIndex != oldTheme) changed = true;
    if (rainAlertEnabled != oldRain) changed = true;
    if (autoScrollEnabled != oldAuto) changed = true;
    if (isFlipped != oldFlip) changed = true;
    if (timeFormat != oldTimeFmt) changed = true;
    if (touchSensitivity != oldSens) changed = true;
    if (screenTimeoutMode != oldTimeout) changed = true;
    if (pendingAction != oldPending) changed = true;
    if (aiEnabled != oldAi) changed = true;
    
    if (alarmHour != oldAH) changed = true;
    if (alarmMinute != oldAM) changed = true;
    if (alarmEnabled != oldAEn) changed = true;

    // Check thay đổi giờ đêm
    if (nightModeEnabled != oldNME) changed = true;
    if (nightStartHour != oldNSH) changed = true;
    if (nightEndHour != oldNEH) changed = true;

    // Check tính năng mới
    if (soundEnabled != oldSound) changed = true;
    if (aiTalkMode != oldAiMode) changed = true;
    if (currentSkin != oldSkin) changed = true; // [MỚI]

    if (!force && !changed) return;
    
    oldCursor = menuCursor; oldBright = brightnessLevel; oldTheme = themeIndex;
    oldRain = rainAlertEnabled; oldAuto = autoScrollEnabled; oldFlip = isFlipped;
    oldTimeFmt = timeFormat; oldSens = touchSensitivity; oldTimeout = screenTimeoutMode;
    oldPending = pendingAction; oldAi = aiEnabled;
    oldAH = alarmHour; oldAM = alarmMinute; oldAEn = alarmEnabled;
    oldNME = nightModeEnabled; oldNSH = nightStartHour; oldNEH = nightEndHour;
    oldSound = soundEnabled; oldAiMode = aiTalkMode; oldSkin = currentSkin;

    uint16_t currentThemeColor = THEME_COLORS[themeIndex];

    if (force || pendingAction == 0) {
        tft.setFont(NULL);
        tft.setTextSize(1);
        tft.setTextColor(0xFFFF);
        if (force) {
             tft.fillRect(0, 20, 160, 15, 0x0000); 
             tft.setCursor(30, 24); tft.print("CAI DAT HE THONG");
             tft.drawFastHLine(10, 36, 140, currentThemeColor);
        }
    }

    int itemsPerPage = 4;
    // [QUAN TRỌNG] Tăng tổng số mục lên 25 để hiển thị được mục Skin (index 20) và các mục sau đó
    int totalItems = 25; 
    int yStart = 42;
    int hItem = 21;

    static int startRow = 0;
    if (menuCursor >= startRow + itemsPerPage) startRow = menuCursor - itemsPerPage + 1;
    else if (menuCursor < startRow) startRow = menuCursor;

    if (pendingAction == 0) { 
        for (int i = 0; i < itemsPerPage; i++) {
            int itemIndex = startRow + i;
            if (itemIndex >= totalItems) break;

            int y = yStart + (i * hItem);
            bool isSel = (menuCursor == itemIndex);
            
            switch (itemIndex) {
                // --- CÀI ĐẶT CƠ BẢN ---
                case 0: { String val = (brightnessLevel == 0) ? "THAP" : ((brightnessLevel == 1) ? "VUA" : "CAO"); drawSettingItem(y, "Do sang", val, isSel); } break;
                case 1: drawSettingItem(y, "Mau chu de", THEME_NAMES[themeIndex], isSel); break;
                case 2: drawSettingItem(y, "Canh bao mua", rainAlertEnabled ? "BAT" : "TAT", isSel); break;
                case 3: drawSettingItem(y, "Tu dong chuyen", autoScrollEnabled ? "BAT" : "TAT", isSel); break;
                case 4: drawSettingItem(y, "Xoay man hinh", isFlipped ? "180" : "0", isSel); break;
                case 5: drawSettingItem(y, "Dinh dang gio", (timeFormat == 0) ? "24H" : "12H", isSel); break;
                case 6: { String tS = (touchSensitivity==0)?"THAP":((touchSensitivity==1)?"VUA":"CAO"); drawSettingItem(y, "Do nhay cam ung", tS, isSel); } break;
                case 7: { String tO = (screenTimeoutMode==0)?"KHONG":((screenTimeoutMode==1)?"15S":((screenTimeoutMode==2)?"30S":"60S")); drawSettingItem(y, "Tu tat man hinh", tO, isSel); } break;
                
                // --- HỆ THỐNG & KẾT NỐI ---
                case 8: drawSettingItem(y, "WiFi Nang cao", ">>", isSel); break;
                case 9: drawSettingItem(y, "Chan doan HT", ">>", isSel); break;
                case 10: drawSettingItem(y, "Tro ly AI", aiEnabled ? "BAT" : "TAT", isSel); break;
                case 11: drawSettingItem(y, "Khoi dong lai", ">>", isSel); break;
                
                // --- BÁO THỨC ---
                case 12: { 
                    char s[10]; sprintf(s, "%02d:00", alarmHour);
                    drawSettingItem(y, "Hen gio (Gio)", String(s), isSel); 
                } break;
                case 13: { 
                    char s[10]; sprintf(s, "00:%02d", alarmMinute);
                    drawSettingItem(y, "Hen gio (Phut)", String(s), isSel); 
                } break;
                case 14: drawSettingItem(y, "Bat bao thuc", alarmEnabled ? "BAT" : "TAT", isSel); break;

                // --- CHẾ ĐỘ BAN ĐÊM ---
                case 15: {
                    drawSettingItem(y, "Che do dem", nightModeEnabled ? "BAT" : "TAT", isSel);
                } break;
                case 16: {
                    char s[10]; sprintf(s, "%02dH", nightStartHour);
                    drawSettingItem(y, "Gio Bat Dem", String(s), isSel);
                } break;
                case 17: {
                    char s[10]; sprintf(s, "%02dH", nightEndHour);
                    drawSettingItem(y, "Gio Tat Dem", String(s), isSel);
                } break;

                // --- [MỚI] TÍNH NĂNG NÂNG CAO ---
                case 18: drawSettingItem(y, "Am thanh", soundEnabled ? "BAT" : "TAT", isSel); break;
                case 19: {
                    String modeStr = "VUA";
                    if (aiTalkMode == 0) modeStr = "IM LANG";
                    else if (aiTalkMode == 2) modeStr = "NOI NHIEU";
                    drawSettingItem(y, "Tan suat AI", modeStr, isSel);
                } break;
                
                // [MỚI] MỤC SKIN AI (Kiểm tra kỹ index này)
                case 20: drawSettingItem(y, "Giao dien AI", SKIN_NAMES[currentSkin], isSel); break;

                // --- KHO GAME & ỨNG DỤNG ---
                case 21: drawSettingItem(y, "CUU SOS", "BAT", isSel); break;
                case 22: drawSettingItem(y, "KHO TRO CHOI", ">>", isSel); break;
                case 23: drawSettingItem(y, "TIEN ICH CAM BIEN", ">>", isSel); break;
                case 24: drawSettingItem(y, "THONG TIN", ">>", isSel); break;
            }
        }
        
        // Thanh cuộn (Scrollbar)
        if (totalItems > itemsPerPage) {
            int barTotalH = 80;
            tft.fillRect(158, 42, 2, barTotalH, 0x2124); 
            int trackY = 42 + map(startRow, 0, totalItems - itemsPerPage, 0, barTotalH - 20);
            tft.fillRect(158, trackY, 2, 20, currentThemeColor); 
        }
    }

    // --- POPUP ---
    if (pendingAction > 0) {
        int wBox = 140; int hBox = 80;
        int xBox = (160 - wBox) / 2; int yBox = (128 - hBox) / 2;
        
        tft.fillRoundRect(xBox, yBox, wBox, hBox, 6, S_POPUP_BG);
        tft.drawRoundRect(xBox, yBox, wBox, hBox, 6, currentThemeColor);
        tft.setFont(NULL); tft.setTextSize(1); tft.setTextColor(0xFFFF);

        if (pendingAction == 1) { // WIFI
            tft.setTextColor(currentThemeColor);
            tft.setCursor(xBox + 30, yBox + 5); tft.print("WIFI MENU");
            tft.setTextColor(0xFFFF);
            tft.setCursor(xBox + 10, yBox + 25); tft.print("RSSI: " + String(WiFi.RSSI()) + " dBm");
            tft.setCursor(xBox + 10, yBox + 38); tft.print("IP: " + WiFi.localIP().toString());
            tft.setTextColor(0xF800);
            tft.setCursor(xBox + 10, yBox + 55); tft.print("An DN de XOA WIFI");
            tft.setCursor(xBox + 10, yBox + 65); tft.print("An UP de THOAT");
        }
        else if (pendingAction == 4) { // CHAN DOAN - [NÂNG CẤP]
            tft.setTextColor(currentThemeColor);
            tft.setCursor(xBox + 40, yBox + 5); tft.print("HE THONG");
            tft.setTextColor(0xFFFF);
            // Hiển thị thời gian chạy (Uptime)
            long runSeconds = millis() / 1000;
            int runH = runSeconds / 3600;
            int runM = (runSeconds % 3600) / 60;
            tft.setCursor(xBox + 10, yBox + 20); tft.print("Run: " + String(runH) + "h " + String(runM) + "m");
            tft.setCursor(xBox + 10, yBox + 32); tft.print("RAM: " + String(ESP.getFreeHeap()/1024) + " KB");
            tft.setCursor(xBox + 10, yBox + 44); tft.print("Loop: " + String(lastLoopTime) + "us");
            tft.setTextColor(0xAAAA);
            tft.setCursor(xBox + 35, yBox + 65); tft.print("BAM DE THOAT");
        }
        else if (pendingAction == 2) { // REBOOT
             tft.setCursor(xBox + 40, yBox + 10); tft.print("KHOI DONG?");
             tft.setTextColor(0xF800);
             tft.setCursor(xBox + 30, yBox + 40); tft.print("DN: DONG Y");
             tft.setTextColor(0xFFFF);
             tft.setCursor(xBox + 30, yBox + 55); tft.print("UP: HUY BO");
        }
        else if (pendingAction == 5) { // [MỚI] THÔNG TIN - BINSMARTCLOCK
             tft.setTextColor(currentThemeColor);
             // Căn giữa tiêu đề (BINSMARTCLOCK ~ 13 ký tự)
             tft.setCursor(xBox + 25, yBox + 5); tft.print("BINSMARTCLOCK");
             
             tft.setTextColor(0xFFFF);
             tft.setCursor(xBox + 10, yBox + 25); tft.print("Dev: BIN DEP TRAi");
             tft.setCursor(xBox + 10, yBox + 40); tft.print("Build: 2026");
             
             tft.setTextColor(0xAAAA);
             tft.setCursor(xBox + 35, yBox + 60); tft.print("BAM DE THOAT");
        }
    }
}

#endif