#ifndef CREATIVE_APPS_H
#define CREATIVE_APPS_H

#include "config.h"
#include <Preferences.h>
#include <vector>

// --- LIÊN KẾT VỚI FILE KHÁC ---
// Gọi hàm từ game_app.h (nếu có)
extern void runReflexGame(); 
// Gọi hàm từ ai_extensions.h (Menu Shop)
extern void runShopMenu(); 

// --- CẤU HÌNH CHUNG ---
#define CHIP_START      1000000 // [MOD] Tăng vốn khởi nghiệp lên 5000
#define GAME_SPEED      30   

// --- RESOURCE MÀU SẮC ---
#define C_CARD_BG       0xFFFF 
#define C_CARD_RED      0xF800
#define C_CARD_BLACK    0x0000
#define C_TABLE_FELT    0x03E0 

// --- HÀM HỖ TRỢ ÂM THANH & INPUT ---
void playToneGame(int freq, int duration) {
    if (freq > 0) {
        ledcWriteTone(BUZZER_PIN, freq);
        delay(duration);
        ledcWriteTone(BUZZER_PIN, 0);
    } else {
        delay(duration);
    }
}

// Hàm đọc nút cảm ứng
bool isPressed(int pin) {
    return touchRead(pin) < TOUCH_THRESHOLD;
}

// Hàm chờ nhả nút (Fix lỗi vào game bị thoát ngay)
void waitForRelease() {
    while(isPressed(TOUCH_MENU) || isPressed(TOUCH_UP) || isPressed(TOUCH_DOWN)) {
        delay(10);
    }
}

// --- QUẢN LÝ CHIP (TIỀN TỆ) ---
Preferences casinoPrefs;

int getChips() {
    casinoPrefs.begin("casino", false);
    int chips = casinoPrefs.getInt("chips", CHIP_START);
    
    // [MOD] CƠ CHẾ BẢO HIỂM PHÁ SẢN (Hết tiền tự động bơm)
    if (chips < 50) {
        chips = CHIP_START; // Bơm lại đầy bình
        casinoPrefs.putInt("chips", chips);
    }
    
    casinoPrefs.end();
    return chips;
}

void updateChips(int delta) {
    casinoPrefs.begin("casino", false);
    int current = casinoPrefs.getInt("chips", CHIP_START);
    int newVal = current + delta;
    if (newVal < 0) newVal = 0;
    casinoPrefs.putInt("chips", newVal);
    casinoPrefs.end();
}

// --- [MỚI] HÀM CHỌN MỨC CƯỢC ---
// Trả về mức cược đã chọn, hoặc 0 nếu thoát
int selectBetAmount() {
    int bets[] = {10, 20, 50, 100, 200, 500, 1000};
    int betIndex = 2; // Mặc định 50
    int totalOptions = 7;
    
    waitForRelease();
    
    while (true) {
        tft.fillScreen(0x0000);
        tft.drawRect(20, 30, 120, 60, 0x07FF);
        
        tft.setTextColor(0xFFFF);
        tft.setCursor(45, 40); tft.print("CHON CUOC");
        
        // Hiển thị mức cược hiện tại
        tft.setTextSize(2);
        tft.setTextColor(0xFD20); // Màu cam vàng
        tft.setCursor(50, 60); 
        tft.print(String(bets[betIndex]) + "$");
        tft.setTextSize(1);
        
        tft.setTextColor(0xAAAA);
        tft.setCursor(30, 100); tft.print("UP/DN: +/-");
        tft.setCursor(30, 110); tft.print("MENU: OK");
        
        // Input loop với độ trễ để dễ chọn
        unsigned long startWait = millis();
        while (millis() - startWait < 150) { // Debounce nhẹ
            if (isPressed(TOUCH_MENU)) {
                waitForRelease();
                return bets[betIndex]; // Chốt mức cược
            }
        }
        
        if (isPressed(TOUCH_UP)) {
            betIndex = (betIndex + 1) % totalOptions;
        }
        if (isPressed(TOUCH_DOWN)) {
            betIndex = (betIndex - 1 + totalOptions) % totalOptions;
        }
        delay(150); // Chậm lại chút để dễ bấm
    }
}

