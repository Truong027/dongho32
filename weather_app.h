#ifndef WEATHER_APP_H
#define WEATHER_APP_H

#include "config.h"
#include <Fonts/FreeSansBold9pt7b.h> 

// --- BẢNG MÀU ---
#define W_BG           0x0000 
#define W_CARD_BG      0x10A2 
#define W_ICON_SUN     0xFFE0 
#define W_ICON_MOON    0xFFE0 // Màu trăng giống màu nắng
#define W_ICON_CLOUD   0xBDF7 
#define W_ICON_RAIN    0x07FF 
#define W_ICON_FOG     0x8410 // Màu xám cho sương mù
#define W_TEXT_LBL     0x9CD3 
#define W_TEXT_VAL     0xFFFF 
#define W_TEXT_UNIT    0xFA60 

// Màu biểu đồ
#define W_GRAPH_BG     0x0842 
#define W_GRID_COLOR   0x2124 
#define W_MAX_COLOR    0xFA60 // Cam (Max Temp)
#define W_MIN_COLOR    0x07FF // Xanh (Min Temp)

// Màu riêng cho 2 biểu đồ
#define COLOR_OUTDOOR  0x07FF  // Cyan (Ngoài trời)
#define COLOR_INDOOR   0xFCA0  // Cam (Trong nhà)

// Dữ liệu biểu đồ (Lưu riêng 2 mảng)
int8_t tempHistoryOut[10] = {0};
int8_t tempHistoryIn[10]  = {0};

// Hàm đẩy dữ liệu
void pushGraphData(int t_out, int t_in) {
    if (t_out < -50 || t_out > 80) t_out = tempHistoryOut[8]; 
    if (t_in < -10 || t_in > 80) t_in = tempHistoryIn[8];

    for (int i = 0; i < 9; i++) {
        tempHistoryOut[i] = tempHistoryOut[i+1];
        tempHistoryIn[i]  = tempHistoryIn[i+1];
    }
    tempHistoryOut[9] = (int8_t)t_out;
    tempHistoryIn[9]  = (int8_t)t_in;
}

// Khởi tạo
void initGraphData(int t_out, int t_in) {
    for (int i = 0; i < 10; i++) {
        tempHistoryOut[i] = (int8_t)t_out;
        tempHistoryIn[i]  = (int8_t)t_in;
    }
}

