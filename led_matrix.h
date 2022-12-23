#pragma once

#include "hardware/pio.h"
#include "pico_graphics.hpp"
#include <drivers/plasma/ws2812.hpp>

using namespace pimoroni;
using namespace plasma;

class LEDMatrix {
    private:
        WS2812 led_strip;
        uint16_t width, height;
        uint8_t brightness;

    public:
        LEDMatrix(uint w, uint h, PIO pio, uint sm, uint pin, bool rgbw = false, WS2812::COLOR_ORDER color_order = WS2812::COLOR_ORDER::GRB);
        LEDMatrix(PicoGraphics *g, PIO pio, uint sm, uint pin, bool rgbw = false, WS2812::COLOR_ORDER color_order = WS2812::COLOR_ORDER::GRB);
        ~LEDMatrix();

        void update(PicoGraphics *graphics);

        void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
        void set_brightness(float value);
        float get_brightness();
        void adjust_brightness(float delta);

};