// ======================================================
// GAME 1: FLAPPY BIRD (PLAY TO EARN: 1 điểm = 10$)
// ======================================================
void runFlappyBird() {
    float birdY = 60;
    float oldBirdY = 60;
    float velocity = 0;
    float gravity = 0.6;
    float lift = -4.5;
    int birdX = 30;
    int birdSize = 10;
    int score = 0;
    
    struct Pipe { float x; float oldX; int gapY; bool passed; };
    std::vector<Pipe> pipes;
    pipes.push_back({160, 160, random(20, 80), false});
    
    int pipeW = 18;
    int gapH = 42; 
    float pipeSpeed = 3.0;

    bool gameOver = false;
    unsigned long lastFrame = 0;
    
    uint16_t bgColor = 0x87CE; 
    tft.fillScreen(bgColor); 
    
    int groundY = 120;
    tft.fillRect(0, groundY, 160, 8, 0xDDB2); 
    tft.drawFastHLine(0, groundY, 160, 0x07E0); 

    while (!gameOver) {
        if (isPressed(TOUCH_MENU)) { tft.fillScreen(0); return; }

        if (millis() - lastFrame > 30) { 
            lastFrame = millis();
            
            if (isPressed(TOUCH_UP) || isPressed(TOUCH_DOWN)) {
                velocity = lift;
                playToneGame(400, 20);
            }
            
            velocity += gravity;
            oldBirdY = birdY; 
            birdY += velocity;
            
            if (birdY < 0) birdY = 0;
            if (birdY > groundY - birdSize) { 
                birdY = groundY - birdSize; 
                gameOver = true; 
            }

            for (int i = 0; i < pipes.size(); i++) {
                pipes[i].oldX = pipes[i].x; 
                pipes[i].x -= pipeSpeed;
            }

            if (pipes.front().x < -pipeW) pipes.erase(pipes.begin());
            
            if (pipes.back().x < 160 - 70) { 
                pipes.push_back({160, 160, random(10, groundY - gapH - 10), false});
            }

            // RENDER
            int minY = min((int)oldBirdY, (int)birdY) - 2;
            int maxY = max((int)oldBirdY, (int)birdY) + birdSize + 2;
            tft.fillRect(birdX, (int)oldBirdY, birdSize+2, birdSize, bgColor); 
            
            tft.fillRect(birdX, (int)birdY, birdSize, birdSize - 2, 0xFFE0); 
            tft.fillRect(birdX+7, (int)birdY+2, 4, 3, 0xF800); 
            tft.drawPixel(birdX+7, (int)birdY+1, 0x0000); 
            tft.drawFastHLine(birdX+2, (int)birdY+4, 4, 0xFFFF);

            for (int i = 0; i < pipes.size(); i++) {
                int px = (int)pipes[i].x;
                int oldPx = (int)pipes[i].oldX;
                int py = pipes[i].gapY;
                
                int clearW = (int)pipeSpeed + 2;
                tft.fillRect(oldPx + pipeW, 0, clearW, py, bgColor);
                tft.fillRect(oldPx + pipeW, py + gapH, clearW, groundY - (py + gapH), bgColor);

                tft.fillRect(px, 0, pipeW, py, 0x07E0);
                tft.drawRect(px, 0, pipeW, py, 0x0000);
                tft.fillRect(px-2, py-4, pipeW+4, 4, 0x07E0); 
                tft.drawRect(px-2, py-4, pipeW+4, 4, 0x0000);

                tft.fillRect(px, py + gapH, pipeW, groundY - (py + gapH), 0x07E0);
                tft.drawRect(px, py + gapH, pipeW, groundY - (py + gapH), 0x0000);
                tft.fillRect(px-2, py + gapH, pipeW+4, 4, 0x07E0); 
                tft.drawRect(px-2, py + gapH, pipeW+4, 4, 0x0000);

                if (birdX + birdSize > px && birdX < px + pipeW) {
                    if (birdY < py || birdY + birdSize > py + gapH) gameOver = true;
                }
                
                if (px < birdX && !pipes[i].passed) {
                    score++;
                    pipes[i].passed = true;
                    playToneGame(2000, 50);
                    tft.fillRect(70, 5, 30, 15, bgColor); 
                    tft.setTextColor(0xFFFF); 
                    tft.setCursor(75, 15); 
                    tft.print(score);
                }
            }
            tft.drawFastHLine(0, groundY, 160, 0x07E0);
        }
    }
    
    // Game Over & Reward
    playToneGame(100, 500);
    delay(500);
    tft.fillScreen(0x0000);
    tft.setTextColor(0xF800); tft.setTextSize(2); 
    tft.setCursor(30, 40); tft.print("GAME OVER");
    tft.setTextColor(0xFFFF); tft.setTextSize(1);
    tft.setCursor(50, 70); tft.print("Score: " + String(score));
    
    // [MOD] 1 Điểm = 10 tiền
    if (score > 0) {
        int bonus = score * 10;
        updateChips(bonus);
        tft.setCursor(30, 90); tft.setTextColor(0x07E0); tft.print("Thuong: +" + String(bonus) + " Chips");
        playToneGame(2000, 200);
    } else {
        tft.setCursor(30, 90); tft.setTextColor(0xAAAA); tft.print("Co gang len!");
    }
    
    tft.setCursor(35, 110); tft.setTextColor(0x07E0); tft.print("BAM DE THOAT");
    waitForRelease();
    while(!isPressed(TOUCH_MENU) && !isPressed(TOUCH_UP));
}

