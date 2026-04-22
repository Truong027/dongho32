#ifndef AI_CORE_H
#define AI_CORE_H

#include "config.h"
#include "ai_extensions.h" 

// --- CẤU HÌNH AI (HYPER CORE VER) ---
#define AI_BASE_Y      13   
#define AI_SIZE        6    
#define AI_BASE_SIZE   6    
#define AI_MIN_X       82   
#define AI_MAX_X       108  
#define AI_START_X     95   

#define AI_COLOR       0xFFE0 
#define AI_TEXT_COLOR  0xFFFF 
#define AI_BUBBLE_EDGE 0x07E0 

// --- ĐỊNH NGHĨA SKIN (GIAO DIỆN) ---
#define SKIN_PACMAN     0
#define SKIN_GHOST      1 
#define SKIN_ROBOT      2 
#define SKIN_POKEMON    3 // [MỚI] Pikachu Style
#define SKIN_ANIME      4 // [MỚI] Anime Cute Style
#define TOTAL_SKINS     5 

int currentSkin = SKIN_PACMAN; 

// Định nghĩa các sự kiện kích hoạt AI
#define AI_EVENT_IDLE       0
#define AI_EVENT_WAKEUP     1
#define AI_EVENT_RELAY_ON   2
#define AI_EVENT_RELAY_OFF  3
#define AI_EVENT_WIFI_LOST  4
#define AI_EVENT_TEMP_HIGH  5
#define AI_EVENT_HOURLY     6 
#define AI_EVENT_ALARM      7 
#define AI_EVENT_TOUCH      8 
#define AI_EVENT_BORED      11 

extern bool aiEnabled; 
extern String lunarDateStr;
extern int8_t tempHistoryOut[10]; 

extern String weatherDesc;
extern float currentTemp;
extern int currentHum;
extern int currentAQI;
extern bool rainAlertEnabled;

extern bool isScreenOff;
extern void applyBrightness(); 
extern unsigned long lastTouchTime;

void triggerAiReaction(int eventType);

// Trạng thái chung
bool aiSpeaking = false;      
unsigned long aiSpeakTime = 0; 
unsigned long messageDuration = 3000; 
String currentMessage = "";
int currentMood = MOOD_HAPPY;

// --- BIẾN CHO HỆ THỐNG SUY NGHĨ ---
bool isAiThinking = false;          
unsigned long aiThinkingStart = 0;  
int pendingTargetMood = MOOD_HAPPY; 
String pendingTargetMsg = "";       
const int THINKING_DELAY = 1200;    

// --- AI MEMORY & EMOTION SYSTEM ---
static int lastProxZone = -1;
static unsigned long lastHumanSeenTime = 0;
static bool humanPresent = false;    
static int closeCount = 0;           
static unsigned long lastHumanInteraction = 0; 
static unsigned long lastSpeechTime = 0; 
static int friendshipScore = 50; 

// Biến chuyển động
float currentAiX = AI_START_X; 
float currentAiY = AI_BASE_Y; 
float aiVy = 0; 
float aiSpeed = 1.5;           
int aiDirection = -1;          
int lastDrawnX = -1;           
int lastDrawnY = -1;
int lastSize = AI_SIZE;        
int lastMouthGap = 0;
bool lastEyeOpen = true;
int lastDirection = -1;
uint16_t lastColor = 0; 

unsigned long lastAnimTime = 0;
unsigned long lastBlinkTime = 0;
bool eyeOpen = true;
int animOffsetY = 0;
float animPhase = 0.0;
float breathPhase = 0.0; 
int energyLevel = 100;   

