#ifndef GALACTIC_TETRIS_UTIL_H
#define GALACTIC_TETRIS_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>
#include "pico/stdlib.h"

#include "libraries/pico_graphics/pico_graphics.hpp"
#include "galactic_unicorn.hpp"

using namespace pimoroni;

void init_hue_map();
void from_hsv(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b);
void text(std::string t, Point p, float s = 1.0f, float a = 1.0f);
void rainbow_text(std::string t, uint32_t delay_ms = 0, bool (*check_func)() = nullptr);
void outline_text(std::string text, bool reverse = false, char colour = 255);
void rgb_from_byte(char val, uint8_t *r, uint8_t *g, uint8_t *b);
void pen_from_byte(char val);
char random_color(char *except = nullptr, int len = 0);

extern PicoGraphics_PenRGB888 graphics;
extern GalacticUnicorn galactic_unicorn;

#endif //GALACTIC_TETRIS_UTIL_H
