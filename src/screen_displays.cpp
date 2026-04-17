#include <stdio.h>
#include "pico/stdlib.h"
#include "display.h"
#include "iostream"
#include "string"
#include "vector"

// Change these to the actual Proton pins you are using
static constexpr uint PIN_SPI_SCK  = 18;
static constexpr uint PIN_SPI_MOSI = 19;
static constexpr uint PIN_SPI_MISO = 16;
static constexpr uint PIN_TFT_CS   = 17;
static constexpr uint PIN_TFT_DC   = 20;
static constexpr uint PIN_TFT_RST  = 21;

ILI9341 tft(spi0, PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST, PIN_SPI_SCK, PIN_SPI_MOSI, PIN_SPI_MISO);

// Forward declarations
int startscreen();
int winning(int money);
int losing(int money);
int game_screen(const std::vector<std::string>& dealerHand,
                const std::vector<std::string>& playerHand,
                const std::vector<std::string>& splitHand1,
                const std::vector<std::string>& splitHand2,
                int money,
                bool canSplit,
                bool canDoubleDown,
                bool doubleDownActive,
                bool splitActive,
                bool leftHandActive);

void demo_all_screens();
int main() {
    stdio_init_all();
    sleep_ms(1500);

    tft.init();
    tft.setRotation(3);

    demo_all_screens();

    while (true) {
        sleep_ms(1000);
    }

    return 0;
};

static void clearOptionsArea() {
    tft.fillRect(0, 96, 225, 24, ILI9341::BLACK);
}

static void drawCardRow(int y, const std::vector<std::string>& cards, uint16_t color, uint8_t scale) {
    const int xPositions[4] = {5, 78, 159, 240};

    for (size_t i = 0; i < cards.size() && i < 4; i++) {
        std::string label = "(" + cards[i] + ")";
        tft.drawText(xPositions[i], y, label.c_str(), color, scale);
    }
}
static void drawCardRowCustomX(const std::vector<int>& xPositions, int y,
                               const std::vector<std::string>& cards,
                               uint16_t color, uint8_t scale) {
    for (size_t i = 0; i < cards.size() && i < xPositions.size(); i++) {
        std::string label = "(" + cards[i] + ")";
        tft.drawText(xPositions[i], y, label.c_str(), color, scale);
    }
}
static void drawMoneyDisplay(int money) {
    std::string curr_avail = std::to_string(money);
    tft.fillRect(225, 96, 95, 20, ILI9341::BLACK);   // clear old money area
    tft.drawText(230, 100, "$", ILI9341::GREEN, 2);
    tft.drawText(240, 100, curr_avail.c_str(), ILI9341::GREEN, 2);
}
// static void drawDealerHeader() {
//     tft.drawText(5, 125, "DEALER", ILI9341::BLACK, 1);}
// static void drawPlayerHeader() {
//     tft.drawText(5, 5, "PLAYER", ILI9341::WHITE, 1);}
static void drawNormalOptions(bool canSplit, bool canDoubleDown) {
    clearOptionsArea();
    tft.drawText(5, 100, "HIT (A)", ILI9341::WHITE, 1);
    tft.drawText(80, 100, "STAND (B)", ILI9341::WHITE, 1);

    if (canSplit) {
        tft.drawText(5, 110, "SPLIT (C)", ILI9341::WHITE, 1);
    }

    if (canDoubleDown) {
        tft.drawText(115, 110, "DOUBLE (D)", ILI9341::WHITE, 1);
    }
}
static void drawDoubleDownBanner() {
    clearOptionsArea();
    tft.drawText(5, 100, "DOUBLE DOWN", ILI9341::YELLOW, 1);
    tft.drawText(5, 110, "ONE CARD ONLY", ILI9341::YELLOW, 1);
}
static void drawSplitBanner(bool leftHandActive) {
    clearOptionsArea();
    tft.drawText(5, 100, "SPLIT MODE", ILI9341::CYAN, 1);

    if (leftHandActive) {
        tft.drawText(5, 110, "PLAYING LEFT HAND", ILI9341::CYAN, 1);
    } else {
        tft.drawText(5, 110, "PLAYING RIGHT HAND", ILI9341::CYAN, 1);
    }
}

