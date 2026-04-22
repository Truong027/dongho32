#ifndef GAME_APP_H
#define GAME_APP_H

#include "config.h"
#include "app_functions.h"
#include <vector>
#include <Preferences.h>

// --- [NÂNG CẤP] QUẢN LÝ LƯU TRỮ (HIGH SCORE) ---
Preferences gamePrefs;

void initGameStorage() {
    gamePrefs.begin("games", false);
}

int getHighScore(const char* gameName) {
    initGameStorage();
    int score = gamePrefs.getInt(gameName, 0);
    gamePrefs.end();
    return score;
}

void saveHighScore(const char* gameName, int score) {
    initGameStorage();
    int currentHigh = gamePrefs.getInt(gameName, 0);
    // Với game phản xạ (reflex), số nhỏ hơn là tốt hơn nhưng ở đây logic chung là lưu số lớn
    if (score > currentHigh) {
        gamePrefs.putInt(gameName, score);
    }
    gamePrefs.end();
}

// --- [NÂNG CẤP] HỆ THỐNG HIỆU ỨNG HẠT (PARTICLE SYSTEM) ---
struct Particle {
    float x, y;
    float vx, vy;
    uint16_t color;
    int life;
};

std::vector<Particle> particles;

void spawnExplosion(int x, int y, uint16_t color, int count = 10) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.x = x; p.y = y;
        p.vx = random(-20, 20) / 10.0;
        p.vy = random(-20, 20) / 10.0;
        p.color = color;
        p.life = random(10, 25);
        particles.push_back(p);
    }
}

void updateAndDrawParticles() {
    for (int i = particles.size() - 1; i >= 0; i--) {
        // Xóa vị trí cũ
        tft.drawPixel((int)particles[i].x, (int)particles[i].y, 0x0000); 
        
        // Cập nhật vị trí vật lý
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].life--;

        // Vẽ vị trí mới nếu còn sống
        if (particles[i].life > 0) {
            tft.drawPixel((int)particles[i].x, (int)particles[i].y, particles[i].color); 
        } else {
            particles.erase(particles.begin() + i);
        }
    }
}

// --- [NÂNG CẤP] HỆ THỐNG ÂM THANH ARCADE ---
void playSound(int type) {
    if (type == 0) { // Jump / Tap
        ledcWriteTone(BUZZER_PIN, 600); delay(20); ledcWriteTone(BUZZER_PIN, 0);
    } else if (type == 1) { // Score / Ding
        ledcWriteTone(BUZZER_PIN, 1500); delay(50); ledcWriteTone(BUZZER_PIN, 2500); delay(50); ledcWriteTone(BUZZER_PIN, 0);
    } else if (type == 2) { // Crash / Fail
        for (int i=0; i<5; i++) {
            ledcWriteTone(BUZZER_PIN, random(100, 500)); delay(30);
        }
        ledcWriteTone(BUZZER_PIN, 0);
    } else if (type == 3) { // Win / Jackpot (Nhạc chiến thắng)
        int notes[] = {523, 659, 784, 1046, 784, 659};
        for (int i=0; i<6; i++) {
            ledcWriteTone(BUZZER_PIN, notes[i]); delay(80);
        }
        ledcWriteTone(BUZZER_PIN, 0);
    }
}

// --- CẤU HÌNH GAME FLAPPY (REALISTIC PHYSICS) ---
#define BIRD_X        20
#define BIRD_SIZE     8
#define GRAVITY       1.2   
#define JUMP_STRENGTH -4.5  
#define PIPE_SPEED    3     
#define PIPE_WIDTH    14    
#define PIPE_GAP      36    

float birdY_float;
float birdVelocity;
int pipeX, pipeGapY;
int score;
bool isGameOver;
unsigned long lastFrameTime = 0;
bool isButtonPressed = false; 

void resetGame() {
    birdY_float = 64.0;
    birdVelocity = 0;
    pipeX = 160;
    pipeGapY = random(20, 80); 
    score = 0;
    isGameOver = false;
    isButtonPressed = false;
    particles.clear();
    tft.fillScreen(0x0000);

    // Intro Sound
    playSound(1); delay(100); playSound(1);
}

