#ifndef BRAIN_QUIZ_APP_H
#define BRAIN_QUIZ_APP_H

#include "config.h"
#include <vector>

// --- CẤU HÌNH ---
#define Q_BG_COLOR      0x0000
#define Q_TEXT_COLOR    0xFFFF
#define Q_ACCENT_COLOR  0x07E0
#define Q_WARN_COLOR    0xF800

// --- KHAI BÁO CÁC HÀM EXTERNAL (Lấy từ creative_apps.h hoặc app_functions.h) ---
// Để code biên dịch được, các hàm này phải tồn tại ở file chính
extern bool isPressed(int pin);
extern void waitForRelease();
extern void playToneGame(int freq, int duration);
extern void updateChips(int delta);

struct Question {
    String q;
    String a; // Đáp án A (Nút UP)
    String b; // Đáp án B (Nút DOWN)
    int correct; // 0: A, 1: B
};

// --- NGÂN HÀNG CÂU HỎI KHỔNG LỒ (50 CÂU) ---
const std::vector<Question> QUIZ_DATA = {
    {"Con gi an lua?", "Khang long", "Vit", 1},
    {"1 kg bong & 1 kg sat?", "Sat nang", "Bang nhau", 1},
    {"Cai gi cang to cang nho?", "Con cua", "Cai tui", 0},
    {"Vua goi la gi?", "Hoang thuong", "Ong noi", 0},
    {"Ben trai duong la gi?", "Ben phai", "Le duong", 1},
    {"Con gi dap thi song?", "Con tim", "Con muoi", 0},
    {"Ai dep trai nhat?", "Dev", "Ban", 0}, 
    {"Cai gi co rang khong mom?", "Luoc", "Cua", 0},
    {"Bien nao khong co nuoc?", "Bien bao", "Ban do", 1},
    {"Con gi co thit khong xuong?", "Con dia", "Con ca", 0},
    {"Quan gi rong nhat?", "Quan dao", "Quan dui", 0},
    {"Lich nao dai nhat?", "Lich su", "Lich van nien", 0},
    {"Xa nao dong nhat?", "Xa hoi", "Xa xoi", 0},
    {"Con duong dai nhat?", "Duong doi", "QL 1A", 0},
    {"Cai gi cua ban nhung nguoi khac dung?", "Ten", "Tien", 0},
    {"So 0 La Ma la gi?", "Khong co", "Zero", 0},
    {"Thang 2 co ngay 30?", "Khong", "Co (Nam nhuan)", 0},
    {"Con gi mang duoc go nhung khong mang duoc soi?", "Con song", "Con kien", 0},
    {"Co 3 qua tao, lay 2, con may?", "2 (Tren tay)", "1 (Tren ban)", 0},
    {"Bo gi khong uong sua?", "Bo song", "Bo sua", 0},
    {"Cai gi di thi nam, dung thi dung?", "Ban chan", "Con ran", 0},
    {"Nha Nam co 4 anh em: Xuan, Ha, Thu...?", "Dong", "Nam", 1},
    {"Cai gi khong canh ma bay?", "Thoi gian", "May bay", 0},
    {"Cai gi cang keo cang ngan?", "Dieu thuoc", "Soi day", 0},
    {"Cai gi den khi ban?", "Bang den", "Than", 0},
    {"An gi khong no?", "An anh", "An com", 0},
    {"Bo gi biet di xe dap?", "Bo hong", "Bo cau", 0}, 
    {"Cay gi khong co la?", "Cay cot dien", "Cay xuong rong", 0},
    {"Con gi sinh ra da la ong cu?", "Ong gia noel", "Con nguoi", 0}, 
    {"Hai con vit di truoc hai con vit...?", "4 con", "3 con", 0}, 
    {"Lam sao de nhet voi vao tu lanh?", "Mo cua, nhet", "Chat nho", 0},
    {"Con kien bo len tai con voi noi gi?", "Em co thai", "Chao anh", 0},
    {"Tai sao con voi chet?", "Soc tim", "Gia", 0},
    {"Mon gi cang thang cang thua?", "Dua xe", "Keo co", 0},
    {"Cai gi quy nhat?", "Ngoc nguyen", "Tu do", 1},
    {"Nuoc nao hinh chiec ung?", "Y", "Viet Nam", 0},
    {"Ai la cha de cua iPhone?", "Steve Jobs", "Bill Gates", 0},
    {"Thu do cua My?", "New York", "Washington DC", 1},
    {"Dinh nui cao nhat?", "Everest", "Phansipan", 0},
    {"Song dai nhat?", "Song Nile", "Song Hong", 0},
    {"Hanh tinh do?", "Sao Hoa", "Sao Kim", 0},
    {"Con vat nhanh nhat?", "Bao dom", "Su tu", 0},
    {"Kim loai nao long o nhiet do phong?", "Thuy ngan", "Vang", 0},
    {"Ai phat minh ra bong den?", "Edison", "Tesla", 0},
    {"Nam nhuan co bao nhieu ngay?", "366", "365", 0},
    {"1 the ky bang bao nhieu nam?", "100", "1000", 0},
    {"Ai viet Truyen Kieu?", "Nguyen Du", "Nguyen Trai", 0},
    {"H2O la gi?", "Nuoc", "Khi", 0},
    {"Mat troi moc huong nao?", "Dong", "Tay", 0},
    {"Trai dat hinh gi?", "Cau", "Det", 0}
};