// ======================================================
// GAME 2: BLACKJACK (Cược tùy chọn)
// ======================================================
void drawCard(int x, int y, int value, bool hidden = false) {
    tft.fillRoundRect(x, y, 20, 28, 3, C_CARD_BG);
    if (hidden) {
        tft.fillRoundRect(x+2, y+2, 16, 24, 2, 0xF800);
        for(int i=0; i<16; i+=4) tft.drawFastHLine(x+2, y+2+i, 16, 0xFFFF);
        return;
    }
    uint16_t color = (random(0,2) == 0) ? C_CARD_RED : C_CARD_BLACK;
    String valStr = String(value);
    if (value == 1) valStr = "A"; else if (value == 11) valStr = "J";
    else if (value == 12) valStr = "Q"; else if (value == 13) valStr = "K";
    tft.setTextColor(color); tft.setTextSize(1);
    tft.setCursor(x + 3, y + 4); tft.print(valStr);
    if (color == C_CARD_RED) tft.fillCircle(x+14, y+20, 2, color);
    else tft.fillTriangle(x+12, y+22, x+16, y+22, x+14, y+18, color);
}

void runBlackjack() {
    int bet = 50; // Giá trị khởi điểm
    while(true) {
        int myChips = getChips();
        tft.fillScreen(C_TABLE_FELT);
        tft.drawRect(5, 5, 150, 118, 0xFFFF);
        
        tft.setTextColor(0xFFFF); tft.setTextSize(1);
        tft.setCursor(10, 10); tft.print("CHIP: " + String(myChips));
        tft.setCursor(30, 60); tft.print("UP: DAT CUOC");
        tft.setCursor(30, 80); tft.print("MENU: THOAT");
        
        // Logic chờ bắt đầu hoặc chỉnh cược
        unsigned long tStart = millis();
        while(true) {
            if (isPressed(TOUCH_MENU)) return;
            if (isPressed(TOUCH_UP)) {
                // Vào màn hình chọn cược
                bet = selectBetAmount(); 
                break;
            }
            if (millis() - tStart > 10000) return;
        }
        
        // Kiểm tra tiền sau khi chọn cược
        if (myChips < bet) {
             tft.fillScreen(0);
             tft.setTextColor(0xF800); tft.setCursor(40, 60); tft.print("KHONG DU TIEN!");
             delay(1000); 
             continue; // Quay lại màn hình chính của game
        }
        
        // Trừ tiền cược & Bắt đầu
        updateChips(-bet);

        tft.fillScreen(C_TABLE_FELT);
        tft.setCursor(5, 5); tft.print("DEALER"); tft.setCursor(5, 70); tft.print("PLAYER (" + String(bet) + "$)");
        
        int pCard1 = random(1, 11); int pCard2 = random(1, 11);
        int dCard1 = random(1, 11); int dCard2 = random(1, 11);
        int pTotal = pCard1 + pCard2; int dTotal = dCard1 + dCard2;
        
        drawCard(20, 20, dCard1); delay(200); drawCard(45, 20, 0, true); delay(200);
        drawCard(20, 85, pCard1); delay(200); drawCard(45, 85, pCard2); delay(200);
        tft.setCursor(80, 95); tft.print("=" + String(pTotal));

        bool playerBust = false; bool playerStand = false; int cardIdx = 2;
        while (!playerBust && !playerStand) {
            tft.fillRect(0, 50, 160, 20, C_TABLE_FELT);
            tft.setTextColor(0x07FF); tft.setCursor(20, 55); tft.print("UP: RUT | DN: DUNG");
            unsigned long actT = millis();
            while(true) {
                if (isPressed(TOUCH_UP)) { // HIT
                    int newC = random(1, 11); pTotal += newC;
                    drawCard(20 + (cardIdx * 25), 85, newC); playToneGame(1500, 100); cardIdx++;
                    tft.fillRect(80, 95, 50, 10, C_TABLE_FELT); 
                    tft.setTextColor(0xFFFF); tft.setCursor(80, 95); tft.print("=" + String(pTotal));
                    if (pTotal > 21) playerBust = true; delay(300); break;
                }
                if (isPressed(TOUCH_DOWN) || pTotal == 21) { playerStand = true; break; }
                if (millis() - actT > 10000) return;
            }
        }
        
        tft.fillRect(0, 50, 160, 20, C_TABLE_FELT);
        if (playerBust) {
            tft.setTextColor(0xF800); tft.setCursor(50, 55); tft.print("QUAC!"); playToneGame(200, 500);
        } else {
            drawCard(45, 20, dCard2); tft.setCursor(80, 30); tft.print("=" + String(dTotal)); delay(500);
            int dCardIdx = 2;
            while (dTotal < 17) {
                int newC = random(1, 11); dTotal += newC;
                drawCard(20 + (dCardIdx * 25), 20, newC);
                tft.fillRect(80, 30, 50, 10, C_TABLE_FELT); tft.setCursor(80, 30); tft.print("=" + String(dTotal));
                delay(600); dCardIdx++;
            }
            String res = ""; int win = 0; uint16_t c = 0xFFFF;
            if (dTotal > 21) { res = "NHA CAI LO"; win=bet*2; c=0x07FF; }
            else if (pTotal > dTotal) { res = "BAN THANG!"; win=bet*2; c=0x07FF; }
            else if (pTotal < dTotal) { res = "THUA ROI!"; win=0; c=0xF800; }
            else { res = "HOA CA!"; win=bet; c=0xFFE0; }
            tft.setTextColor(c); tft.setCursor(10, 55); tft.print(res);
            updateChips(win);
            if(win > bet) playToneGame(2000, 300); else playToneGame(200, 300);
        }
        delay(2000);
    }
}

