#include <stdio.h>
#include "pico/stdlib.h"
#include "display.h"
#include "iostream"
#include "string"

// Change these to the actual Proton pins you are using
static constexpr uint PIN_SPI_SCK  = 18;
static constexpr uint PIN_SPI_MOSI = 19;
static constexpr uint PIN_SPI_MISO = 16;
static constexpr uint PIN_TFT_CS   = 17;
static constexpr uint PIN_TFT_DC   = 20;
static constexpr uint PIN_TFT_RST  = 21;

ILI9341 tft(spi0, PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST, PIN_SPI_SCK, PIN_SPI_MOSI, PIN_SPI_MISO);
int main() {
    stdio_init_all();
    sleep_ms(1500);
    int money = 300;
    tft.init();
    tft.setRotation(3);
    //startscreen();
    //winning(money);
    //losing(money);
};

int winning(int money){
    std::string win = std::to_string(money);
    while (true) {
    static bool toggle = false;
    tft.fillScreen(toggle ? ILI9341::GREEN : ILI9341::BLACK);
    tft.drawText(80, 80, "YOU WIN!!", toggle ? ILI9341::BLACK : ILI9341::GREEN, 3);
    tft.drawText(75, 80, "+$" ,toggle ? ILI9341::BLACK : ILI9341::GREEN, 3);
    tft.drawText(80, 80, win.c_str(), toggle ? ILI9341::BLACK : ILI9341::GREEN, 3);
    toggle = !toggle;
    sleep_ms(1000);
    }
};

int losing(int money){
    std::string lose = std::to_string(money);
    while (true) {
    static bool toggle = false;
    tft.fillScreen(toggle ? ILI9341::GREEN : ILI9341::BLACK);
    tft.drawText(80, 80, "YOU LOSE :(", toggle ? ILI9341::BLACK : ILI9341::RED, 3);
    tft.drawText(75, 80, "-$" ,toggle ? ILI9341::BLACK : ILI9341::RED, 3);
    tft.drawText(80, 80, lose.c_str(), toggle ? ILI9341::BLACK : ILI9341::RED, 3);
    toggle = !toggle;
    sleep_ms(1000);
    }
};

int startscreen(){
    sleep_ms(2000);
    tft.fillScreen(ILI9341::BLACK);
    sleep_ms(1000);
    tft.drawText(80, 80, "WELCOME TO: BLACKJACK", ILI9341::WHITE, 4);
    sleep_ms(500);
    tft.drawText(80, 100, "BROUGHT TO YOU BY:", ILI9341::WHITE, 2);
    sleep_ms(500);
    tft.drawText(80, 120, "THE BELLAGIO", ILI9341::WHITE, 5);
    sleep_ms(2000);
    tft.fillScreen(ILI9341::BLACK);
    sleep_ms(500);
    tft.drawText(80, 80, "WOULD YOU LIKE TO BEGIN?", ILI9341::WHITE, 3);
    sleep_ms(500);
    tft.drawText(60, 100, "YES (PRESS A)", ILI9341::GREEN, 3);
    tft.drawText(100, 100, "NO (PRESS B)", ILI9341:: RED, 1);
    /* if(yes(gpioA)){
    call game screen
    }
    else if (no(gpioB)){
    put up dummy screen
    }

    */
};

int game_screen(/*str hand[] ????? */ int money){
    tft.fillScreen(ILI9341::BLACK);
    sleep_ms(500);
    tft.fillRect(160, 140, 320, 140, ILI9341::GREEN); // Player Area
    tft.fillRect(160, 100, 320, 100, ILI9341::BLACK); // Dealer Area
    // Player Options
    tft.drawText(80, 80, "HIT (PRESS #)", ILI9341:: WHITE, 2);
    tft.drawText(80, 80, "STAND (PRESS #)", ILI9341:: WHITE, 2);
    tft.drawText(80, 80, "SPLIT (PRESS #)", ILI9341:: WHITE, 2);
    tft.drawText(80, 80, "DOUBLE DOWN (PRESS #)", ILI9341:: WHITE, 2);
    std::string curr_avail = std::to_string(money);
    // Attempt to draw dealer

    //Show Cards
    tft.drawText(80, 80, "(  )", ILI9341::WHITE, 2); // need to make dupicates for each dealer and player to have maybe 6 cards? need to check sizing
    tft.drawText(80, 80, "$" ,ILI9341::GREEN, 3); // $ for currency 
    tft.drawText(80, 80, curr_avail.c_str(), ILI9341::GREEN, 3); // actual $ value
    //Double Down
        //Get rid of extra boxes bc you cant play after double down
    //Split
        // Maybe shrink existing view of players cards to make room for 2nd hand
};
//Text
    //tft.setTextColor()
    
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