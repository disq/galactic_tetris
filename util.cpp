// Utils hacked together from https://github.com/pimoroni/pimoroni-pico/blob/main/examples/galactic_unicorn

#include "util.hpp"

using namespace pimoroni;

uint8_t hue_map[53][3];

void init_hue_map() {
  for (int i = 0; i < 53; i++) {
    from_hsv(i / 53.0f, 1.0f, 0.1f, hue_map[i][0], hue_map[i][1], hue_map[i][2]);
  }
}

void from_hsv(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b) {
  float i = floor(h * 6.0f);
  float f = h * 6.0f - i;
  v *= 255.0f;
  uint8_t p = v * (1.0f - s);
  uint8_t q = v * (1.0f - f * s);
  uint8_t t = v * (1.0f - (1.0f - f) * s);

  switch (int(i) % 6) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
}

void text(std::string t, Point p, float s, float a) {
  int w = graphics.measure_text(t, s);
  p.x += (53 / 2) - (w / 2);
  p.y += (11 / 2);
  graphics.text(t, Point(p.x, p.y), -1, s, a);
  graphics.text(t, Point(p.x + 1, p.y), -1, s, a);
  graphics.text(t, Point(p.x + 1, p.y + 1), -1, s, a);
  graphics.text(t, Point(p.x, p.y + 1), -1, s, a);
}

void rainbow_text(std::string t, uint32_t delay_ms, bool (*check_func)()) {
  graphics.set_font("sans");
  uint i = 0;

  uint32_t start = millis();

  while(true) {
    if (delay_ms > 0 && start + delay_ms < millis()) break;
    if (check_func && check_func()) break;

    i++;
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    float s = 0.8f;//0.65f + (sin(i / 25.0f) * 0.15f);
    float a = 1.0f;// (sin(i / 25.0f) * 100.0f);

    float x = (sin((i) / 50.0f) * 90.0f);
    float y = (cos((i) / 40.0f) * 5.0f);
    graphics.set_pen(255, 255, 255);
    text(t, Point(x, y), s, a);

    uint8_t *p = (uint8_t *)graphics.frame_buffer;
    for(size_t i = 0; i < 53 * 11; i++) {
      int x = i % 53;
      int y = i / 53;
      uint r = *p++;
      uint g = *p++;
      uint b = *p++;
      p++;

      if(r > 0) {
        r = hue_map[x][0];
        g = hue_map[x][1];
        b = hue_map[x][2];
      }

      graphics.set_pen(r, g, b);
      graphics.pixel(Point(x, y));
    }

    galactic_unicorn.update(&graphics);
  }
}

void outline_text(std::string text, bool reverse, char colour) {
  graphics.set_font("bitmap8");
//  uint8_t v = (sin(float(millis()) / 100.0f) + 1.0f) * 127.0f;
  uint w = graphics.measure_text(text, 1);

  int x = 53 / 2 - w / 2 + 1, y = 2;

  pen_from_byte(reverse ? colour : 0);
  graphics.text(text, Point(x - 1, y - 1), -1, 1);
  graphics.text(text, Point(x    , y - 1), -1, 1);
  graphics.text(text, Point(x + 1, y - 1), -1, 1);
  graphics.text(text, Point(x - 1, y    ), -1, 1);
  graphics.text(text, Point(x + 1, y    ), -1, 1);
  graphics.text(text, Point(x - 1, y + 1), -1, 1);
  graphics.text(text, Point(x    , y + 1), -1, 1);
  graphics.text(text, Point(x + 1, y + 1), -1, 1);

  pen_from_byte(reverse ? 0: colour);
  graphics.text(text, Point(x, y), -1, 1);
}

void rgb_from_byte(char val, uint8_t *r, uint8_t *g, uint8_t *b) {
  if (val == 0) {
    *r = 0;
    *g = 0;
    *b = 0;
    return;
  }

  uint8_t val_r = (val & 0b01100000);
  uint8_t val_g = (val & 0b00011100) << 2;
  uint8_t val_b = (val & 0b00000011) << 5;
  if (val_r > val_g && val_r > val_b) val_r += 128;
  else if (val_g > val_r && val_g > val_b) val_g += 128;
  else if (val_b > val_r && val_b > val_g) val_b += 128;
  else { val_r = 255; val_g = 255; val_b = 255; }

  *r = val_r;
  *g = val_g;
  *b = val_b;
}

void pen_from_byte(char val) {
  uint8_t r, g, b;
  rgb_from_byte(val, &r, &g, &b);
  graphics.set_pen(r, g, b);
}

char random_color(char *except, int len) {
  const char mask = 0b00101101; // don't want similar colours to the old one
  static char oldColor = 0;
  char col;

  do {
    col = (rand() % 255) + 1;

    for (int i = 0; i < len; i++) {
      if ((col & mask) == (except[i] & mask)) {
        col = oldColor; // continue outer loop
        break;
      }
    }

  }while((col & mask) == (oldColor & mask));

  oldColor = col;

  return col;
}