// ======================================================
// GAME 3: SLOT MACHINE (Cược tùy chọn)
// ======================================================
const char* SLOTS[] = {"7", "BAR", "CAM", "TAO", "CHUONG"};
const uint16_t SLOT_COLORS[] = {0xF800, 0x07E0, 0xFD20, 0x07FF, 0xFFE0};

void drawSlotBox(int x, int y, int index, bool blur) {
    tft.drawRect(x, y, 40, 50, 0xFFFF);
    tft.fillRect(x+1, y+1, 38, 48, 0x0000);
    if (blur) {
        for(int i=5; i<45; i+=5) tft.drawFastHLine(x+5, y+i, 30, SLOT_COLORS[random(0,5)]);
    } else {
        tft.setTextColor(SLOT_COLORS[index]); tft.setTextSize(1); 
        tft.setCursor(x + 5, y + 20); tft.print(SLOTS[index]);
    }
}

void runSlotMachine() {
    int bet = 20; // Default bet
    while(true) {
        int myChips = getChips();
        tft.fillScreen(0x0000);
        tft.drawRect(10, 10, 140, 80, 0xFA60); 
        tft.setTextColor(0xFA60); tft.setCursor(45, 15); tft.print("JACKPOT");
        drawSlotBox(15, 30, 0, false); drawSlotBox(60, 30, 0, false); drawSlotBox(105, 30, 0, false);
        tft.setTextColor(0xFFFF); tft.setCursor(20, 100); tft.print("CHIP: " + String(myChips));
        tft.setCursor(20, 115); tft.print("UP: QUAY (" + String(bet) + "$)");
        
        while(true) {
            if (isPressed(TOUCH_MENU)) return;
            
            // Nhấn giữ UP để chọn mức cược mới (Logic mới)
            // Hoặc nhấn nhanh để quay luôn với mức cược cũ
            if (isPressed(TOUCH_UP)) {
                // Đợi xem có phải giữ lâu không để vào menu cược
                unsigned long holdStart = millis();
                while(isPressed(TOUCH_UP)) {
                    if(millis() - holdStart > 800) { // Giữ 0.8s -> Vào chỉnh cược
                        bet = selectBetAmount();
                        // Vẽ lại màn hình sau khi chỉnh
                        tft.fillScreen(0x0000);
                        tft.drawRect(10, 10, 140, 80, 0xFA60); 
                        tft.setTextColor(0xFA60); tft.setCursor(45, 15); tft.print("JACKPOT");
                        drawSlotBox(15, 30, 0, false); drawSlotBox(60, 30, 0, false); drawSlotBox(105, 30, 0, false);
                        tft.setTextColor(0xFFFF); tft.setCursor(20, 100); tft.print("CHIP: " + String(myChips));
                        tft.setCursor(20, 115); tft.print("UP: QUAY (" + String(bet) + "$)");
                        break; 
                    }
                }
                
                if (myChips < bet) { 
                    tft.setCursor(20, 90); tft.setTextColor(0xF800); tft.print("KHONG DU TIEN!"); 
                    delay(1000); 
                    break; // Quay lại vòng lặp vẽ
                }
                
                updateChips(-bet); 
                
                // --- LOGIC QUAY ---
                int res[3];
                for(int turn=0; turn<20; turn++) {
                    playToneGame(200 + turn*20, 20);
                    drawSlotBox(15, 30, 0, true); drawSlotBox(60, 30, 0, true); drawSlotBox(105, 30, 0, true);
                    if (turn > 10) { res[0] = random(0, 5); drawSlotBox(15, 30, res[0], false); }
                    if (turn > 15) { res[1] = random(0, 5); drawSlotBox(60, 30, res[1], false); }
                    if (turn == 19) { res[2] = random(0, 5); drawSlotBox(105, 30, res[2], false); }
                    delay(50 + turn * 10);
                }
                
                int win = 0;
                // Thưởng theo bội số cược
                if (res[0] == res[1] && res[1] == res[2]) {
                    win = (res[0] == 0) ? bet * 25 : bet * 10; // 777 = x25, khác = x10
                    for(int i=0; i<3; i++) { tft.invertDisplay(true); delay(100); tft.invertDisplay(false); delay(100); }
                    tft.setCursor(40, 90); tft.setTextColor(0x07E0); tft.print("NO HU !! +" + String(win));
                } else if (res[0] == res[1] || res[1] == res[2] || res[0] == res[2]) {
                    win = (int)(bet * 1.5); // Trúng đôi = x1.5
                    tft.setCursor(40, 90); tft.setTextColor(0xFFFF); tft.print("TRUNG NHO +" + String(win));
                } else {
                    tft.setCursor(50, 90); tft.setTextColor(0xF800); tft.print("THUA ROI");
                }
                updateChips(win); 
                delay(2000);
                break; // Break ra vòng lặp while nhỏ để vẽ lại màn hình chính
            }
        }
    }
}

