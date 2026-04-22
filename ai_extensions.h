#ifndef AI_EXTENSIONS_H
#define AI_EXTENSIONS_H

#include "config.h"
#include <Preferences.h>

// Forward Declaration
extern int getChips();
extern void updateChips(int delta);
extern bool isPressed(int pin);
extern void waitForRelease();
extern void playToneGame(int freq, int duration);

// [LIÊN KẾT] Biến từ ai_assistant.h
extern int energyLevel; 
extern int currentMood; 

// [FIX] ĐỊNH NGHĨA ĐẦY ĐỦ CÁC TRẠNG THÁI CẢM XÚC (MOODS)
#define MOOD_HAPPY     0
#define MOOD_SAD       1
#define MOOD_SLEEPY    2
#define MOOD_ALERT     3
#define MOOD_ANGRY     4
#define MOOD_LOVELY    5
#define MOOD_THINKING  6
#define MOOD_HYPER     7

// --- ĐỊNH NGHĨA VẬT PHẨM ---
#define ITEM_NONE       0
// --- TRANG BỊ CƠ BẢN ---
#define ITEM_BOW        1 
#define ITEM_GLASSES    2 
#define ITEM_HEADPHONE  3 
#define ITEM_HAT        4 
#define ITEM_CROWN      5 
#define ITEM_MASK       6 
#define ITEM_CAT_EARS   7 
#define ITEM_POTTER     8 
#define ITEM_MUSTACHE   9 
#define ITEM_SCARF      10
#define ITEM_FLOWER     11

// --- TRANG BỊ VIP ---
#define ITEM_WINGS      21 
#define ITEM_WEAPON_17  22 

// --- CONSUMABLES ---
#define ITEM_COOKIE     12
#define ITEM_COFFEE     13
#define ITEM_PIZZA      14
#define ITEM_BURGER     15
#define ITEM_SUSHI      16
#define ITEM_ICECREAM   17
#define ITEM_SODA       18
#define ITEM_CAKE       19
#define ITEM_MILK       20
#define ITEM_JEWEL      23 // Ngọc Đập Đồ

struct ShopItem {
    int id;
    String name;
    int price;
    bool isConsumable; 
};

const ShopItem SHOP_LIST[] = {
    // --- VIP ITEMS (GIÁ ĐẮT) ---
    {ITEM_WINGS,     "Canh Thien Than", 50000, false},
    {ITEM_WEAPON_17, "Kiem Rong VIP",   100000, false},
    {ITEM_JEWEL,     "Ngoc Tam Linh",   1000,  true},

    // --- TRANG BỊ THƯỜNG ---
    {ITEM_BOW,       "No Do (Cute)",    200, false},
    {ITEM_GLASSES,   "Kinh Ram (Ngau)", 350, false},
    {ITEM_HEADPHONE, "Tai Nghe DJ",     500, false},
    {ITEM_HAT,       "Mu Luoi Trai",    300, false},
    {ITEM_CROWN,     "Vuong Mien VIP",  5000, false},
    {ITEM_MASK,      "Khau Trang Y Te", 150, false},
    {ITEM_CAT_EARS,  "Tai Meo Neko",    400, false},
    {ITEM_POTTER,    "Kinh Harry P.",   450, false},
    {ITEM_MUSTACHE,  "Ria Mep Quy Ong", 100, false},
    {ITEM_SCARF,     "Khan Quang Co",   250, false},
    {ITEM_FLOWER,    "Bong Hoa Nho",    120, false},
    
    // --- ĐỒ ĂN ---
    {ITEM_COOKIE,    "Banh Quy (Snack)", 50,  true},
    {ITEM_COFFEE,    "Ca Phe (Tinh Tao)",80,  true},
    {ITEM_MILK,      "Sua Tuoi (Healthy)",40, true},
    {ITEM_SODA,      "Nuoc Ngot (Gas)",  40,  true},
    {ITEM_ICECREAM,  "Kem Oc Que (Mat)", 60,  true},
    {ITEM_BURGER,    "Hamburger (No)",   120, true},
    {ITEM_PIZZA,     "Pizza (Party)",    150, true},
    {ITEM_CAKE,      "Banh Kem (SinhNhat)", 200, true},
    {ITEM_SUSHI,     "Sushi (Sang Chanh)", 250, true}
};
const int SHOP_COUNT = 23; 