void drawGame() {
    // 1. Xóa chim cũ
    tft.fillRect(BIRD_X, (int)birdY_float - 5, BIRD_SIZE, BIRD_SIZE + 10, 0x0000); 

    // 2. Cập nhật vật lý
    birdVelocity += GRAVITY;
    birdY_float += birdVelocity;

    // Giới hạn trần/sàn
    if (birdY_float < 0) { birdY_float = 0; birdVelocity = 0; }
    if (birdY_float > 120) isGameOver = true;

    // 3. Vẽ Chim (Có mắt mỏ chi tiết hơn chút)
    tft.fillRect(BIRD_X, (int)birdY_float, BIRD_SIZE, BIRD_SIZE, 0xFFE0); 
    tft.fillRect(BIRD_X + 5, (int)birdY_float + 2, 3, 4, 0xF800); // Mỏ
    tft.drawPixel(BIRD_X + 5, (int)birdY_float + 1, 0x0000); // Mắt

    // 4. Xử lý Cống
    // Xóa cống (tối ưu hóa chỉ xóa phần rìa di chuyển)
    tft.fillRect(pipeX + PIPE_WIDTH, 0, PIPE_SPEED, 128, 0x0000); 
    pipeX -= PIPE_SPEED;
    
    if (pipeX < -PIPE_WIDTH) {
        pipeX = 160;
        pipeGapY = random(20, 80); 
        score++;
        playSound(1); // Ting!
    }

    // Vẽ Cống (Có viền 3D nhẹ)
    tft.fillRect(pipeX, 0, PIPE_WIDTH, pipeGapY, 0x07E0); 
    tft.drawRect(pipeX, 0, PIPE_WIDTH, pipeGapY, 0x05E0);
    tft.fillRect(pipeX - 1, pipeGapY - 4, PIPE_WIDTH + 2, 4, 0x07E0); // Nắp trên

    tft.fillRect(pipeX, pipeGapY + PIPE_GAP, PIPE_WIDTH, 128 - (pipeGapY + PIPE_GAP), 0x07E0); 
    tft.drawRect(pipeX, pipeGapY + PIPE_GAP, PIPE_WIDTH, 128 - (pipeGapY + PIPE_GAP), 0x05E0);
    tft.fillRect(pipeX - 1, pipeGapY + PIPE_GAP, PIPE_WIDTH + 2, 4, 0x07E0); // Nắp dưới

    // 5. Va chạm
    if (BIRD_X + BIRD_SIZE > pipeX && BIRD_X < pipeX + PIPE_WIDTH) {
        if (birdY_float < pipeGapY || birdY_float + BIRD_SIZE > pipeGapY + PIPE_GAP) {
            isGameOver = true;
            spawnExplosion(BIRD_X + BIRD_SIZE/2, (int)birdY_float, 0xF800, 20); // Nổ xác chim!
        }
    }

    // 6. UI & Particles
    tft.setCursor(75, 10);
    tft.setTextColor(0xFFFF, 0x0000); 
    tft.setTextSize(1);
    tft.print(score);
    
    updateAndDrawParticles();
}

void runFlappyGame() {
    resetGame();
    int highScore = getHighScore("flappy");
    tft.setFont(NULL);
    
    while (true) {
        if (touchRead(TOUCH_MENU) < TOUCH_THRESHOLD) {
             tft.fillScreen(0); return; 
        }

        if (isGameOver) {
            saveHighScore("flappy", score);
            
            // Vẽ màn hình Game Over xịn hơn
            tft.fillScreen(0x0000);
            tft.setCursor(40, 40); tft.setTextSize(2); tft.setTextColor(0xF800);
            tft.print("GAME OVER");
            
            tft.setTextSize(1); tft.setTextColor(0xFFFF);
            tft.setCursor(50, 65); tft.print("Score: "); tft.print(score);
            
            tft.setTextColor(0xFFE0);
            tft.setCursor(50, 80); tft.print("Best:  "); tft.print(max(score, highScore));
            
            tft.setTextColor(0x07E0);
            tft.setCursor(35, 100); tft.print("UP: Choi Lai");
            
            playSound(2); // Âm thanh thua cuộc
            
            while(true) {
                if (isTouchedSafe(TOUCH_UP)) { resetGame(); break; }
                if (isTouchedSafe(TOUCH_MENU)) { tft.fillScreen(0); return; }
                delay(50);
            }
        } else {
            bool currentTouch = (touchRead(TOUCH_UP) < TOUCH_THRESHOLD);
            if (currentTouch) {
                if (!isButtonPressed) {
                    birdVelocity = JUMP_STRENGTH;
                    playSound(0); 
                    spawnExplosion(BIRD_X, (int)birdY_float + BIRD_SIZE, 0xFFFF, 3); // Bụi bay khi vỗ cánh
                    isButtonPressed = true;
                }
            } else {
                isButtonPressed = false;
            }
            
            if (millis() - lastFrameTime > 30) {
                drawGame();
                lastFrameTime = millis();
            }
        }
    }
}

