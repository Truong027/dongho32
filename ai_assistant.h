#ifndef AI_ASSISTANT_H
#define AI_ASSISTANT_H

// Include logic cốt lõi
#include "ai_core.h"

// --- RENDER FUNCTIONS ---
void renderAiAvatar(int x, int y, int size, int mouthGap, bool eOpen, int direction, uint16_t color) {
    
    // Hiệu ứng thời tiết: Run rẩy khi lạnh
    if (currentTemp > 0 && currentTemp < 20) {
        x += random(-1, 2); 
        y += random(-1, 2); 
    }

    // --- VẼ HÌNH DÁNG (BODY) DỰA THEO SKIN ---
    if (currentSkin == SKIN_PACMAN) {
        tft.fillCircle(x, y, size, color);
    } 
    else if (currentSkin == SKIN_GHOST) {
        tft.fillCircle(x, y - 2, size, color);
        tft.fillRect(x - size, y - 2, size * 2 + 1, size + 4, color);
        if ((millis()/200)%2==0) {
            tft.drawPixel(x - size, y + size + 2, 0x0000);
            tft.drawPixel(x, y + size + 2, 0x0000);
            tft.drawPixel(x + size, y + size + 2, 0x0000);
        }
    }
    else if (currentSkin == SKIN_ROBOT) {
        tft.fillRect(x - size, y - size, size * 2 + 1, size * 2 + 1, color);
        tft.drawFastVLine(x, y - size - 4, 4, color); // Ăng ten
        tft.drawPixel(x, y - size - 5, (millis()/500)%2==0 ? 0xF800 : 0x0000); // Đèn đỏ
    }
    else if (currentSkin == SKIN_POKEMON) { // [ĐẸP HƠN] Pikachu Style
        // Tai dài đặc trưng (Tam giác nhọn)
        tft.fillTriangle(x - size + 1, y - size, x - size - 4, y - size - 8, x - size + 3, y - size, color); // Tai trái
        tft.fillTriangle(x + size - 1, y - size, x + size + 4, y - size - 8, x + size - 3, y - size, color); // Tai phải
        
        // Chóp tai đen
        tft.drawLine(x - size - 2, y - size - 5, x - size - 4, y - size - 8, 0x0000);
        tft.drawLine(x + size + 2, y - size - 5, x + size + 4, y - size - 8, 0x0000);
        
        // Đầu tròn
        tft.fillCircle(x, y, size, color);
        
        // Má hồng
        tft.fillCircle(x - size + 1, y + 2, 2, 0xF800);
        tft.fillCircle(x + size - 1, y + 2, 2, 0xF800);
    }
    else if (currentSkin == SKIN_ANIME) { // [ĐẸP HƠN] Anime Style
        // Đầu tròn
        tft.fillCircle(x, y, size, color);
        // Tóc mái (đơn giản nhưng hiệu quả)
        tft.fillRect(x - size, y - size - 1, size * 2 + 1, 4, 0x8410); // Tóc màu xám/đen
        tft.drawPixel(x - 3, y - size + 3, 0x8410);
        tft.drawPixel(x + 3, y - size + 3, 0x8410);
    }

    int eyeX = x + (direction * (size/2));
    int eyeY = y - (size/2);
    tft.setTextColor(color); 

    // Hiệu ứng thời tiết: Đổ mồ hôi khi nóng
    if (currentTemp > 32) {
        if ((millis()/500)%2 == 0) {
            tft.drawPixel(x - direction*4, y - size - 2, 0x07FF);
            tft.drawPixel(x - direction*5, y - size - 1, 0x07FF);
        }
    }

    // --- VẼ MẮT VÀ BIỂU CẢM ---
    if (currentSkin == SKIN_ROBOT) {
        switch(currentMood) {
            case MOOD_HAPPY: case MOOD_HYPER: tft.fillRect(eyeX - 1, eyeY - 1, 3, 3, 0x0000); break;
            case MOOD_SLEEPY: tft.drawFastHLine(eyeX - 2, eyeY, 5, 0x0000); break;
            case MOOD_ANGRY: tft.drawLine(eyeX - 2, eyeY - 2, eyeX + 2, eyeY + 1, 0x0000); break;
            default: if (eOpen) tft.fillRect(eyeX, eyeY, 2, 2, 0x0000); else tft.drawFastHLine(eyeX - 1, eyeY, 3, 0x0000); break;
        }
    } 
    else if (currentSkin == SKIN_ANIME || currentSkin == SKIN_POKEMON) { 
        // Mắt To Long Lanh cho Anime/Pokemon
        int animeEyeY = y - 1;
        if (eOpen) {
            tft.fillRect(x - 3, animeEyeY - 2, 3, 4, 0x0000); // Mắt trái
            tft.fillRect(x + 1, animeEyeY - 2, 3, 4, 0x0000); // Mắt phải
            tft.drawPixel(x - 3, animeEyeY - 2, 0xFFFF); // Sparkle trái
            tft.drawPixel(x + 1, animeEyeY - 2, 0xFFFF); // Sparkle phải
        } else {
            // Nhắm mắt vui vẻ ^_^
            tft.drawPixel(x - 3, animeEyeY, 0x0000); tft.drawPixel(x - 2, animeEyeY - 1, 0x0000); tft.drawPixel(x - 1, animeEyeY, 0x0000);
            tft.drawPixel(x + 1, animeEyeY, 0x0000); tft.drawPixel(x + 2, animeEyeY - 1, 0x0000); tft.drawPixel(x + 3, animeEyeY, 0x0000);
        }
    }
    else { // Pacman, Ghost
        switch(currentMood) {
            case MOOD_SLEEPY: tft.drawLine(eyeX - 2, eyeY, eyeX + 2, eyeY, 0x0000); break;
            case MOOD_ANGRY: tft.drawLine(eyeX - 2, eyeY - 2, eyeX + 2, eyeY + 1, 0x0000); break;
            case MOOD_LOVELY: tft.drawPixel(eyeX, eyeY, 0xF800); break; 
            case MOOD_SAD: tft.drawLine(eyeX - 2, eyeY - 1, eyeX + 2, eyeY + 1, 0x0000); break;
            case MOOD_ALERT: tft.fillCircle(eyeX, eyeY, 2, 0xFFFF); tft.drawCircle(eyeX, eyeY, 2, 0xF800); break;
            case MOOD_HYPER: tft.fillCircle(eyeX, eyeY, 2, 0xFFFF); break;
            case MOOD_THINKING: if ((millis()/150)%2==0) tft.drawPixel(eyeX-1, eyeY, 0x0000); else tft.drawPixel(eyeX+1, eyeY, 0x0000); break;
            default: if (eOpen) tft.fillCircle(eyeX, eyeY, 1, 0x0000); else tft.drawLine(eyeX - 1, eyeY, eyeX + 1, eyeY, 0x0000); break;
        }
    }

    // --- VẼ MIỆNG ---
    uint16_t BG_COLOR = 0x1082; 
    
    if (currentSkin == SKIN_ROBOT) {
        if (mouthGap > 0) tft.fillRect(x - 3, y + 2, 7, mouthGap/2 + 1, 0x0000);
        else tft.drawFastHLine(x - 2, y + 4, 5, 0x0000);
    } 
    else if (currentSkin == SKIN_ANIME) {
        if (mouthGap > 0) tft.fillCircle(x, y + 3, 2, 0xF800); // Miệng hồng chúm chím
        else tft.drawPixel(x, y + 3, 0x0000); 
    }
    else if (currentSkin == SKIN_POKEMON) {
        // Miệng mèo :3
        tft.drawPixel(x, y + 2, 0x0000);
        tft.drawPixel(x - 1, y + 3, 0x0000);
        tft.drawPixel(x + 1, y + 3, 0x0000);
    }
    else { // Pacman, Ghost
        if (mouthGap > 0) {
            if (direction == 1) tft.fillTriangle(x, y, x + size + 2, y - mouthGap, x + size + 2, y + mouthGap, BG_COLOR);
            else tft.fillTriangle(x, y, x - size - 2, y - mouthGap, x - size - 2, y + mouthGap, BG_COLOR);
        } else {
            int mx = (direction == 1) ? x + 2 : x - 2;
            tft.drawLine(x, y + (size/3), mx, y + (size/3), 0x0000);
        }
    }
}