// --- QUẢN LÝ KHO ĐỒ & LEVEL ---
Preferences shopPrefs;
int currentEquippedItem = ITEM_NONE;
int ownedItemsMask = 0; 
int itemLevels[50] = {0}; // Mảng lưu cấp độ cho từng ID vật phẩm (0-17)

void initAiShop() {
    shopPrefs.begin("ai_store", false); 
    currentEquippedItem = shopPrefs.getInt("equipped", ITEM_NONE);
    ownedItemsMask = shopPrefs.getInt("owned", 0); 
    
    // Tải Level của các món đồ
    for(int i=0; i<30; i++) { // Quét qua các ID có thể có
        char k[10]; sprintf(k, "lv_%d", i);
        itemLevels[i] = shopPrefs.getInt(k, 0);
    }
    
    shopPrefs.end();
}

bool isItemOwned(int itemID) {
    return (ownedItemsMask & (1 << itemID));
}

void setItemOwned(int itemID) {
    ownedItemsMask |= (1 << itemID);
    shopPrefs.begin("ai_store", false);
    shopPrefs.putInt("owned", ownedItemsMask);
    shopPrefs.end();
}

void saveEquippedItem(int itemID) {
    shopPrefs.begin("ai_store", false);
    shopPrefs.putInt("equipped", itemID);
    shopPrefs.end();
    currentEquippedItem = itemID;
}

void saveItemLevel(int itemID, int level) {
    if (itemID <= 0 || itemID >= 50) return;
    itemLevels[itemID] = level;
    shopPrefs.begin("ai_store", false);
    char k[10]; sprintf(k, "lv_%d", itemID);
    shopPrefs.putInt(k, level);
    shopPrefs.end();
}

// Hàm lấy chỉ số may mắn
int getLuckBonus() {
    if (currentEquippedItem == ITEM_NONE) return 0;
    return itemLevels[currentEquippedItem] * 2; 
}

// Hàm lấy màu hiệu ứng theo cấp độ
// [MOD] Cập nhật mốc màu theo yêu cầu:
// +1 -> +6: Xanh Dương (0x07FF)
// +7 -> +12: Vàng (0xFFE0)
// +13 -> +17: Cầu Vồng
uint16_t getEffectColor(int level) {
    if (level >= 13) {
        // Cầu vồng nhấp nháy (+13 đến +17)
        return (millis() / 100 % 2 == 0) ? 0xF81F : 0x07E0; 
    } else if (level >= 7) {
        return 0xFFE0; // Vàng kim (+7 đến +12)
    } else if (level >= 1) {
        return 0x07FF; // Xanh Cyan (+1 đến +6)
    }
    return 0xFFFF; // Trắng mặc định (Level 0)
}

// --- [FIX] HÀM HỖ TRỢ VẼ GIỚI HẠN (CLIP) ---
// Giới hạn không vẽ đè xuống dưới status bar
// [MOD] Đặt giới hạn là 19 (tức dòng pixel thứ 20) để khớp với vùng xóa 20px (0-19)
#define STATUS_BAR_LIMIT 19

void drawClippedPixel(int x, int y, uint16_t color) {
    // Chỉ vẽ nếu y nằm trong vùng Status Bar (0 đến 19)
    // Thêm kiểm tra x và y >= 0 để tránh lỗi vẽ ra ngoài vùng nhớ
    if (x >= 0 && x < 160 && y >= 0 && y <= STATUS_BAR_LIMIT) {
        tft.drawPixel(x, y, color);
    }
}

// Thuật toán Bresenham vẽ đường tròn có cắt bớt phần dưới
void drawClippedCircle(int x0, int y0, int r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawClippedPixel(x0, y0 + r, color);
    drawClippedPixel(x0, y0 - r, color);
    drawClippedPixel(x0 + r, y0, color);
    drawClippedPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawClippedPixel(x0 + x, y0 + y, color);
        drawClippedPixel(x0 - x, y0 + y, color);
        drawClippedPixel(x0 + x, y0 - y, color);
        drawClippedPixel(x0 - x, y0 - y, color);
        drawClippedPixel(x0 + y, y0 + x, color);
        drawClippedPixel(x0 - y, y0 + x, color);
        drawClippedPixel(x0 + y, y0 - x, color);
        drawClippedPixel(x0 - y, y0 - x, color);
    }
}

