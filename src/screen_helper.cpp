#include "display.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/time.h"
#include "math.h"

ILI9341::ILI9341(spi_inst_t* spiPort,
                 uint csPin,
                 uint dcPin,
                 uint rstPin,
                 uint sckPin,
                 uint mosiPin,
                 uint misoPin)
    : spi(spiPort), cs(csPin), dc(dcPin), rst(rstPin),
      sck(sckPin), mosi(mosiPin), miso(misoPin) {}

void ILI9341::hardwareReset() {
    gpio_put(rst, 1);
    sleep_ms(5);
    gpio_put(rst, 0);
    sleep_ms(20);
    gpio_put(rst, 1);
    sleep_ms(150);
}
const uint8_t digits5x7[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
};
const uint8_t letters5x7[26][5] = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};
static const uint8_t symbols5x7[8][5] = {
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x00, 0x10, 0x10, 0x10, 0x00}  // -
};

void ILI9341::writeCommand(uint8_t cmd) {
    gpio_put(dc, 0);
    gpio_put(cs, 0);
    spi_write_blocking(spi, &cmd, 1);
    gpio_put(cs, 1);
}

void ILI9341::writeData(const uint8_t* data, size_t len) {
    gpio_put(dc, 1);
    gpio_put(cs, 0);
    spi_write_blocking(spi, data, len);
    gpio_put(cs, 1);
}

void ILI9341::writeDataByte(uint8_t data) {
    writeData(&data, 1);
}

void ILI9341::init() {
    spi_init(spi, 40000000); // start fast; lower to 10-20 MHz if unstable

    gpio_set_function(sck, GPIO_FUNC_SPI);
    gpio_set_function(mosi, GPIO_FUNC_SPI);
    if (miso != (uint)-1) {
        gpio_set_function(miso, GPIO_FUNC_SPI);
    }

    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    gpio_put(cs, 1);

    gpio_init(dc);
    gpio_set_dir(dc, GPIO_OUT);
    gpio_put(dc, 1);

    gpio_init(rst);
    gpio_set_dir(rst, GPIO_OUT);
    gpio_put(rst, 1);

    hardwareReset();

    writeCommand(0x01); // SWRESET
    sleep_ms(150);

    writeCommand(0x28); // display off

    writeCommand(0xCF);
    { uint8_t d[] = {0x00, 0x83, 0x30}; writeData(d, sizeof(d)); }

    writeCommand(0xED);
    { uint8_t d[] = {0x64, 0x03, 0x12, 0x81}; writeData(d, sizeof(d)); }

    writeCommand(0xE8);
    { uint8_t d[] = {0x85, 0x01, 0x79}; writeData(d, sizeof(d)); }

    writeCommand(0xCB);
    { uint8_t d[] = {0x39, 0x2C, 0x00, 0x34, 0x02}; writeData(d, sizeof(d)); }

    writeCommand(0xF7);
    writeDataByte(0x20);

    writeCommand(0xEA);
    { uint8_t d[] = {0x00, 0x00}; writeData(d, sizeof(d)); }

    writeCommand(0xC0); // Power control
    writeDataByte(0x26);

    writeCommand(0xC1); // Power control
    writeDataByte(0x11);

    writeCommand(0xC5); // VCOM control
    { uint8_t d[] = {0x35, 0x3E}; writeData(d, sizeof(d)); }

    writeCommand(0xC7);
    writeDataByte(0xBE);

    writeCommand(0x36); // MADCTL
    writeDataByte(0x48);

    writeCommand(0x3A); // Pixel format
    writeDataByte(0x55); // 16-bit

    writeCommand(0xB1);
    { uint8_t d[] = {0x00, 0x1B}; writeData(d, sizeof(d)); }

    writeCommand(0xF2);
    writeDataByte(0x08);

    writeCommand(0x26);
    writeDataByte(0x01);

    writeCommand(0xE0); // positive gamma
    {
        uint8_t d[] = {
            0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87,
            0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00
        };
        writeData(d, sizeof(d));
    }

    writeCommand(0xE1); // negative gamma
    {
        uint8_t d[] = {
            0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78,
            0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F
        };
        writeData(d, sizeof(d));
    }

    writeCommand(0x11); // sleep out
    sleep_ms(120);

    writeCommand(0x29); // display on
    sleep_ms(20);

    setRotation(1);
    fillScreen(BLACK);
}