// --- HÀM LẤY MÀU ---
uint16_t getMoodColor(int mood) {
    if (mood == MOOD_HYPER) {
        long t = millis() / 100;
        int r = (sin(t * 0.1) + 1) * 15; 
        int g = (sin(t * 0.1 + 2) + 1) * 31; 
        int b = (sin(t * 0.1 + 4) + 1) * 15; 
        return (r << 11) | (g << 5) | b;
    }
    
    // Màu đặc trưng cho từng Skin
    if (currentSkin == SKIN_POKEMON) return 0xFFE0; // Vàng Pikachu
    if (currentSkin == SKIN_ANIME) return 0xFDD7;   // Màu da/Hồng phấn
    if (currentSkin == SKIN_ROBOT) return 0x8410;   // Xám
    if (currentSkin == SKIN_GHOST) return 0xF81F;   // Tím hồng (Ma)

    if (currentTemp > 0 && currentTemp < 20) return 0x07FF; // Lạnh -> Xanh

    switch(mood) {
        case MOOD_HAPPY:    return 0xFFE0;
        case MOOD_ANGRY:    return 0xF800; 
        case MOOD_SAD:      return 0x0415; 
        case MOOD_SLEEPY:   return 0x8410; 
        case MOOD_LOVELY:   return 0xFD20; 
        case MOOD_ALERT:    return 0xF800; 
        case MOOD_THINKING: return 0x07FF; 
        default:            return 0xFFE0; 
    }
}

// --- HỆ THỐNG ÂM THANH ---
void aiTone(int freq, int duration) {
    if (freq <= 0) { delay(duration); return; }
    ledcWriteTone(BUZZER_PIN, freq);
    delay(duration);
    ledcWriteTone(BUZZER_PIN, 0);
}

void playAiVoice(int mood) {
    if (!aiEnabled || mood == MOOD_ALERT) return; 
}

// --- LOGIC CẢM XÚC ---
void adjustFriendship(int delta) {
    friendshipScore += delta;
    if (friendshipScore > 100) friendshipScore = 100;
    if (friendshipScore < 0) friendshipScore = 0;
}

void startThinking(int targetMood, String targetMsg) {
    if (isAiThinking && targetMood == pendingTargetMood) return;
    isAiThinking = true;
    aiThinkingStart = millis();
    currentMood = MOOD_THINKING; 
    
    // [NÂNG CẤP] Suy nghĩ theo phong cách Skin
    if (currentSkin == SKIN_POKEMON) currentMessage = "Pika...?";
    else if (currentSkin == SKIN_ANIME) currentMessage = "Etto...";
    else if (currentSkin == SKIN_ROBOT) currentMessage = "Processing...";
    else currentMessage = "Hmm...";     
    
    aiSpeaking = true;           
    aiSpeakTime = millis();
    pendingTargetMood = targetMood;
    pendingTargetMsg = targetMsg;
}

void setMoodSoft(int newMood, String msg) {
    if (currentMood == newMood) return;
    
    if (friendshipScore < 30 && newMood == MOOD_HAPPY) {
        newMood = MOOD_THINKING; 
        // [NÂNG CẤP] Dỗi theo phong cách Skin
        if (currentSkin == SKIN_POKEMON) msg = "Pika pika...";
        else if (currentSkin == SKIN_ANIME) msg = "Doi roi day...";
        else if (currentSkin == SKIN_ROBOT) msg = "Error 404.";
        else msg = "Van dang doi...";
    }

    if ((currentMood == MOOD_HAPPY && newMood == MOOD_ANGRY) ||
        (currentMood == MOOD_SLEEPY && newMood == MOOD_HYPER) ||
        (currentMood == MOOD_SAD && newMood == MOOD_HYPER)) {
        startThinking(newMood, msg);
    } else {
        currentMood = newMood;
        currentMessage = msg;
        aiSpeaking = true;
        aiSpeakTime = millis();
        lastSpeechTime = millis();
        messageDuration = 3000 + (currentMessage.length() * 100);
    }
}

// --- CẢM BIẾN TIẾP CẬN (PROXIMITY) ---
unsigned long proxTimer = 0;