// --- HỆ THỐNG VISUAL LEVEL ---
// [MOD] Cập nhật mốc hiệu ứng theo yêu cầu
void drawLevelEffect(int x, int y, int size, int level) {
    if (level <= 0) return;
    
    uint16_t auraColor = 0xFFFF;
    bool showGlow = false;
    bool showSparkle = false;

    if (level >= 13) { // Siêu Cấp (+13 -> +17)
        auraColor = 0xF81F; // Hồng tím (Magenta) - Base cho cầu vồng
        showGlow = true;
        showSparkle = true;
    } else if (level >= 7) { // Cao Cấp (+7 -> +12)
        auraColor = 0xFFE0; // Vàng kim (Gold)
        showGlow = true;
    } else if (level >= 1) { // Sơ Cấp (+1 -> +6)
        auraColor = 0x07FF; // Xanh Cyan
        showGlow = true; // Nhẹ hơn
    }

    // [MOD] BỎ VẼ SỐ CẤP ĐỘ KHI DI CHUYỂN
    // Việc này giúp tránh bị vệt chữ và "loạn xạ" khi AI chạy nhanh.
    /*
    tft.setTextSize(1);
    tft.setTextColor(auraColor);
    tft.setCursor(x + size + 5, y - size - 5);
    tft.print("+" + String(level));
    */

    // Vẽ vòng hào quang (Aura) - [FIX] SỬ DỤNG DRAW CLIPPED
    if (showGlow) {
        if (level >= 13) {
            // Hào quang cầu vồng nhấp nháy cho cấp +13 trở lên
            uint16_t rainbow = (millis() / 100 % 2 == 0) ? 0xF800 : 0x07E0;
            drawClippedCircle(x, y, size + 10, rainbow);
            drawClippedCircle(x, y, size + 11, auraColor);
        } else {
            drawClippedCircle(x, y, size + 9, auraColor);
        }
    }

    // Vẽ lấp lánh (Sparkles) - [FIX] SỬ DỤNG DRAW CLIPPED
    if (showSparkle || (level >= 7 && (millis()/200)%2==0)) {
        int sx = x + (random(0,2) ? size+8 : -size-8);
        int sy = y + (random(0,2) ? size+8 : -size-8);
        
        drawClippedPixel(sx, sy, 0xFFFF);
        drawClippedPixel(sx+1, sy, auraColor);
        drawClippedPixel(sx-1, sy, auraColor);
        drawClippedPixel(sx, sy+1, auraColor);
        drawClippedPixel(sx, sy-1, auraColor);
    }
}