// ======================================================
// GAME 4: BẦU CUA TÔM CÁ (Cược tùy chọn)
// ======================================================
const char* BAUCUA_NAMES[] = {"NAI", "BAU", "GA", "CA", "CUA", "TOM"};
const uint16_t BAUCUA_COLORS[] = {0xCA40, 0x07E0, 0xF800, 0x07FF, 0xFD20, 0xF81F}; 

void runBauCua() {
    int selection = 0; int myChips = getChips(); int bet = 50;
    while(true) {
        tft.fillScreen(0x0000);
        tft.setTextColor(0xFFFF); tft.setCursor(30, 5); tft.print("BAU CUA (" + String(bet) + "$)");
        
        for(int i=0; i<6; i++) {
            int x = (i % 3) * 53 + 2; int y = (i / 3) * 40 + 20;
            if (i == selection) tft.drawRect(x, y, 50, 35, 0xFFFF); else tft.drawRect(x, y, 50, 35, 0x4208);
            tft.fillRect(x+2, y+2, 46, 31, BAUCUA_COLORS[i]);
            tft.setTextColor(0x0000); tft.setCursor(x+10, y+12); tft.print(BAUCUA_NAMES[i]);
        }
        
        // Vẽ nút THOÁT / CHỈNH CƯỢC
        if (selection == 6) {
            tft.fillRoundRect(40, 105, 80, 20, 4, 0xF800); tft.setTextColor(0xFFFF);
        } else {
            tft.drawRoundRect(40, 105, 80, 20, 4, 0xFFFF); tft.setTextColor(0xAAAA);
        }
        tft.setCursor(55, 110); tft.print("CUOC/THOAT");

        while(true) {
            if (isPressed(TOUCH_UP)) { selection = (selection + 1) % 7; break; }
            if (isPressed(TOUCH_DOWN)) { selection = (selection - 1 + 7) % 7; break; }
            
            if (isPressed(TOUCH_MENU)) {
                if (selection == 6) {
                    // Nếu chọn nút dưới cùng -> Vào menu chỉnh cược hoặc thoát
                    tft.fillScreen(0);
                    tft.setCursor(20, 40); tft.print("UP: DOI CUOC");
                    tft.setCursor(20, 60); tft.print("DN: THOAT GAME");
                    waitForRelease();
                    while(true) {
                        if (isPressed(TOUCH_UP)) { bet = selectBetAmount(); break; }
                        if (isPressed(TOUCH_DOWN)) return; // Thoát game
                    }
                    break; // Vẽ lại bàn cờ
                } 

                if (myChips < bet) { tft.fillScreen(0); tft.setCursor(40, 60); tft.print("HET TIEN!"); delay(1000); break; }
                updateChips(-bet);
                tft.fillRect(0, 100, 160, 28, 0x0000); tft.setCursor(40, 110); tft.setTextColor(0x07FF); tft.print("DANG LAC...");
                int res[3];
                for (int i=0; i<10; i++) { playToneGame(500 + random(0, 500), 30); delay(50); }
                tft.fillScreen(0x0000); tft.setCursor(40, 10); tft.setTextColor(0xFFFF); tft.print("KET QUA");
                int hits = 0;
                for(int i=0; i<3; i++) {
                    res[i] = random(0, 6); int x = 10 + i * 50;
                    tft.fillRect(x, 30, 40, 40, BAUCUA_COLORS[res[i]]);
                    tft.setTextColor(0x0000); tft.setCursor(x+5, 45); tft.print(BAUCUA_NAMES[res[i]]);
                    if (res[i] == selection) hits++;
                }
                int win = 0;
                if (hits > 0) {
                    win = bet + (bet * hits); tft.setCursor(30, 90); tft.setTextColor(0x07E0); tft.print("TRUNG " + String(hits) + " NHAY!");
                    tft.setCursor(40, 105); tft.print("+" + String(win) + " Chip"); playToneGame(2000, 300);
                } else {
                    tft.setCursor(50, 90); tft.setTextColor(0xF800); tft.print("TRAT ROI!"); playToneGame(200, 500);
                }
                updateChips(win); myChips = getChips();
                tft.setTextColor(0xFFFF); tft.setCursor(20, 120); tft.print("Cham de tiep tuc...");
                delay(1000); 
                while(!isPressed(TOUCH_MENU) && !isPressed(TOUCH_UP)); 
                break;
            }
            delay(100);
        }
        delay(150);
    }
}