int readDistance() {
    long totalDuration = 0;
    int validReadings = 0;
    for(int i=0; i<3; i++) {
        digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
        digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);
        long duration = pulseIn(ECHO_PIN, HIGH, 25000); 
        if (duration > 0) { totalDuration += duration; validReadings++; }
        delay(3);
    }
    if (validReadings == 0) return 999;
    return (totalDuration / validReadings) * 0.034 / 2;
}

void checkProximitySensor() {
    if (millis() - proxTimer < 250) return; 
    proxTimer = millis();
    int dist = readDistance();
    if (dist <= 0 || dist > 400) return; 

    if (isScreenOff && dist < 100) {
        isScreenOff = false; applyBrightness(); lastTouchTime = millis();
        if (aiEnabled) {
            triggerAiReaction(AI_EVENT_WAKEUP);
            humanPresent = true; lastHumanSeenTime = millis(); adjustFriendship(5);
        }
        return;
    }

    if (isScreenOff || !aiEnabled) return;
    if (aiSpeaking && millis() - aiSpeakTime < 2000 && currentMood != MOOD_THINKING) return;

    static int stableZone = 4; static int tempZone = 4; static unsigned long zoneStableStartTime = 0;
    int detectedZone = 4;
    if (dist < 8) detectedZone = 0; else if (dist < 25) detectedZone = 1; else if (dist < 70) detectedZone = 2; else if (dist < 150) detectedZone = 3; 
    
    if (detectedZone != tempZone) { tempZone = detectedZone; zoneStableStartTime = millis(); }

    if (millis() - zoneStableStartTime > 400) {
        if (stableZone != tempZone) {
            stableZone = tempZone;
            if (stableZone != 4) {
                humanPresent = true; lastHumanSeenTime = millis();
                if (lastProxZone == 4) triggerAiReaction(AI_EVENT_WAKEUP); 
                else {
                    switch (stableZone) {
                        case 0: 
                            adjustFriendship(-2); 
                            // [NÂNG CẤP] Thoại khi bị ép sát
                            if (currentSkin == SKIN_POKEMON) setMoodSoft(MOOD_ANGRY, "Pikaaa!!!");
                            else if (currentSkin == SKIN_ANIME) setMoodSoft(MOOD_ANGRY, "Baka! Gan qua!");
                            else if (currentSkin == SKIN_ROBOT) setMoodSoft(MOOD_ANGRY, "Warn: Too Close");
                            else setMoodSoft(MOOD_ANGRY, "Tranh ra!");
                            break;
                        case 1: 
                            if (friendshipScore > 70) {
                                if (currentSkin == SKIN_POKEMON) setMoodSoft(MOOD_LOVELY, "Pi..Ka..Chu <3");
                                else if (currentSkin == SKIN_ANIME) setMoodSoft(MOOD_LOVELY, "Master <3");
                                else if (currentSkin == SKIN_ROBOT) setMoodSoft(MOOD_LOVELY, "User: Loved");
                                else setMoodSoft(MOOD_LOVELY, "Hiii master <3");
                            } else {
                                if (currentSkin == SKIN_POKEMON) setMoodSoft(MOOD_HYPER, "Pika?");
                                else if (currentSkin == SKIN_ANIME) setMoodSoft(MOOD_HYPER, "Gan qua roi...");
                                else setMoodSoft(MOOD_HYPER, "Soi ki the?");
                            }
                            break;
                        case 2: 
                            adjustFriendship(1); 
                            if (currentSkin == SKIN_POKEMON) setMoodSoft(MOOD_HAPPY, "Pika Pika!");
                            else if (currentSkin == SKIN_ANIME) setMoodSoft(MOOD_HAPPY, "Konbanwa!");
                            else if (currentSkin == SKIN_ROBOT) setMoodSoft(MOOD_HAPPY, "User Detected");
                            else setMoodSoft(MOOD_HAPPY, "Chao ban hien.");
                            break;
                        case 3: 
                            if (currentMood != MOOD_HAPPY) {
                                if (currentSkin == SKIN_POKEMON) setMoodSoft(MOOD_HAPPY, "Pika..");
                                else if (currentSkin == SKIN_ANIME) setMoodSoft(MOOD_HAPPY, "Thay ban roi");
                                else setMoodSoft(MOOD_HAPPY, "Toi van thay ban.");
                            }
                            break;
                    }
                }
            }
            lastProxZone = stableZone;
        }
    }
    if (humanPresent && stableZone == 4 && millis() - lastHumanSeenTime > 10000) {
        humanPresent = false; closeCount = 0;
        if (currentSkin == SKIN_POKEMON) setMoodSoft(MOOD_SAD, "Pika...");
        else if (currentSkin == SKIN_ANIME) setMoodSoft(MOOD_SAD, "Matte...");
        else if (currentSkin == SKIN_ROBOT) setMoodSoft(MOOD_SAD, "User Left.");
        else setMoodSoft(MOOD_SAD, "Tam biet.");
    }
    if (humanPresent && stableZone <= 3) {
        lastHumanSeenTime = millis(); 
        if (millis() - lastSpeechTime > 25000 && !aiSpeaking && !isAiThinking) triggerAiReaction(AI_EVENT_BORED);
    }
}

