#include <stdio.h>
#include "pico/stdlib.h"
#include "display.h"
#include <string>
#include <vector>

// TFT pins
static constexpr uint PIN_SPI_SCK  = 18;
static constexpr uint PIN_SPI_MOSI = 19;
static constexpr uint PIN_SPI_MISO = 16;
static constexpr uint PIN_TFT_CS   = 17;
static constexpr uint PIN_TFT_DC   = 20;
static constexpr uint PIN_TFT_RST  = 21;

ILI9341 tft(spi0, PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST,
            PIN_SPI_SCK, PIN_SPI_MOSI, PIN_SPI_MISO);

// Forward declarations
int startscreen(int selectedIndex);
int bet_screen(int currentMoney, int currentBet);
int push_screen();
int winning(int money);
int losing(int money);

extern "C" void Screen_Init(void) {
    tft.init();
    tft.setRotation(3);
}

int game_screen(const std::vector<std::string>& dealerHand,
                const std::vector<std::string>& playerHand,
                const std::vector<std::string>& splitHand1,
                const std::vector<std::string>& splitHand2,
                int money,
                bool canSplit,
                bool canDoubleDown,
                bool doubleDownActive,
                bool splitActive,
                bool leftHandActive,
                int selectedIndex);

void demo_all_screens();

static void clearOptionsArea();
static void drawCardRow(int y, const std::vector<std::string>& cards, uint16_t color, uint8_t scale);
static void drawCardRowCustomX(const std::vector<int>& xPositions, int y,
                               const std::vector<std::string>& cards,
                               uint16_t color, uint8_t scale);
static void drawMoneyDisplay(int money);
static void drawGameCursor(int selectedIndex, bool canSplit, bool canDoubleDown);
static void drawNormalOptions(int selectedIndex, bool canSplit, bool canDoubleDown);
static void drawDoubleDownBanner();
static void drawSplitBanner(bool leftHandActive);
static void drawStartCursor(int selectedIndex);

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
    tft.fillRect(225, 96, 95, 20, ILI9341::BLACK);
    tft.drawText(230, 100, "$", ILI9341::GREEN, 2);
    tft.drawText(240, 100, curr_avail.c_str(), ILI9341::GREEN, 2);
}

static void drawGameCursor(int selectedIndex, bool canSplit, bool canDoubleDown) {
    clearOptionsArea();

    uint16_t hitColor    = (selectedIndex == 0) ? ILI9341::YELLOW : ILI9341::WHITE;
    uint16_t standColor  = (selectedIndex == 1) ? ILI9341::YELLOW : ILI9341::WHITE;
    uint16_t splitColor  = (selectedIndex == 2) ? ILI9341::YELLOW : ILI9341::WHITE;
    uint16_t doubleColor = (selectedIndex == 3) ? ILI9341::YELLOW : ILI9341::WHITE;

    tft.drawText(5, 100, "HIT", hitColor, 1);
    tft.drawText(80, 100, "STAND", standColor, 1);

    if (canSplit) {
        tft.drawText(5, 110, "SPLIT", splitColor, 1);
    }

    if (canDoubleDown) {
        tft.drawText(115, 110, "DOUBLE", doubleColor, 1);
    }

    // Marker uses color only, no special symbols needed
    if (selectedIndex == 0) {
        tft.fillRect(0, 100, 3, 8, ILI9341::YELLOW);
    } else if (selectedIndex == 1) {
        tft.fillRect(75, 100, 3, 8, ILI9341::YELLOW);
    } else if (selectedIndex == 2 && canSplit) {
        tft.fillRect(0, 110, 3, 8, ILI9341::YELLOW);
    } else if (selectedIndex == 3 && canDoubleDown) {
        tft.fillRect(110, 110, 3, 8, ILI9341::YELLOW);
    }
}

static void drawNormalOptions(int selectedIndex, bool canSplit, bool canDoubleDown) {
    drawGameCursor(selectedIndex, canSplit, canDoubleDown);
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
        tft.drawText(5, 110, "LEFT HAND", ILI9341::CYAN, 1);
    } else {
        tft.drawText(5, 110, "RIGHT HAND", ILI9341::CYAN, 1);
    }
}

static void drawStartCursor(int selectedIndex) {
    tft.fillRect(35, 90, 160, 130, ILI9341::BLACK);

    uint16_t yesColor = (selectedIndex == 0) ? ILI9341::YELLOW : ILI9341::GREEN;
    uint16_t noColor  = (selectedIndex == 1) ? ILI9341::YELLOW : ILI9341::RED;

    tft.drawText(60, 100, "YES", yesColor, 2);
    tft.drawText(100, 200, "NO", noColor, 1);

    if (selectedIndex == 0) {
        tft.fillRect(45, 100, 5, 16, ILI9341::YELLOW);
    } else {
        tft.fillRect(85, 200, 5, 8, ILI9341::YELLOW);
    }
}

