#ifndef ILI9341_H
#define ILI9341_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <string>
#include <vector>

int startscreen(int selectedIndex);
int bet_screen(int currentMoney, int currentBet);
int push_screen(void);
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
                bool leftHandActive,
                int selectedIndex);

#ifdef __cplusplus
extern "C" {
#endif

void Screen_Init(void);

#ifdef __cplusplus
}
#endif

class ILI9341 {
public:
    ILI9341(spi_inst_t* spiPort,
            uint csPin,
            uint dcPin,
            uint rstPin,
            uint sckPin,
            uint mosiPin,
            uint misoPin);

    void init();
    void setRotation(uint8_t rotation);
    void fillScreen(uint16_t color);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale);
    void drawText(uint16_t x, uint16_t y, const char* str, uint16_t color, uint8_t scale);
    void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
    void drawCircle(int x0, int y0, int r, uint16_t color);
    void fillCircle(int x0, int y0, int r, uint16_t color);

    static constexpr uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    static constexpr uint16_t BLACK   = 0x0000;
    static constexpr uint16_t WHITE   = 0xFFFF;
    static constexpr uint16_t RED     = 0xF800;
    static constexpr uint16_t GREEN   = 0x07E0;
    static constexpr uint16_t BLUE    = 0x001F;
    static constexpr uint16_t YELLOW  = 0xFFE0;
    static constexpr uint16_t CYAN    = 0x07FF;
    static constexpr uint16_t MAGENTA = 0xF81F;
    static constexpr uint16_t ORANGE  = 0xFD20;

private:
    spi_inst_t* spi;
    uint cs, dc, rst, sck, mosi, miso;
    uint16_t width = 240;
    uint16_t height = 320;

    void writeCommand(uint8_t cmd);
    void writeData(const uint8_t* data, size_t len);
    void writeDataByte(uint8_t data);
    void hardwareReset();
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void writeColor(uint16_t color, uint32_t count);
};

#endif