// [CẬP NHẬT] Vẽ Icon thời tiết chi tiết hơn
void drawWeatherIcon(int x, int y, String desc) {
    desc.toLowerCase();
    tft.setFont(NULL);
    
    // 1. MƯA (Rain / Drizzle)
    if (desc.indexOf("rain") >= 0 || desc.indexOf("drizzle") >= 0) {
        tft.fillCircle(x+15, y+15, 10, W_ICON_CLOUD);
        tft.fillCircle(x+25, y+15, 12, W_ICON_CLOUD);
        tft.fillCircle(x+20, y+12, 10, W_ICON_CLOUD);
        // Hạt mưa
        tft.drawLine(x+18, y+30, x+16, y+36, W_ICON_RAIN);
        tft.drawLine(x+25, y+30, x+23, y+36, W_ICON_RAIN);
        tft.drawLine(x+32, y+30, x+30, y+36, W_ICON_RAIN);
    } 
    // 2. SƯƠNG MÙ (Fog / Mist / Haze)
    else if (desc.indexOf("fog") >= 0 || desc.indexOf("mist") >= 0 || desc.indexOf("haze") >= 0) {
        // Vẽ 3 đường ngang mờ ảo
        for(int i=0; i<3; i++) {
            tft.fillRoundRect(x+10 + (i*5), y+15 + (i*8), 30, 4, 2, W_ICON_FOG);
        }
        // Thêm mặt trời mờ phía sau nếu muốn
        tft.fillCircle(x+35, y+10, 6, 0xCE59); // Màu vàng xám
    }
    // 3. NHIỀU MÂY (Clouds)
    else if (desc.indexOf("cloud") >= 0) {
        // Đám mây to
        tft.fillCircle(x+30, y+18, 10, W_ICON_CLOUD); 
        tft.fillCircle(x+15, y+22, 9, W_ICON_CLOUD); 
        tft.fillCircle(x+22, y+15, 11, W_ICON_CLOUD);
        // Nếu là "broken clouds" hoặc "scattered", thêm mặt trời ló dạng
        if (desc.indexOf("broken") >= 0 || desc.indexOf("scattered") >= 0) {
             tft.fillCircle(x+10, y+10, 6, W_ICON_SUN);
        }
    } 
    // 4. DÔNG BÃO (Thunderstorm)
    else if (desc.indexOf("thunder") >= 0) {
        tft.fillCircle(x+15, y+15, 10, 0x528A); // Mây đen
        tft.fillCircle(x+25, y+15, 12, 0x528A);
        // Tia sét vàng
        tft.fillTriangle(x+20, y+28, x+15, y+35, x+22, y+35, W_ICON_SUN);
        tft.drawLine(x+22, y+35, x+18, y+42, W_ICON_SUN);
    } 
    // 5. TUYẾT (Snow)
    else if (desc.indexOf("snow") >= 0) {
        tft.fillCircle(x+15, y+15, 10, W_ICON_CLOUD);
        tft.fillCircle(x+25, y+15, 12, W_ICON_CLOUD);
        // Bông tuyết trắng
        tft.drawPixel(x+18, y+32, 0xFFFF);
        tft.drawPixel(x+25, y+35, 0xFFFF);
        tft.drawPixel(x+32, y+32, 0xFFFF);
    }
    // 6. TRỜI QUANG (Clear / Sun) - Mặc định
    else {
        // Kiểm tra ban ngày hay ban đêm để vẽ Mặt Trời hoặc Mặt Trăng
        // Ở đây đơn giản vẽ mặt trời
        int h = hour();
        if (h >= 6 && h <= 18) { // Ban ngày: Mặt trời
            tft.fillCircle(x+22, y+22, 9, W_ICON_SUN);
            // Tia nắng
            for (int i=0; i<8; i++) {
                tft.drawPixel(x+22+15*cos(i*0.785), y+22+15*sin(i*0.785), W_ICON_SUN);
            }
        } else { // Ban đêm: Mặt trăng khuyết
            tft.fillCircle(x+22, y+22, 9, W_ICON_MOON);
            tft.fillCircle(x+18, y+18, 9, W_CARD_BG); // Che bớt để tạo hình lưỡi liềm (dùng màu nền card)
        }
    }
}

// Hàm vẽ text an toàn
void drawSafeTemp(int x, int y, float temp) {
    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(W_TEXT_VAL);
    tft.setCursor(x, y); 
    if (isnan(temp) || temp < -50 || temp > 99) tft.print("--");
    else tft.print((int)temp);
}

void drawSafeHum(int hum) {
    tft.setTextColor(0x07E0);
    if (hum < 0 || hum > 100) tft.print("--%");
    else { tft.print(hum); tft.print("%"); }
}

// --- BIỂU ĐỒ CON ---
void drawMiniGraph(int x, int y, int w, int h, int8_t* data, uint16_t color, String label) {
    // 1. Vẽ khung nền
    tft.fillRoundRect(x, y, w, h, 4, W_GRAPH_BG);
    tft.drawRoundRect(x, y, w, h, 4, W_GRID_COLOR);

    // 2. Tìm Min/Max
    int minT = 100, maxT = -100;
    for(int i=0; i<10; i++) {
        if (data[i] < minT) minT = data[i];
        if (data[i] > maxT) maxT = data[i];
    }
    if (maxT <= minT) { maxT = minT + 1; minT = minT - 1; }

    // 3. Phân chia khu vực vẽ (Layout)
    int xGraphStart = x + 30; 
    int xGraphEnd = x + w - 26;
    int graphWidth = xGraphEnd - xGraphStart;
    
    // 4. Vẽ Text thông tin
    tft.setFont(NULL);
    
    // Label bên trái (Out/In)
    tft.setTextColor(color); 
    tft.setCursor(x + 5, y + (h/2) - 3); 
    tft.print(label);

    // Max Temp
    tft.setTextColor(W_MAX_COLOR); 
    tft.setCursor(x + w - 22, y + 4); 
    tft.print(maxT);

    // Min Temp
    tft.setTextColor(W_MIN_COLOR); 
    tft.setCursor(x + w - 22, y + h - 10); 
    tft.print(minT);

    // 5. Vẽ đường biểu đồ
    int step = graphWidth / 9; 
    int bottomY = y + h - 5;
    int topY = y + 5; 

    tft.drawFastHLine(xGraphStart, y + h/2, graphWidth, W_GRID_COLOR);

    for (int i = 0; i < 9; i++) {
        int y1 = map(data[i], minT, maxT, bottomY, topY);
        int y2 = map(data[i+1], minT, maxT, bottomY, topY);
        
        int x1 = xGraphStart + (i * step);
        int x2 = xGraphStart + ((i + 1) * step);
        
        tft.drawLine(x1, y1, x2, y2, color);
        tft.drawLine(x1, y1+1, x2, y2+1, color);
    }

    int xEnd = xGraphStart + 9 * step;
    int yEnd = map(data[9], minT, maxT, bottomY, topY);
    tft.fillCircle(xEnd, yEnd, 2, W_TEXT_VAL);
}