// --- HÀM VẼ TRANG BỊ ---
void drawEquippedItems(int x, int y, int size) {
    if (currentEquippedItem == ITEM_NONE) return;

    int offset = size + 2; 
    int level = itemLevels[currentEquippedItem];
    uint16_t effectColor = getEffectColor(level);

    // --- VIP ITEMS ---
    if (currentEquippedItem == ITEM_WINGS) {
        // Cánh Thiên Thần
        uint16_t wingColor = (level >= 1) ? effectColor : 0xFFFF;
        tft.fillTriangle(x - size - 2, y, x - size - 12, y - 8, x - size - 2, y + 6, wingColor);
        tft.drawTriangle(x - size - 2, y, x - size - 12, y - 8, x - size - 2, y + 6, 0x07FF); 
        tft.fillTriangle(x + size + 2, y, x + size + 12, y - 8, x + size + 2, y + 6, wingColor);
        tft.drawTriangle(x + size + 2, y, x + size + 12, y - 8, x + size + 2, y + 6, 0x07FF);
    }
    else if (currentEquippedItem == ITEM_WEAPON_17) {
        // Kiếm Rồng
        uint16_t bladeColor = (level >= 1) ? effectColor : 0x07FF;
        int wx = x + size + 4; 
        int wy = y + 2;
        tft.drawLine(wx, wy, wx + 8, wy - 12, bladeColor); 
        tft.drawLine(wx + 1, wy, wx + 9, wy - 12, 0xFFFF);
        tft.drawLine(wx - 2, wy, wx + 2, wy + 2, 0xF800); 
        tft.drawPixel(wx, wy, 0xFFE0); 
    }
    // --- CÁC ITEM CŨ (Giữ nguyên) ---
    else if (currentEquippedItem == ITEM_BOW) {
        uint16_t c = 0xF800; 
        tft.fillTriangle(x - offset - 4, y - offset, x - offset, y - offset + 4, x - offset, y - offset - 4, c);
        tft.fillTriangle(x - offset + 4, y - offset, x - offset, y - offset + 4, x - offset, y - offset - 4, c);
        tft.fillCircle(x - offset, y - offset, 2, 0xFFFF);
    }
    else if (currentEquippedItem == ITEM_GLASSES) {
        uint16_t c = 0x0000; 
        tft.fillRect(x - 2, y - 2, 8, 4, c); 
        tft.drawFastHLine(x - 4, y - 1, 10, 0xFFFF); 
    }
    else if (currentEquippedItem == ITEM_HEADPHONE) {
        uint16_t c = 0x07E0; 
        tft.fillRect(x - size - 4, y - 4, 3, 8, c); 
        tft.fillRect(x + size + 1, y - 4, 3, 8, c); 
        tft.drawFastHLine(x - size, y - size - 2, size * 2, c); 
    }
    else if (currentEquippedItem == ITEM_HAT) {
        uint16_t c = 0x001F; 
        tft.fillRect(x - size, y - size - 4, size * 2, 4, c); 
        tft.fillRect(x - size, y - size - 1, size * 2 + 6, 2, c); 
    }
    else if (currentEquippedItem == ITEM_CROWN) {
        uint16_t c = 0xFFE0; 
        int topY = y - size - 6;
        tft.fillTriangle(x - 4, topY, x - 4, topY + 6, x, topY + 6, c); 
        tft.fillTriangle(x, topY - 2, x - 2, topY + 6, x + 2, topY + 6, c); 
        tft.fillTriangle(x + 4, topY, x, topY + 6, x + 4, topY + 6, c); 
        tft.drawPixel(x, topY - 3, 0xF800); 
    }
    else if (currentEquippedItem == ITEM_MASK) {
        uint16_t c = 0xFFFF;
        tft.fillRect(x - size, y + 2, size * 2, 4, c); 
        tft.drawFastHLine(x - size - 2, y + 3, 2, 0xFFFF); 
        tft.drawFastHLine(x + size, y + 3, 2, 0xFFFF);
    }
    else if (currentEquippedItem == ITEM_CAT_EARS) {
        uint16_t c = 0x0000;
        tft.fillTriangle(x - size, y - size, x - size - 4, y - size - 6, x - size + 2, y - size - 2, c); 
        tft.fillTriangle(x + size, y - size, x + size + 4, y - size - 6, x + size - 2, y - size - 2, c); 
        tft.fillTriangle(x - size - 2, y - size - 3, x - size - 3, y - size - 5, x - size, y - size - 3, 0xF81F); 
        tft.fillTriangle(x + size + 2, y - size - 3, x + size + 3, y - size - 5, x + size, y - size - 3, 0xF81F);
    }
    else if (currentEquippedItem == ITEM_POTTER) {
        uint16_t c = 0x0000;
        tft.drawCircle(x - 3, y - 2, 3, c); 
        tft.drawCircle(x + 3, y - 2, 3, c); 
        tft.drawFastHLine(x - 1, y - 2, 2, c); 
    }
    else if (currentEquippedItem == ITEM_MUSTACHE) {
        uint16_t c = 0x0000;
        tft.drawFastHLine(x - 3, y + 3, 7, c);
        tft.drawPixel(x - 3, y + 4, c); tft.drawPixel(x + 3, y + 4, c);
    }
    else if (currentEquippedItem == ITEM_SCARF) {
        uint16_t c = 0xF800;
        tft.fillRect(x - size + 1, y + size - 1, size * 2 - 2, 3, c);
        tft.fillRect(x + 2, y + size, 3, 5, c); 
    }
    else if (currentEquippedItem == ITEM_FLOWER) {
        uint16_t c = 0xF81F; 
        tft.fillCircle(x + size - 1, y - size + 1, 2, c);
        tft.drawPixel(x + size - 1, y - size + 1, 0xFFE0); 
    }

    // [MỚI] VẼ HIỆU ỨNG CẤP ĐỘ ĐÈ LÊN TRÊN CÙNG
    drawLevelEffect(x, y, size, itemLevels[currentEquippedItem]);
}