// Wrapper xóa cũ vẽ mới - Đã tối ưu để xóa vệt
void drawHoloPacman(int x, int y, int size, int mouthGap, bool eOpen, int direction) {
    uint16_t BAR_BG = 0x1082; 
    int STATUS_BAR_H = 20; 

    // [FIX VỆT] Tăng kích thước vùng xóa tối đa cho cả Pokemon (tai) và Robot
    if (lastDrawnX != -1) {
        // Tăng vùng xóa lên đáng kể để bao trùm tai Pokemon và các hiệu ứng
        int offSide = lastSize + 22; 
        int offTop = lastSize + 20;  // Xóa cao lên để hết tai
        int offBottom = lastSize + 8; 

        int clrX = lastDrawnX - offSide;
        int clrY = lastDrawnY - offTop;
        int clrW = offSide * 2 + 2; 
        int clrH = offTop + offBottom;

        // Cắt bớt nếu vùng xóa chờm lên Status Bar hoặc ra ngoài màn hình
        if (clrY + clrH > STATUS_BAR_H) clrH = STATUS_BAR_H - clrY;
        if (clrY < 0) { clrH += clrY; clrY = 0; }
        if (clrX < 60) { int diff = 60 - clrX; clrX = 60; clrW -= diff; }
        if (clrX + clrW > 130) clrW = 130 - clrX;
        
        // Fill màu nền Status Bar để xóa hoàn toàn vị trí cũ
        if (clrH > 0 && clrW > 0) tft.fillRect(clrX, clrY, clrW, clrH, BAR_BG);
    }
    
    uint16_t color = getMoodColor(currentMood);
    renderAiAvatar(x, y, size, mouthGap, eOpen, direction, color);
    drawEquippedItems(x, y, size);
    
    lastDrawnX = x; lastDrawnY = y; lastSize = size; lastColor = color; 
}