// --- HÀM CẬP NHẬT CHỈ SỐ LIỆU ---
void updateWeatherReadings(float t_out, int h_out, float t_in, int h_in) {
    tft.setTextWrap(false);
    int yBase = 22; 
    int xText = 48;

    // Xóa và vẽ lại
    tft.fillRoundRect(xText + 25, yBase + 5, 80, 20, 0, W_CARD_BG); 
    tft.fillRoundRect(xText + 25, yBase + 28, 80, 20, 0, W_CARD_BG); 

    drawSafeTemp(xText + 28, yBase + 19, t_out);
    tft.setFont(NULL); tft.setTextColor(W_TEXT_UNIT);
    tft.setCursor(xText + 58, yBase + 10); tft.print("o");
    tft.setCursor(xText + 75, yBase + 10); drawSafeHum(h_out);

    drawSafeTemp(xText + 28, yBase + 41, t_in);
    tft.setFont(NULL); tft.setTextColor(W_TEXT_UNIT);
    tft.setCursor(xText + 58, yBase + 30); tft.print("o");
    tft.setCursor(xText + 75, yBase + 30); drawSafeHum(h_in);
}

// Vẽ chỉ số AQI
void drawAQI(int aqi) {
    uint16_t color = 0x07E0; // Tốt (Xanh)
    String label = "TOT";
    if (aqi == 2) { color = 0xFFE0; label = "KHA"; } // Vàng
    else if (aqi == 3) { color = 0xFA60; label = "VUA"; } // Cam
    else if (aqi >= 4) { color = 0xF800; label = "KEM"; } // Đỏ
    
    int xBox = 112;
    int yBox = 74;
    int wBox = 46;
    int hBox = 52;
    
    tft.fillRoundRect(xBox, yBox, wBox, hBox, 4, W_CARD_BG);
    tft.drawRoundRect(xBox, yBox, wBox, hBox, 4, W_GRID_COLOR);
    tft.fillRect(xBox + 38, yBox + 4, 4, hBox - 8, color);

    tft.setFont(NULL);
    tft.setTextColor(W_TEXT_LBL);
    tft.setCursor(xBox + 4, yBox + 8);
    tft.print("AQI");
    
    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(color);
    tft.setCursor(xBox + 6, yBox + 32);
    tft.print(aqi);

    tft.setFont(NULL);
    tft.setTextSize(1);
    tft.setTextColor(W_TEXT_VAL);
    tft.setCursor(xBox + 4, yBox + 38);
    tft.print(label);
}

// --- MAIN DRAW FUNCTION ---
void drawWeatherScreen(float t_out, int h_out, float t_in, int h_in, String desc, int aqi) {
    tft.setTextWrap(false);
    int yBase = 22; 
    
    // 1. INFO CARD
    tft.fillRoundRect(2, yBase, 156, 50, 4, W_CARD_BG);
    drawWeatherIcon(6, yBase + 5, desc);
    
    int xText = 48; 
    tft.setFont(NULL); tft.setTextColor(W_TEXT_LBL); 
    tft.setCursor(xText, yBase + 10); tft.print("OUT:");
    tft.setCursor(xText, yBase + 30); tft.print("IN :");

    updateWeatherReadings(t_out, h_out, t_in, h_in);
    drawAQI(aqi);

    // 2. DUAL CHART CARD 
    int gX = 2; 
    int gW = 108; 
    int gH = 25; 
    drawMiniGraph(gX, 74, gW, gH, tempHistoryOut, COLOR_OUTDOOR, "Out");
    drawMiniGraph(gX, 101, gW, gH, tempHistoryIn, COLOR_INDOOR, "In");
}

#endif