// --- REFLEX GAME (VUA PHẢN XẠ - PRO) ---
enum ReflexState { REFLEX_IDLE, REFLEX_WAIT, REFLEX_READY, REFLEX_RESULT };
ReflexState rState = REFLEX_IDLE;
unsigned long waitStartTime = 0;
unsigned long randomWaitTime = 0;
unsigned long reflexStartTime = 0;
unsigned long reactionTime = 0;

void drawReflexUI(String msg, uint16_t color, uint16_t bgColor) {
    tft.fillScreen(bgColor);
    tft.setTextColor(color);
    tft.setTextSize(2);
    int x = (160 - (msg.length() * 12)) / 2;
    if (x < 0) x = 0;
    tft.setCursor(x, 55);
    tft.print(msg);
}

void runReflexGame() {
    initGameStorage();
    int bestTime = gamePrefs.getInt("reflex_best", 9999);
    gamePrefs.end();

    rState = REFLEX_IDLE;
    drawReflexUI("SAN SANG?", 0xFFFF, 0x0000); 
    tft.setTextSize(1); tft.setCursor(35, 80); tft.print("Bam UP de choi");
    if (bestTime < 9999) {
        tft.setCursor(35, 95); tft.print("Best: " + String(bestTime) + "ms");
    }

    while(true) {
        if (isTouchedSafe(TOUCH_MENU)) { tft.fillScreen(0); return; }

        switch (rState) {
            case REFLEX_IDLE:
                if (isTouchedSafe(TOUCH_UP) || isTouchedSafe(TOUCH_DOWN)) {
                    rState = REFLEX_WAIT;
                    drawReflexUI("CHO...", 0xFFFF, 0xF800); 
                    waitStartTime = millis();
                    randomWaitTime = random(2000, 6000); 
                }
                break;
            case REFLEX_WAIT:
                if (touchRead(TOUCH_UP) < TOUCH_THRESHOLD || touchRead(TOUCH_DOWN) < TOUCH_THRESHOLD) {
                    rState = REFLEX_RESULT;
                    drawReflexUI("PHAM QUY!", 0xFFFF, 0x0000);
                    playSound(2); 
                    delay(1000);
                    tft.setTextSize(1); tft.setCursor(20, 90); tft.print("Bam nut de lai");
                }
                if (millis() - waitStartTime > randomWaitTime) {
                    rState = REFLEX_READY;
                    drawReflexUI("BAM NGAY!", 0x0000, 0x07E0); 
                    reflexStartTime = millis();
                    ledcWriteTone(BUZZER_PIN, 4000); // Beep liên tục
                }
                break;
            case REFLEX_READY:
                if (touchRead(TOUCH_UP) < TOUCH_THRESHOLD || touchRead(TOUCH_DOWN) < TOUCH_THRESHOLD) {
                    ledcWriteTone(BUZZER_PIN, 0); // Tắt beep
                    reactionTime = millis() - reflexStartTime;
                    rState = REFLEX_RESULT;
                    
                    // Lưu kỷ lục mới
                    if (reactionTime < bestTime) {
                        initGameStorage();
                        gamePrefs.putInt("reflex_best", reactionTime);
                        gamePrefs.end();
                        bestTime = reactionTime;
                    }

                    tft.fillScreen(0x0000);
                    tft.setTextColor(0xFFFF);
                    tft.setTextSize(3);
                    tft.setCursor(40, 40);
                    tft.print(String(reactionTime));
                    tft.setTextSize(1);
                    tft.setCursor(110, 60); tft.print("ms");
                    
                    tft.setCursor(40, 80);
                    tft.setTextColor(0x07E0);
                    if (reactionTime < 200) tft.print("THAN THANH!");
                    else if (reactionTime < 300) tft.print("XUAT SAC!");
                    else if (reactionTime < 400) { tft.setTextColor(0xFFE0); tft.print("KHA TOT"); }
                    else if (reactionTime < 600) { tft.setTextColor(0xFFFF); tft.print("TRUNG BINH"); }
                    else { tft.setTextColor(0xF800); tft.print("RUA BO!"); }
                    
                    delay(200);
                    if (reactionTime < 300) playSound(3); // Nhạc chiến thắng
                    else playSound(1);
                }
                break;
            case REFLEX_RESULT:
                if (isTouchedSafe(TOUCH_UP) || isTouchedSafe(TOUCH_DOWN)) {
                    rState = REFLEX_WAIT; 
                    drawReflexUI("CHO...", 0xFFFF, 0xF800);
                    waitStartTime = millis();
                    randomWaitTime = random(2000, 5000);
                    delay(500); 
                }
                break;
        }
    }
}