void ILI9341::setRotation(uint8_t rotation) {
    writeCommand(0x36);

    uint8_t madctl = 0x48;
    switch (rotation & 3) {
        case 0:
            madctl = 0x48;
            width = 240; height = 320;
            break;
        case 1:
            madctl = 0x28;
            width = 320; height = 240;
            break;
        case 2:
            madctl = 0x88;
            width = 240; height = 320;
            break;
        case 3:
            madctl = 0xE8;
            width = 320; height = 240;
            break;
    }
    writeDataByte(madctl);
}

void ILI9341::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writeCommand(0x2A);
    uint8_t dataX[] = {
        (uint8_t)(x0 >> 8), (uint8_t)(x0 & 0xFF),
        (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF)
    };
    writeData(dataX, sizeof(dataX));

    writeCommand(0x2B);
    uint8_t dataY[] = {
        (uint8_t)(y0 >> 8), (uint8_t)(y0 & 0xFF),
        (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF)
    };
    writeData(dataY, sizeof(dataY));

    writeCommand(0x2C);
}

void ILI9341::writeColor(uint16_t color, uint32_t count) {
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    gpio_put(dc, 1);
    gpio_put(cs, 0);

    uint8_t buf[128];
    for (size_t i = 0; i < sizeof(buf); i += 2) {
        buf[i] = hi;
        buf[i + 1] = lo;
    }

    while (count > 0) {
        uint32_t pixelsThisChunk = count > 64 ? 64 : count;
        spi_write_blocking(spi, buf, pixelsThisChunk * 2);
        count -= pixelsThisChunk;
    }

    gpio_put(cs, 1);
}

void ILI9341::fillScreen(uint16_t color) {
    setAddressWindow(0, 0, width - 1, height - 1);
    writeColor(color, (uint32_t)width * height);
}

void ILI9341::drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= width || y >= height) return;
    setAddressWindow(x, y, x, y);
    writeColor(color, 1);
}

void ILI9341::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= width || y >= height) return;
    if ((x + w) > width)  w = width - x;
    if ((y + h) > height) h = height - y;

    setAddressWindow(x, y, x + w - 1, y + h - 1);
    writeColor(color, (uint32_t)w * h);
}

void ILI9341::drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale) {
    if (scale == 0) return;
    
    const uint8_t* bitmap = nullptr;

    if (c >= '0' && c <= '9') {
        bitmap = digits5x7[c - '0'];
    } else if (c >= 'A' && c <= 'Z') {
        bitmap = letters5x7[c - 'A'];
    } else {
        switch (c) {
            case '+': bitmap = symbols5x7[0]; break;
            case '(': bitmap = symbols5x7[1]; break;
            case ')': bitmap = symbols5x7[2]; break;
            case '!': bitmap = symbols5x7[3]; break;
            case '$': bitmap = symbols5x7[4]; break;
            case ':': bitmap = symbols5x7[5]; break;
            case '?': bitmap = symbols5x7[6]; break;
            case '-': bitmap = symbols5x7[7]; break;
            case ' ': return;
            default:  return;
        }
    }

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t bits = bitmap[col];
        for (uint8_t row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                fillRect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

void ILI9341::drawText(uint16_t x, uint16_t y, const char* str, uint16_t color, uint8_t scale) {
    if (scale == 0) return;
    
    while (*str) {
        if (*str == ' ') {
            x += 6;
        } else {
            drawChar(x, y, *str, color, scale);
        }
        x += 6 * scale; // 5 pixels + 1 space
        str++;
    }
}

void ILI9341::drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        drawPixel(x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ILI9341::drawCircle(int x0, int y0, int r, uint16_t color) {
    int x = 0;
    int y = r;
    int d = 1 - r;

    while (y >= x) {
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 - y, y0 - x, color);

        x++;

        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
    }
}

void ILI9341::fillCircle(int x0, int y0, int r, uint16_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if ((x * x + y * y) <= (r * r)) {
                drawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}