// ======================================================
// GAME 5: PONG (PLAY TO EARN)
// ======================================================
void runPong() {
    float pY = 54, aiY = 54; 
    float bX = 80, bY = 64;  
    float vX = 2, vY = 1;    
    int pH = 24;             
    int scoreP = 0, scoreAI = 0;

    tft.fillScreen(0x0000);
    for(int i=0; i<128; i+=8) tft.drawFastVLine(80, i, 4, 0x4208);

    while(scoreP < 5 && scoreAI < 5) {
        if (isPressed(TOUCH_MENU)) { tft.fillScreen(0); return; }

        if (isPressed(TOUCH_UP)) pY -= 4;
        if (isPressed(TOUCH_DOWN)) pY += 4;
        if (pY < 0) pY = 0; if (pY > 128-pH) pY = 128-pH;

        if (bX > 80) { 
            if (bY > aiY + pH/2) aiY += 2.5; 
            else if (bY < aiY + pH/2) aiY -= 2.5;
        }
        if (aiY < 0) aiY = 0; if (aiY > 128-pH) aiY = 128-pH;

        tft.fillRect((int)bX, (int)bY, 4, 4, 0x0000);
        bX += vX; bY += vY;

        if (bY <= 0 || bY >= 124) { vY = -vY; playToneGame(100, 10); }

        if (bX <= 8 && bX >= 4 && bY + 4 >= pY && bY <= pY + pH) {
            vX = -vX * 1.1; if (vX > 5) vX = 5; vX = abs(vX); playToneGame(500, 20);
        }
        if (bX >= 148 && bX <= 152 && bY + 4 >= aiY && bY <= aiY + pH) {
            vX = -vX * 1.1; if (vX < -5) vX = -5; vX = -abs(vX); playToneGame(500, 20);
        }

        bool goal = false;
        if (bX < 0) { scoreAI++; goal = true; }
        else if (bX > 160) { scoreP++; goal = true; }

        if (goal) {
            playToneGame(1000, 200);
            tft.setTextColor(0xFFFF); tft.setCursor(50, 50); 
            if (bX > 160) tft.print("GOAL!!!"); else tft.print("MISS...");
            delay(1000);
            bX = 80; bY = 64; vX = (random(0,2)==0 ? 2 : -2); vY = 1;
            tft.fillScreen(0x0000);
            for(int i=0; i<128; i+=8) tft.drawFastVLine(80, i, 4, 0x4208);
        }

        tft.fillRect(4, 0, 4, 128, 0x0000); tft.fillRect(4, (int)pY, 4, pH, 0x07FF); 
        tft.fillRect(152, 0, 4, 128, 0x0000); tft.fillRect(152, (int)aiY, 4, pH, 0xF800); 
        tft.fillRect((int)bX, (int)bY, 4, 4, 0xFFFF);
        tft.setTextColor(0x07FF); tft.setCursor(60, 5); tft.print(scoreP);
        tft.setTextColor(0xF800); tft.setCursor(95, 5); tft.print(scoreAI);
        delay(15);
    }
    
    tft.fillScreen(0x0000); tft.setCursor(30, 50); tft.setTextColor(0xFFFF);
    
    // [MOD] THẮNG AI ĐƯỢC THƯỞNG 200 TIỀN
    if (scoreP >= 5) { 
        tft.print("YOU WIN!"); 
        updateChips(200); 
        tft.setCursor(30, 70); tft.setTextColor(0x07E0); tft.print("+200 Chips"); 
        playToneGame(2000, 500);
    } else { 
        tft.print("YOU LOSE"); playToneGame(200, 500);
    }
    delay(2000);
}