// --- LOGIC GAME CHÍNH ---
void runBrainQuizBig() {
    int score = 0;
    int lives = 3;
    int streak = 0; // Chuỗi trả lời đúng liên tiếp
    int timeLimit = 10000; // Thời gian trả lời cơ bản (ms)
    
    // Tạo danh sách index để xáo trộn câu hỏi
    std::vector<int> qIndices;
    for(int i=0; i<QUIZ_DATA.size(); i++) qIndices.push_back(i);
    
    // Xáo trộn câu hỏi (Shuffle)
    for(int i=0; i<qIndices.size(); i++) {
        int r = random(i, qIndices.size());
        int temp = qIndices[i];
        qIndices[i] = qIndices[r];
        qIndices[r] = temp;
    }
    
    int currentQIndex = 0;
    waitForRelease();
    
    while(lives > 0) {
        // Hết câu hỏi thì reset lại từ đầu (để chơi vô tận)
        if (currentQIndex >= qIndices.size()) currentQIndex = 0;
        
        Question currentQ = QUIZ_DATA[qIndices[currentQIndex]];
        currentQIndex++;
        
        tft.fillScreen(Q_BG_COLOR);
        
        // --- Header Info ---
        tft.fillRect(0, 0, 160, 24, 0x2124);
        tft.setTextColor(Q_TEXT_COLOR); 
        tft.setCursor(5, 6); 
        tft.print("Diem: " + String(score));
        
        // Vẽ tim (mạng sống)
        for(int i=0; i<lives; i++) {
            tft.fillCircle(150 - (i*12), 12, 4, Q_WARN_COLOR);
        }
        
        // --- Hiển thị câu hỏi (Word Wrap xịn hơn) ---
        tft.setCursor(5, 35); tft.setTextColor(Q_TEXT_COLOR); tft.setTextSize(1);
        String qText = currentQ.q;
        int lineLen = 22; // Số ký tự ước tính trên 1 dòng
        int cursorY = 35;
        
        while (qText.length() > 0) {
            if (qText.length() <= lineLen) {
                tft.setCursor(5, cursorY); tft.print(qText);
                break;
            }
            int splitIdx = qText.lastIndexOf(' ', lineLen);
            if (splitIdx == -1) splitIdx = lineLen;
            tft.setCursor(5, cursorY); tft.print(qText.substring(0, splitIdx));
            qText = qText.substring(splitIdx + 1);
            cursorY += 12;
        }
        
        // --- Hiển thị đáp án (Box phong cách game show) ---
        int boxY_A = 75;
        int boxY_B = 100;
        
        tft.fillRoundRect(5, boxY_A, 150, 20, 4, 0x07FF); // Box A - Xanh Cyan
        tft.setTextColor(0x0000); tft.setCursor(10, boxY_A + 6); tft.print("UP: " + currentQ.a);
        
        tft.fillRoundRect(5, boxY_B, 150, 20, 4, 0xFA60); // Box B - Cam
        tft.setTextColor(0x0000); tft.setCursor(10, boxY_B + 6); tft.print("DN: " + currentQ.b);
        
        // --- Vòng lặp thời gian trả lời ---
        unsigned long qStart = millis();
        bool answered = false;
        
        // Giảm thời gian theo điểm số (càng chơi càng nhanh)
        int currentTimeLimit = timeLimit - (score * 100); 
        if (currentTimeLimit < 4000) currentTimeLimit = 4000; // Tối thiểu 4s
        
        while(millis() - qStart < currentTimeLimit) {
            // Thanh thời gian (Giảm dần)
            float percent = 1.0 - (float)(millis() - qStart) / currentTimeLimit;
            int barW = percent * 160;
            
            // Màu thanh thời gian: Xanh -> Vàng -> Đỏ
            uint16_t barColor = Q_ACCENT_COLOR;
            if (percent < 0.6) barColor = 0xFD20;
            if (percent < 0.3) barColor = Q_WARN_COLOR;
            
            tft.fillRect(0, 24, 160, 4, 0x0000); // Xóa nền thanh cũ
            tft.fillRect(0, 24, barW, 4, barColor);
            
            if (isPressed(TOUCH_MENU)) { return; } // Thoát game
            
            int choice = -1;
            if (isPressed(TOUCH_UP)) choice = 0;
            if (isPressed(TOUCH_DOWN)) choice = 1;
            
            if (choice != -1) {
                answered = true;
                waitForRelease();
                
                // Hiệu ứng chọn (Viền trắng)
                int selectedY = (choice == 0) ? boxY_A : boxY_B;
                tft.drawRect(4, selectedY-1, 152, 22, 0xFFFF); 
                delay(200);
                
                if (choice == currentQ.correct) {
                    // --- TRẢ LỜI ĐÚNG ---
                    playToneGame(2000, 100); delay(50); playToneGame(2500, 150);
                    tft.fillRoundRect(5, selectedY, 150, 20, 4, 0x07E0); // Xanh lá đậm
                    tft.setTextColor(0xFFFF); tft.setCursor(60, selectedY + 6); tft.print("DUNG!");
                    
                    score++;
                    streak++;
                    
                    // Thưởng Streak (Combo)
                    if (streak >= 3) {
                        tft.setCursor(100, 6); tft.setTextColor(0xFD20); tft.print("x" + String(streak));
                    }
                    
                    delay(800);
                } else {
                    // --- TRẢ LỜI SAI ---
                    playToneGame(200, 400);
                    tft.fillRoundRect(5, selectedY, 150, 20, 4, 0xF800); // Đỏ
                    tft.setTextColor(0xFFFF); tft.setCursor(55, selectedY + 6); tft.print("SAI ROI");
                    
                    // Hiện đáp án đúng để người chơi biết
                    int correctY = (currentQ.correct == 0) ? boxY_A : boxY_B;
                    tft.drawRect(4, correctY-1, 152, 22, 0x07E0); // Viền xanh cho đáp án đúng
                    
                    lives--;
                    streak = 0;
                    delay(1500); // Dừng lâu hơn để xem đáp án
                }
                break;
            }
            delay(10);
        }
        
        if (!answered) {
            playToneGame(200, 400);
            tft.fillRect(30, 60, 100, 30, Q_WARN_COLOR);
            tft.setTextColor(0xFFFF); tft.setCursor(45, 70); tft.print("HET GIO!");
            lives--;
            streak = 0;
            delay(1000);
        }
    }
    
    // --- MÀN HÌNH KẾT THÚC ---
    tft.fillScreen(0x0000);
    
    // Hiệu ứng pháo hoa (đơn giản) nếu điểm cao
    if (score > 10) {
        for(int i=0; i<20; i++) {
            int x = random(10, 150); int y = random(10, 100);
            uint16_t c = random(0, 0xFFFF);
            tft.fillCircle(x, y, 2, c);
            delay(20);
        }
    }
    
    tft.setTextColor(0xFD20); tft.setTextSize(2); 
    tft.setCursor(30, 30); tft.print("KET THUC");
    
    tft.setTextColor(0xFFFF); tft.setTextSize(1); 
    tft.setCursor(40, 60); tft.print("Diem so: " + String(score));
    
    // Tính thưởng: (Điểm * 50) + (Điểm cao * 20 bonus)
    int bonus = score * 50;
    if (score > 10) bonus += (score - 10) * 50; // Thưởng thêm nếu > 10 câu
    
    if (bonus > 0) {
        updateChips(bonus);
        tft.setTextColor(0x07E0); tft.setCursor(30, 80); tft.print("+" + String(bonus) + " Chips");
        playToneGame(1000, 100); playToneGame(1500, 100); playToneGame(2000, 200);
    } else {
        tft.setTextColor(0xAAAA); tft.setCursor(30, 80); tft.print("Co gang lan sau!");
    }
    
    tft.setCursor(35, 110); tft.setTextColor(0xFFFF); tft.print("BAM DE THOAT");
    
    waitForRelease();
    while(!isPressed(TOUCH_MENU) && !isPressed(TOUCH_UP));
}

#endif