// --- LOGIC PHẢN ỨNG THÔNG MINH ---
String getGreeting() {
    int h = hour();
    // [NÂNG CẤP] Chào hỏi theo Skin
    if (h >= 22 || h < 5) {
        if (currentSkin == SKIN_POKEMON) return "Pika..zzZ";
        if (currentSkin == SKIN_ANIME) return "Oyasumi..";
        if (currentSkin == SKIN_ROBOT) return "Sleep Mode.";
        return "Dem thanh vang.";
    }
    if (h >= 5 && h < 11) {
        if (currentSkin == SKIN_POKEMON) return "Pika Pika!";
        if (currentSkin == SKIN_ANIME) return "Ohayo!";
        if (currentSkin == SKIN_ROBOT) return "Good Morning.";
        return "Morning.";
    }
    if (currentSkin == SKIN_POKEMON) return "Pikachu!";
    if (currentSkin == SKIN_ANIME) return "Master!";
    if (currentSkin == SKIN_ROBOT) return "Online.";
    return "Hello Master.";
}

String getSmartThought(int inputEvent) {
    int h = hour();
    energyLevel += 20; if (energyLevel > 100) energyLevel = 100; 

    if (inputEvent == AI_EVENT_BORED) {
        adjustFriendship(1); currentMood = MOOD_THINKING;
        if (currentSkin == SKIN_POKEMON) return "Pika?";
        if (currentSkin == SKIN_ANIME) return "Nee nee...";
        if (currentSkin == SKIN_ROBOT) return "Waiting...";
        return "Sao dung im the?";
    }
    
    if (inputEvent == AI_EVENT_TOUCH) { 
        lastHumanInteraction = millis(); 
        currentSkin++; if (currentSkin >= TOTAL_SKINS) currentSkin = 0; // Đổi skin xoay vòng
        
        if (energyLevel < 40) {
            energyLevel = 100; adjustFriendship(10); currentMood = MOOD_LOVELY;
            if (currentSkin == SKIN_POKEMON) return "Pikaaa <3";
            if (currentSkin == SKIN_ANIME) return "Arigatou!";
            return "Cam on!";
        }
        adjustFriendship(5); 
        currentMood = MOOD_HAPPY; 
        
        // [NÂNG CẤP] Câu nói khi đổi skin
        if (currentSkin == SKIN_POKEMON) return "Pika Chu!";
        if (currentSkin == SKIN_ANIME) return "Yay! Skin moi!";
        if (currentSkin == SKIN_ROBOT) return "System Up.";
        if (currentSkin == SKIN_GHOST) return "Boo!";
        return "Bien hinh!"; 
    }

    if (inputEvent == AI_EVENT_ALARM) return "ALARM! ALARM!";
    if (inputEvent == AI_EVENT_WAKEUP) { lastHumanInteraction = millis(); currentMood = MOOD_LOVELY; return getGreeting(); }
    if (inputEvent == AI_EVENT_HOURLY) return String(h) + "H"; 
    
    if (inputEvent == AI_EVENT_RELAY_ON) {
        if (currentSkin == SKIN_POKEMON) return "Pika!";
        if (currentSkin == SKIN_ANIME) return "Hai!";
        if (currentSkin == SKIN_ROBOT) return "ON.";
        return "Da bat.";
    }
    if (inputEvent == AI_EVENT_RELAY_OFF) {
        if (currentSkin == SKIN_POKEMON) return "Chu..";
        if (currentSkin == SKIN_ANIME) return "Hai~";
        if (currentSkin == SKIN_ROBOT) return "OFF.";
        return "Da tat.";
    }
    
    if (inputEvent == AI_EVENT_WIFI_LOST) { 
        currentMood = MOOD_SAD; 
        if (currentSkin == SKIN_POKEMON) return "Pika..no..";
        if (currentSkin == SKIN_ANIME) return "Mat mang roi";
        return "No WiFi..."; 
    }

    bool skipEnvironment = (millis() - lastHumanInteraction < 120000);
    String w = weatherDesc; w.toLowerCase();
    
    if (!skipEnvironment) {
        if (WiFi.status() != WL_CONNECTED) { currentMood = MOOD_THINKING; return "Tim WiFi..."; }
        
        if (w.indexOf("rain") >= 0 && rainAlertEnabled) { 
            currentMood = MOOD_ALERT; 
            if (currentSkin == SKIN_POKEMON) return "Pika!!";
            if (currentSkin == SKIN_ANIME) return "Ame da!";
            if (currentSkin == SKIN_ROBOT) return "Rain Alert";
            return "Dang mua.";
        }
        if (currentTemp > 33) { 
            currentMood = MOOD_ANGRY; 
            if (currentSkin == SKIN_POKEMON) return "Pika..hot";
            if (currentSkin == SKIN_ANIME) return "Atsui..";
            if (currentSkin == SKIN_ROBOT) return "Overheat";
            return "Nong chay mo!";
        }
        if (currentTemp < 20 && currentTemp != 0) { 
            currentMood = MOOD_SLEEPY; 
            if (currentSkin == SKIN_POKEMON) return "Brrr..";
            if (currentSkin == SKIN_ANIME) return "Samui..";
            if (currentSkin == SKIN_ROBOT) return "Low Temp";
            return "Lanh... run...";
        }
    }

    if (h >= 23 || h < 5) { currentMood = MOOD_SLEEPY; return "Zzz..."; }
    if (energyLevel < 20) { currentMood = MOOD_SLEEPY; return "Doi..."; }

    currentMood = MOOD_HAPPY;
    if (currentSkin == SKIN_POKEMON) return "Pikachu!";
    if (currentSkin == SKIN_ANIME) return "Ganbatte!";
    if (currentSkin == SKIN_ROBOT) return "Standby.";
    return "Pacman AI.";
}

void triggerAiReaction(int eventType) {
    if (!aiEnabled && eventType != AI_EVENT_ALARM) return;
    
    isAiThinking = false;
    aiSpeaking = false; 
    currentMessage = getSmartThought(eventType);
    
    messageDuration = 3000 + (currentMessage.length() * 100);
    if (eventType == AI_EVENT_ALARM) messageDuration = 60000;
    
    if (eventType == AI_EVENT_RELAY_ON || eventType == AI_EVENT_RELAY_OFF) { aiTone(1200, 20); } 
    if (eventType == AI_EVENT_TOUCH || eventType == AI_EVENT_WAKEUP) { aiVy = -3.0; }

    aiSpeaking = true;
    aiSpeakTime = millis();
    lastSpeechTime = millis(); 
    animPhase = 0;
}

void wakeUpAi() { triggerAiReaction(AI_EVENT_WAKEUP); }
void onAiTouched() { if (aiEnabled) triggerAiReaction(AI_EVENT_TOUCH); }

#endif