// ======================================================
// GAME 6: DINO RUN (PLAY TO EARN)
// ======================================================
void runDino() {
    float dy = 100; float dvy = 0; bool jumping = false;
    int score = 0; int speed = 3;
    struct Cactus { int x; int h; bool passed; };
    std::vector<Cactus> cacti;
    cacti.push_back({200, 15, false}); 

    tft.fillScreen(0xFFFF); 
    tft.setTextColor(0x0000); tft.drawFastHLine(0, 110, 160, 0x0000); 

    while(true) {
        if (isPressed(TOUCH_MENU)) { tft.fillScreen(0); return; }

        if ((isPressed(TOUCH_UP) || isPressed(TOUCH_DOWN)) && !jumping) {
             dvy = -5.5; jumping = true; playToneGame(600, 30);
        }

        float oldY = dy; dy += dvy; dvy += 0.4; 
        if (dy >= 100) { dy = 100; jumping = false; dvy = 0; }

        tft.fillRect(10, (int)oldY, 10, 10, 0xFFFF); tft.fillRect(10, (int)dy, 10, 10, 0x0000); 

        for (int i=0; i<cacti.size(); i++) {
            tft.fillRect(cacti[i].x + speed, 110 - cacti[i].h, 6, cacti[i].h, 0xFFFF);
            cacti[i].x -= speed;
            tft.fillRect(cacti[i].x, 110 - cacti[i].h, 6, cacti[i].h, 0x07E0); 

            if (cacti[i].x < 20 && cacti[i].x + 6 > 10) {
                if (dy + 10 > 110 - cacti[i].h) {
                    playToneGame(100, 500);
                    tft.setTextColor(0xF800); tft.setCursor(50, 50); tft.print("GAME OVER");
                    
                    // [MOD] 1 Điểm = 10 Tiền
                    if (score > 0) {
                        int bonus = score * 10;
                        updateChips(bonus);
                        tft.setTextColor(0x07E0); tft.setCursor(30, 70); tft.print("+" + String(bonus) + " Chips");
                    }
                    
                    delay(2000); tft.fillScreen(0); return;
                }
            }
            if (cacti[i].x < 10 && !cacti[i].passed) {
                score++; cacti[i].passed = true;
                if (score % 5 == 0) speed++; 
                tft.fillRect(100, 0, 60, 20, 0xFFFF);
                tft.setTextColor(0x0000); tft.setCursor(110, 5); tft.print("HI: " + String(score));
            }
        }
        if (cacti.front().x < -10) cacti.erase(cacti.begin());
        if (cacti.back().x < 160 - random(60, 120)) { 
             cacti.push_back({160 + random(0, 50), random(12, 20), false}); 
        }
        delay(20);
    }
}