void drawHoloBubble(int pacX, int pacY, String msg) {
    int w = 110; int h = 35; 
    int x = pacX - (w / 2);
    if (x < 5) x = 5; if (x + w > 155) x = 155 - w;
    int y = 25; 

    uint16_t borderColor = getMoodColor(currentMood);

    tft.fillRect(x, y, w, h, 0x0000); 
    tft.drawRoundRect(x, y, w, h, 4, borderColor);
    tft.drawLine(x + w/2, y, pacX, pacY + AI_BASE_SIZE, borderColor); 

    tft.setFont(NULL); tft.setTextSize(1); tft.setTextColor(AI_TEXT_COLOR);
    
    if (isAiThinking) {
         int dotCount = (millis() / 300) % 4;
         String dots = ""; for(int i=0; i<dotCount; i++) dots += ".";
         tft.setCursor(x + 20, y + 12); tft.print("Dang quet" + dots);
         return;
    }

    int maxChars = 18; 
    if (msg.length() > maxChars) {
        int split = msg.lastIndexOf(' ', maxChars);
        if (split == -1) split = maxChars;
        tft.setCursor(x + 2, y + 6); tft.print(msg.substring(0, split));
        tft.setCursor(x + 2, y + 18); tft.print(msg.substring(split + 1));
    } else {
        tft.setCursor(x + (w - (msg.length() * 6)) / 2, y + 12); tft.print(msg);
    }
}

bool ai_loop() {
    if (!aiEnabled && currentMood != MOOD_ALERT) return false;
    checkProximitySensor();
    unsigned long currentMs = millis();
    bool requestRedraw = false;

    if (isAiThinking) {
        if (currentMs - aiThinkingStart > THINKING_DELAY) {
            isAiThinking = false;
            currentMood = pendingTargetMood;
            currentMessage = pendingTargetMsg;
            playAiVoice(currentMood);
            aiSpeaking = true; aiSpeakTime = currentMs; lastSpeechTime = currentMs;
            messageDuration = 3000 + (currentMessage.length() * 100);
            requestRedraw = true; 
        } 
    }

    static unsigned long lastTrigger = 0; static unsigned long lastEnergyTick = 0;
    if (currentMs - lastEnergyTick > 10000) { if (energyLevel > 0) energyLevel--; lastEnergyTick = currentMs; }
    if (currentMood != MOOD_ALERT && !isAiThinking && currentMs - lastTrigger > 60000) { if (!aiSpeaking) triggerAiReaction(AI_EVENT_IDLE); lastTrigger = currentMs; }

    if (currentMs - lastAnimTime > 30) { 
        currentAiY += aiVy; aiVy += 0.4; 
        if (currentAiY >= AI_BASE_Y) {
            currentAiY = AI_BASE_Y;
            if (currentMood == MOOD_HYPER || (currentMood == MOOD_HAPPY && random(0, 100) < 5)) aiVy = -2.0; 
            else aiVy = 0;
        }
        if (currentMood == MOOD_SLEEPY) aiSpeed = 0.2; else if (currentMood == MOOD_HYPER) aiSpeed = 3.0; else aiSpeed = 1.0; 
        if (!aiSpeaking && currentMood != MOOD_THINKING && !isAiThinking) {
            currentAiX += (aiSpeed * aiDirection);
            if (currentAiX >= AI_MAX_X) { currentAiX = AI_MAX_X; aiDirection = -1; } 
            else if (currentAiX <= AI_MIN_X) { currentAiX = AI_MIN_X; aiDirection = 1; }
        }
        breathPhase += 0.1; float breathOffset = sin(breathPhase) * 1.0; 
        int currentSize = AI_BASE_SIZE + (int)breathOffset;
        if (currentMood == MOOD_HYPER) currentSize += 1; 
        if (currentMs - lastBlinkTime > random(2000, 5000)) { eyeOpen = !eyeOpen; if (eyeOpen) lastBlinkTime = currentMs; }
        int mouthGap = 0;
        if (aiSpeaking) mouthGap = (int)(abs(sin(currentMs / 40.0)) * 4); else mouthGap = (int)(abs(sin(currentAiX / 5.0)) * 5);
        drawHoloPacman((int)currentAiX, (int)currentAiY, currentSize, mouthGap, eyeOpen, aiDirection);
        lastAnimTime = currentMs;
    }

    static bool bubbleDrawn = false;
    if (aiSpeaking || isAiThinking) {
        if (!bubbleDrawn || (millis() % 200 == 0)) { drawHoloBubble((int)currentAiX, (int)currentAiY, currentMessage); bubbleDrawn = true; }
        if (!isAiThinking) {
            if (currentMs - aiSpeakTime > messageDuration) {
                aiSpeaking = false; bubbleDrawn = false;
                tft.fillRect(0, 25, 160, 40, 0x0000); 
                if (currentMood == MOOD_ALERT) currentMood = MOOD_HAPPY; 
                requestRedraw = true; 
            }
        }
    }
    return requestRedraw;
}

#endif