void demo_all_screens() {
    while (true) {
        // 1. Normal screen
        game_screen({"A", "?"}, {"10", "8"},{},{},250,
        true, true, false, false, true);
        tft.drawText(5, 210, "DEALER", ILI9341::WHITE, 1);
        tft.drawText(5, 80, "PLAYER", ILI9341::WHITE, 1);
        sleep_ms(3000);

        // 2. Double down screen
        game_screen({"9", "?"}, {"6", "5", "K"},{},{},200,
        false, false, true, false, true);
        tft.drawText(5, 210, "DEALER", ILI9341::WHITE, 1);
        tft.drawText(5, 80, "PLAYER", ILI9341::WHITE, 1);
        sleep_ms(3000);

        // 3. Split screen, hand 1 active
        game_screen({"K", "?"},{},{"8", "3"},{"8", "Q"},180,
        false, false, false, true, true);
        tft.drawText(5, 210, "DEALER", ILI9341::WHITE, 1);
        tft.drawText(5, 80, "PLAYER", ILI9341::WHITE, 1);
        sleep_ms(3000);

        // 4. Split screen, hand 2 active
        game_screen({"K", "?"}, {},{"8", "3"},{"8", "Q"},180,
        false, false, false, true, false);
        tft.drawText(5, 210, "DEALER", ILI9341::WHITE, 1);
        tft.drawText(5, 80, "PLAYER", ILI9341::WHITE, 1);
        sleep_ms(3000);

        // 5. Winning screen
        winning(50);

        // 6. Losing screen
        losing(25);
    }
}

int winning(int money) {
    std::string win = std::to_string(money);
    bool toggle = false;

    uint32_t start = to_ms_since_boot(get_absolute_time());

    while (to_ms_since_boot(get_absolute_time()) - start < 3000) {
        tft.fillScreen(toggle ? ILI9341::GREEN : ILI9341::BLACK);
        tft.drawText(80, 80, "YOU WIN!!", toggle ? ILI9341::BLACK : ILI9341::GREEN, 3);
        tft.drawText(100, 120, "+$", toggle ? ILI9341::BLACK : ILI9341::GREEN, 3);
        tft.drawText(140, 120, win.c_str(), toggle ? ILI9341::BLACK : ILI9341::GREEN, 3);

        toggle = !toggle;
        sleep_ms(1000);
    }

    return 0;
}

int losing(int money) {
    std::string lose = std::to_string(money);
    bool toggle = false;

    uint32_t start = to_ms_since_boot(get_absolute_time());

    while (to_ms_since_boot(get_absolute_time()) - start < 3000) {
        tft.fillScreen(toggle ? ILI9341::RED : ILI9341::BLACK);
        tft.drawText(60, 80, "YOU LOSE :(", toggle ? ILI9341::BLACK : ILI9341::RED, 3);
        tft.drawText(100, 120, "-$", toggle ? ILI9341::BLACK : ILI9341::RED, 3);
        tft.drawText(140, 120, lose.c_str(), toggle ? ILI9341::BLACK : ILI9341::RED, 3);

        toggle = !toggle;
        sleep_ms(1000);
    }
    return 0;
}

int startscreen(){
    sleep_ms(2000);
    tft.fillScreen(ILI9341::BLACK);
    sleep_ms(1000);
    tft.drawText(15, 40, "WELCOME TO: BLACKJACK", ILI9341::WHITE, 2);
    sleep_ms(500);
    tft.drawText(70, 80, "BROUGHT TO YOU BY:", ILI9341::WHITE, 1);
    sleep_ms(500);
    tft.drawText(40, 120, "THE BELLAGIO", ILI9341::WHITE, 3);
    sleep_ms(2000);
    tft.fillScreen(ILI9341::BLACK);
    sleep_ms(500);
    tft.drawText(5, 40, "WOULD YOU LIKE TO BEGIN?", ILI9341::WHITE, 2);
    sleep_ms(500);
    tft.drawText(60, 100, "YES", ILI9341::GREEN, 2);
    tft.drawText(100, 200, "NO", ILI9341:: RED, 1);
    /* if(yes(gpioA)){
    call game screen
    }
    else if (no(gpioB)){
    put up dummy screen
    }

    */
   return 0;
};