// [MỚI] INCLUDE MODULE GAME ĐỐ VUI (Sau khi đã định nghĩa các hàm hỗ trợ)
#include "brain_quiz_app.h"

// ======================================================
// MENU TRUNG TÂM
// ======================================================
void runCreativeMenu() {
    int cursor = 0;
    waitForRelease();

    while (true) {
        tft.fillScreen(0x0000);
        
        tft.fillRect(0, 0, 160, 20, 0x07FF);
        tft.setTextColor(0x0000); tft.setTextSize(1);
        tft.setCursor(35, 6); tft.print("KHO TRO CHOI");
        
        // CẬP NHẬT MENU: Thêm DO VUI HAI NAO
        String items[] = {"FLAPPY BIRD", "BLACKJACK", "SLOT MACHINE", "BAU CUA", "PONG (AI)", "DINO RUN", "VUA PHAN XA", "DO VUI (VIP)", "SHOP AI", "THOAT"};
        int itemCount = 10; 
        
        int itemsPerPage = 5;
        int scrollOffset = 0;
        if (cursor >= itemsPerPage) scrollOffset = cursor - itemsPerPage + 1;

        for (int i=0; i<itemsPerPage; i++) {
            int idx = scrollOffset + i;
            if (idx >= itemCount) break;

            int y = 30 + i * 18;
            if (idx == cursor) {
                tft.fillRoundRect(5, y, 150, 16, 2, 0x07E0); 
                tft.setTextColor(0x0000); tft.setCursor(15, y + 4); tft.print("> " + items[idx]);
            } else {
                tft.drawRoundRect(5, y, 150, 16, 2, 0x4208);
                tft.setTextColor(0xFFFF); tft.setCursor(15, y + 4); tft.print(items[idx]);
            }
        }
        
        tft.drawFastHLine(0, 120, 160, 0xFFFF);
        tft.setTextColor(0xFD20); tft.setCursor(5, 122); tft.print("$: " + String(getChips()));

        while (true) {
            if (isPressed(TOUCH_MENU)) {
                waitForRelease(); 
                if (cursor == 0) runFlappyBird();
                else if (cursor == 1) runBlackjack();
                else if (cursor == 2) runSlotMachine();
                else if (cursor == 3) runBauCua();
                else if (cursor == 4) runPong();
                else if (cursor == 5) runDino();
                else if (cursor == 6) runReflexGame(); 
                else if (cursor == 7) runBrainQuizBig(); // [MỚI] Gọi hàm từ brain_quiz_app.h
                else if (cursor == 8) runShopMenu(); 
                else if (cursor == 9) return;
                break; 
            }
            if (isPressed(TOUCH_UP)) { cursor = (cursor - 1 + itemCount) % itemCount; break; }
            if (isPressed(TOUCH_DOWN)) { cursor = (cursor + 1) % itemCount; break; }
            delay(50);
        }
        delay(100);
    }
}

#endif