// --- MENU SHOP & KHO ĐỒ ---
void runShopMenu() {
    int cursor = 0;
    waitForRelease();
    
    unsigned long lastInputTime = 0; 
    const int INPUT_DELAY = 150; 

    while(true) {
        int myChips = getChips(); 
        tft.fillScreen(0x0000);
        
        tft.fillRect(0, 0, 160, 25, 0xF800);
        tft.setTextColor(0xFFFF); tft.setTextSize(1);
        tft.setCursor(45, 8); tft.print("SHOP AI");
        
        int itemsPerPage = 4;
        int scrollOffset = (cursor / itemsPerPage) * itemsPerPage;
        
        for(int i=0; i<itemsPerPage; i++) {
            int idx = scrollOffset + i;
            if (idx >= SHOP_COUNT) break;
            
            int y = 35 + i * 20;
            if (idx == cursor) tft.fillRect(0, y-2, 160, 18, 0x2124);
            
            ShopItem item = SHOP_LIST[idx];
            bool owned = isItemOwned(item.id);
            bool equipped = (currentEquippedItem == item.id);
            int lvl = itemLevels[item.id]; // Lấy cấp độ hiện tại

            tft.setTextColor(0xFFFF); tft.setCursor(5, y); 
            tft.print(item.name);
            
            if (item.isConsumable) {
                tft.setTextColor(0xFD20); tft.setCursor(120, y);
                tft.print(String(item.price) + "$");
            } else {
                if (equipped) {
                    tft.setTextColor(0x07E0); tft.setCursor(90, y); tft.print("[+" + String(lvl) + "]");
                } else if (owned) {
                    tft.setTextColor(0xAAAA); tft.setCursor(90, y); tft.print("KHO+" + String(lvl)); 
                } else {
                    tft.setTextColor(0xFD20); tft.setCursor(120, y);
                    tft.print(String(item.price) + "$"); 
                }
            }
        }
        
        tft.drawFastHLine(0, 115, 160, 0xFFFF);
        tft.setTextColor(0x07E0); tft.setCursor(5, 118); tft.print("Tien: " + String(myChips) + "$");
        tft.setTextColor(0xAAAA); tft.setCursor(80, 118); tft.print("UP/DN:Chon");
        
        while(true) {
            unsigned long now = millis();
            if (isPressed(TOUCH_MENU)) { waitForRelease(); return; }
            
            if (now - lastInputTime > INPUT_DELAY) {
                if (isPressed(TOUCH_UP)) { 
                    cursor = (cursor - 1 + SHOP_COUNT) % SHOP_COUNT; 
                    lastInputTime = now;
                    break; 
                }
                
                if (isPressed(TOUCH_DOWN)) { 
                    lastInputTime = now; 
                    ShopItem item = SHOP_LIST[cursor];
                    bool owned = isItemOwned(item.id);

                    if (item.isConsumable) {
                          if (myChips >= item.price) {
                            updateChips(-item.price);
                            
                            // 1. NGỌC TÂM LINH (ĐẬP ĐỒ)
                            if (item.id == ITEM_JEWEL) {
                                if (currentEquippedItem == ITEM_NONE) {
                                    tft.setTextColor(0xF800); tft.setCursor(20, 60); tft.print("CHUA MAC DO!");
                                    playToneGame(200, 300); delay(1000);
                                } else {
                                    int curLvl = itemLevels[currentEquippedItem];
                                    if (curLvl >= 17) {
                                        tft.setTextColor(0x07E0); tft.setCursor(30, 60); tft.print("MAX LEVEL!");
                                        playToneGame(500, 300); delay(1000);
                                    } else {
                                        // Animation đập đồ
                                        tft.fillScreen(0x0000);
                                        tft.setTextColor(0xFD20); tft.setCursor(30, 50); tft.print("DANG DAP +" + String(curLvl+1) + "...");
                                        for(int k=0; k<4; k++) {
                                            playToneGame(1000 + k*200, 100); delay(150);
                                            tft.drawCircle(80, 64, 10 + k*5, 0xFFFF);
                                        }
                                        
                                        // Logic Tỷ lệ thành công
                                        int chance = 100;
                                        if (curLvl >= 15) chance = 20;      // +16, +17 cực khó
                                        else if (curLvl >= 10) chance = 40; // +11 -> +15 khó vừa
                                        else if (curLvl >= 6) chance = 60;  // +6 -> +10 trung bình
                                        
                                        int roll = random(0, 100);
                                        if (roll < chance) {
                                            // Thành công
                                            tft.fillScreen(0x07E0); 
                                            tft.setTextColor(0x0000); tft.setTextSize(2);
                                            tft.setCursor(30, 50); tft.print("LEN +" + String(curLvl+1));
                                            tft.setTextSize(1);
                                            saveItemLevel(currentEquippedItem, curLvl + 1);
                                            playToneGame(3000, 100); delay(100); playToneGame(4000, 200);
                                            currentMood = MOOD_HYPER;
                                        } else {
                                            // Xịt (Rớt cấp)
                                            tft.fillScreen(0xF800); 
                                            tft.setTextColor(0xFFFF); tft.setTextSize(2);
                                            tft.setCursor(40, 50); tft.print("XIT!!");
                                            tft.setTextSize(1);
                                            
                                            int newLvl = curLvl;
                                            if (curLvl > 10) newLvl = 10; // Mốc bảo hiểm 10
                                            else if (curLvl > 6) newLvl = 6; // Mốc bảo hiểm 6
                                            else newLvl = 0; // Về mo
                                            
                                            if (newLvl != curLvl) {
                                                tft.setCursor(30, 80); tft.print("ROT VE +" + String(newLvl));
                                                saveItemLevel(currentEquippedItem, newLvl);
                                            }
                                            playToneGame(200, 600);
                                        }
                                        delay(2000);
                                    }
                                }
                            } 
                            // 2. ĐỒ ĂN THƯỜNG
                            else {
                                energyLevel = 100; currentMood = MOOD_HAPPY; 
                                playToneGame(2000, 100); 
                                tft.fillRect(20, 50, 120, 30, 0x07E0);
                                tft.setTextColor(0xFFFF); tft.setCursor(50, 60); tft.print("YUMMY!");
                                playToneGame(500, 50); delay(50); playToneGame(1000, 50); delay(500);
                            }
                         } else {
                            playToneGame(200, 300);
                            tft.fillRect(20, 50, 120, 30, 0xF800);
                            tft.setTextColor(0xFFFF); tft.setCursor(30, 60); tft.print("KHONG DU TIEN");
                            delay(800);
                         }
                    } 
                    else {
                        // LOGIC TRANG BỊ
                        if (owned) {
                            if (currentEquippedItem == item.id) {
                                saveEquippedItem(ITEM_NONE); playToneGame(500, 100);
                            } else {
                                saveEquippedItem(item.id); playToneGame(1500, 100);
                            }
                        } else {
                            if (myChips >= item.price) {
                                updateChips(-item.price);
                                setItemOwned(item.id); 
                                playToneGame(2000, 100);
                                tft.fillRect(20, 50, 120, 30, 0x07E0);
                                tft.setTextColor(0xFFFF); tft.setCursor(40, 60); tft.print("DA MUA!");
                                delay(500);
                            } else {
                                playToneGame(200, 300);
                                tft.fillRect(20, 50, 120, 30, 0xF800);
                                tft.setTextColor(0xFFFF); tft.setCursor(30, 60); tft.print("KHONG DU TIEN");
                                delay(800);
                            }
                        }
                    }
                    break; 
                }
            }
            delay(10); 
        }
    }
}

#endif