int game_screen(const std::vector<std::string>& dealerHand,
                const std::vector<std::string>& playerHand,
                const std::vector<std::string>& splitHand1,
                const std::vector<std::string>& splitHand2,
                int money, bool canSplit, bool canDoubleDown,
                bool doubleDownActive,bool splitActive, bool leftHandActive) {
    tft.fillScreen(ILI9341::BLACK);
    sleep_ms(200);

    // Main layout
    tft.fillRect(0,   0, 320, 120, ILI9341::BLACK); // Player area
    tft.fillRect(0, 120, 320, 120, ILI9341::GREEN); // Dealer area

    //drawPlayerHeader();
    //drawDealerHeader();
    drawMoneyDisplay(money);

    // Dealer drawing
    // Dealer always uses normal top row in green area
    drawCardRow(180, dealerHand, ILI9341::WHITE, 3);

    // Player drawing
    if (!splitActive) {
        // Normal one-hand layout
        drawCardRow(30, playerHand, ILI9341::WHITE, 3);

        if (doubleDownActive) {
            drawDoubleDownBanner();
        } else {
            drawNormalOptions(canSplit, canDoubleDown);
        }
    } else {
        // Split layout:
        // use smaller cards so two hands fit
        // left hand on left side, right hand on right side
        tft.drawText(5, 15, "HAND 1", ILI9341::WHITE, 1);
        tft.drawText(170, 15, "HAND 2", ILI9341::WHITE, 1);

        drawCardRowCustomX({5, 45, 85}, 35, splitHand1, ILI9341::WHITE, 2);
        drawCardRowCustomX({170, 210, 250}, 35, splitHand2, ILI9341::WHITE, 2);

        // active hand marker
        if (leftHandActive) {
            tft.drawText(5, 65, "<-- ACTIVE", ILI9341::YELLOW, 1);
        } else {
            tft.drawText(170, 65, "<-- ACTIVE", ILI9341::YELLOW, 1);
        }

        drawSplitBanner(leftHandActive);
    }

    return 0;
}
//Penar
/*    sleep_ms(1500);
    tft.fillRect(250, 80, 60, 60, ILI9341::WHITE);
    tft.fillRect(250, 130, 60, 60, ILI9341::WHITE);
    tft.fillRect(130, 117, 150, 40, ILI9341::WHITE);

    while (true) {
        sleep_ms(1000);
        tft.drawPixel(120, 110, ILI9341::YELLOW);
        sleep_ms(500);
        tft.drawPixel(110, 110, ILI9341::YELLOW);
        sleep_ms(500);
        tft.drawPixel(100, 105, ILI9341::YELLOW);
        sleep_ms(500);
        tft.drawPixel(90, 100, ILI9341::YELLOW);
        sleep_ms(500);
        tft.drawPixel(80, 95, ILI9341::YELLOW);
        sleep_ms(500);
        tft.drawPixel(75, 85, ILI9341::YELLOW);
        sleep_ms(500);
        tft.drawPixel(70, 75, ILI9341::YELLOW);
        sleep_ms(500);
        tft.drawPixel(65, 65, ILI9341::YELLOW);
        sleep_ms(500);
        tft.drawPixel(60, 55, ILI9341::YELLOW);
        sleep_ms(500);
        tft.fillRect(50, 30, 25, 25, ILI9341::YELLOW);
        sleep_ms(1000);
        tft.fillRect(40, 30, 83, 83, ILI9341::BLACK); 
        sleep_ms(1000);
    }
*/
    // while (true) {
    //     static bool toggle = false;
    //     tft.fillRect(20, 100, 280, 40, toggle ? ILI9341::YELLOW : ILI9341::MAGENTA);
    //     toggle = !toggle;
    //     sleep_ms(1000);
    // }