// --- FLASHLIGHT TOOL (GIỮ NGUYÊN) ---
int flashMode = 0; // 0:ON, 1:STROBE, 2:SOS
unsigned long lastFlashTime = 0;
bool flashState = true;
// Mẫu SOS: ... --- ... (Đơn vị thời gian: Dot = 200ms)
const int sosPattern[] = {1,0,1,0,1,0, 3,0,3,0,3,0, 1,0,1,0,1,0, 7}; 
int sosIndex = 0;

void drawFlashUI(String modeName) {
    tft.fillRect(0, 0, 160, 20, 0x0000);
    tft.setTextSize(1); tft.setTextColor(0xAAAA);
    tft.setCursor(5, 5); tft.print("MODE: " + modeName);
    tft.setCursor(100, 5); tft.print("UP: Doi");
}

void runFlashlightTool() {
    flashMode = 0; 
    flashState = true;
    lastFlashTime = millis();
    sosIndex = 0;
    
    tft.fillScreen(0xFFFF);
    ledcWrite(TFT_BL, 255); 
    drawFlashUI("TINH");

    while(true) {
        if (isTouchedSafe(TOUCH_MENU)) {
             applyBrightness(); // Khôi phục độ sáng cũ
             tft.fillScreen(0); 
             return; 
        }

        if (isTouchedSafe(TOUCH_UP)) {
            flashMode = (flashMode + 1) % 3;
            tft.fillScreen(0xFFFF); 
            if (flashMode == 0) drawFlashUI("TINH");
            else if (flashMode == 1) drawFlashUI("NHAY");
            else drawFlashUI("SOS");
            delay(200);
        }

        unsigned long now = millis();
        
        if (flashMode == 0) {
            // Tĩnh: Luôn sáng
            ledcWrite(TFT_BL, 255);
        }
        else if (flashMode == 1) {
            // Strobe: Nháy nhanh
            if (now - lastFlashTime > 100) {
                flashState = !flashState;
                ledcWrite(TFT_BL, flashState ? 255 : 0);
                lastFlashTime = now;
            }
        }
        else if (flashMode == 2) {
            // SOS Pattern
            int duration = sosPattern[sosIndex] * 200; // Đơn vị 200ms
            if (now - lastFlashTime > duration) {
                sosIndex++;
                if (sosIndex >= 19) sosIndex = 0; // Lặp lại
                
                bool isLightStep = (sosIndex % 2 == 0);
                if (sosIndex == 18) isLightStep = false;

                ledcWrite(TFT_BL, isLightStep ? 255 : 0);
                lastFlashTime = now;
            }
        }
    }
}

#endif