int bet_screen(int currentMoney, int currentBet) {
    tft.fillScreen(ILI9341::BLACK);
    sleep_ms(200);

    tft.drawText(70, 20, "PLACE YOUR BET", ILI9341::WHITE, 2);

    tft.drawText(20, 70, "BANK:", ILI9341::WHITE, 2);
    tft.drawText(95, 70, "$", ILI9341::GREEN, 2);

    std::string moneyStr = std::to_string(currentMoney);
    tft.drawText(110, 70, moneyStr.c_str(), ILI9341::GREEN, 2);

    tft.drawText(20, 120, "BET:", ILI9341::WHITE, 2);
    tft.drawText(85, 120, "$", ILI9341::YELLOW, 3);

    std::string betStr = std::to_string(currentBet);
    tft.drawText(110, 120, betStr.c_str(), ILI9341::YELLOW, 3);

    tft.drawText(20, 180, "UP: INCREASE", ILI9341::WHITE, 1);
    tft.drawText(20, 195, "DOWN: DECREASE", ILI9341::WHITE, 1);
    tft.drawText(20, 210, "CLICK: CONFIRM", ILI9341::WHITE, 1);

    return 0;
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

int push_screen() {
    bool toggle = false;
    uint32_t start = to_ms_since_boot(get_absolute_time());

    while (to_ms_since_boot(get_absolute_time()) - start < 3000) {
        tft.fillScreen(toggle ? ILI9341::BLUE : ILI9341::BLACK);
        tft.drawText(95, 90, "PUSH", toggle ? ILI9341::BLACK : ILI9341::BLUE, 4);
        tft.drawText(45, 140, "BET RETURNED", toggle ? ILI9341::BLACK : ILI9341::BLUE, 2);

        toggle = !toggle;
        sleep_ms(1000);
    }

    return 0;
}

int startscreen(int selectedIndex) {
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

    drawStartCursor(selectedIndex);

    return 0;
}

int game_screen(const std::vector<std::string>& dealerHand,
                const std::vector<std::string>& playerHand,
                const std::vector<std::string>& splitHand1,
                const std::vector<std::string>& splitHand2,
                int money,
                bool canSplit,
                bool canDoubleDown,
                bool doubleDownActive,
                bool splitActive,
                bool leftHandActive,
                int selectedIndex) {
    tft.fillScreen(ILI9341::BLACK);
    sleep_ms(200);

    // top = dealer, bottom = player
    tft.fillRect(0,   0, 320, 120, ILI9341::GREEN);
    tft.fillRect(0, 120, 320, 120, ILI9341::BLACK);

    // labels
    tft.drawText(5, 5, "DEALER", ILI9341::BLACK, 1);
    tft.drawText(5, 210, "PLAYER", ILI9341::WHITE, 1);

    drawMoneyDisplay(money);

    // dealer cards on top
    drawCardRow(30, dealerHand, ILI9341::WHITE, 3);

    if (!splitActive) {
        // player cards on bottom
        drawCardRow(140, playerHand, ILI9341::WHITE, 3);

        if (doubleDownActive) {
            drawDoubleDownBanner();
        } else {
            drawNormalOptions(selectedIndex, canSplit, canDoubleDown);
        }
    } else {
        tft.drawText(5, 135, "HAND 1", ILI9341::WHITE, 1);
        tft.drawText(170, 135, "HAND 2", ILI9341::WHITE, 1);

        drawCardRowCustomX({5, 45, 85}, 155, splitHand1, ILI9341::WHITE, 2);
        drawCardRowCustomX({170, 210, 250}, 155, splitHand2, ILI9341::WHITE, 2);

        if (leftHandActive) {
            tft.drawText(5, 185, "ACTIVE", ILI9341::YELLOW, 1);
        } else {
            tft.drawText(170, 185, "ACTIVE", ILI9341::YELLOW, 1);
        }

        drawSplitBanner(leftHandActive);
    }

    return 0;
}

void demo_all_screens() {
    while (true) {
        startscreen(0);
        sleep_ms(2500);

        startscreen(1);
        sleep_ms(2500);

        bet_screen(250, 25);
        sleep_ms(3000);

        game_screen({"A", "?"}, {"10", "8"}, {}, {}, 250,
                    true, true, false, false, true, 0);
        sleep_ms(3000);

        game_screen({"9", "?"}, {"6", "5", "K"}, {}, {}, 200,
                    false, false, true, false, true, 1);
        sleep_ms(3000);

        game_screen({"K", "?"}, {}, {"8", "3"}, {"8", "Q"}, 180,
                    false, false, false, true, true, 0);
        sleep_ms(3000);

        game_screen({"K", "?"}, {}, {"8", "3"}, {"8", "Q"}, 180,
                    false, false, false, true, false, 0);
        sleep_ms(3000);

        winning(50);
        losing(25);
        push_screen();
    }
}