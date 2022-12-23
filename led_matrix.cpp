#include <math.h>

#include "led_matrix.h"

  LEDMatrix::LEDMatrix(uint w, uint h, PIO pio, uint sm, uint pin, bool rgbw, WS2812::COLOR_ORDER color_order):
      led_strip(w * h, pio, sm, pin, WS2812::DEFAULT_SERIAL_FREQ, rgbw, color_order),
      width(w),
      height(h),
      brightness(50) {
        printf("plasma init: %d leds\n", led_strip.num_leds);

        led_strip.clear();
        led_strip.update();
  }

    LEDMatrix::LEDMatrix(PicoGraphics *g, PIO pio, uint sm, uint pin, bool rgbw, WS2812::COLOR_ORDER color_order):
      LEDMatrix::LEDMatrix(g->bounds.w, g->bounds.h, pio, sm, pin, rgbw, color_order) {

}

    LEDMatrix::~LEDMatrix() {
      led_strip.stop();
      led_strip.clear();
    }

    void LEDMatrix::set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
      if(x < 0 || x >= this->width || y < 0 || y >= this->height) return;

      r = (r * this->brightness) >> 8;
      g = (g * this->brightness) >> 8;
      b = (b * this->brightness) >> 8;

      uint32_t index = (y * this->width) + x;

      led_strip.set_rgb(index, r, g, b);
    }

    void LEDMatrix::set_brightness(float value) {
      value = value < 0.0f ? 0.0f : value;
      value = value > 1.0f ? 1.0f : value;
      this->brightness = floor(value * 256.0f);
    }

    float LEDMatrix::get_brightness() {
      return this->brightness / 255.0f;
    }

    void LEDMatrix::adjust_brightness(float delta) {
      this->set_brightness(this->get_brightness() + delta);
    }

    void LEDMatrix::update(PicoGraphics *graphics) {
      if(graphics->pen_type == PicoGraphics::PEN_RGB888) {
        uint32_t *p = (uint32_t *)graphics->frame_buffer;
        for(size_t j = 0; j < graphics->bounds.w * graphics->bounds.h; j++) {
          int x = j % graphics->bounds.w;
          int y = j / graphics->bounds.w;

          uint32_t col = *p;
          uint8_t r = (col & 0xff0000) >> 16;
          uint8_t g = (col & 0x00ff00) >>  8;
          uint8_t b = (col & 0x0000ff) >>  0;
          p++;

          set_pixel(x, y, r, g, b);
        }
      }
      else if(graphics->pen_type == PicoGraphics::PEN_RGB565) {
        uint16_t *p = (uint16_t *)graphics->frame_buffer;
        for(size_t j = 0; j < graphics->bounds.w * graphics->bounds.h; j++) {
          int x = j % graphics->bounds.w;
          int y = j / graphics->bounds.w;

          uint16_t col = __builtin_bswap16(*p);
          uint8_t r = (col & 0b1111100000000000) >> 8;
          uint8_t g = (col & 0b0000011111100000) >> 3;
          uint8_t b = (col & 0b0000000000011111) << 3;
          p++;

          set_pixel(x, y, r, g, b);
        }
      } else {
        printf("unhandled pen\n");
      }

      led_strip.update();
    }
