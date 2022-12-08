#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"

#include "libraries/pico_graphics/pico_graphics.hpp"
#include "galactic_unicorn.hpp"
#include "util.hpp"

using namespace pimoroni;

PicoGraphics_PenRGB888 graphics(53, 11, nullptr);
GalacticUnicorn galactic_unicorn;

#define ROWS 53 // you can change height and width of table with ROWS and COLS
#define COLS 11

char Table[ROWS][COLS];
int score;
bool playing;
bool paused = false;

const uint32_t min_speed = 200;
const uint32_t start_speed = 400; // decrease this to make it faster
uint32_t speed;
int speed_step = 10;

typedef struct {
    char **array;
    int width, row, col;
} Shape;
Shape current;

const Shape ShapesArray[7]= {
	{(char *[]){(char []){0,1,1},(char []){1,1,0}, (char []){0,0,0}}, 3},                           //S shape
	{(char *[]){(char []){1,1,0},(char []){0,1,1}, (char []){0,0,0}}, 3},                           //Z shape
	{(char *[]){(char []){0,1,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //T shape
	{(char *[]){(char []){0,0,1},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //L shape
	{(char *[]){(char []){1,0,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //flipped L shape    
	{(char *[]){(char []){1,1},(char []){1,1}}, 2},                                                 //square shape
	{(char *[]){(char []){0,0,0,0}, (char []){1,1,1,1}, (char []){0,0,0,0}, (char []){0,0,0,0}}, 4} //long bar shape
	// you can add any shape like it's done above. Don't be naughty.
};

Shape CopyShape(Shape shape){
	Shape new_shape = shape;
	char **copyshape = shape.array;
	new_shape.array = (char**)malloc(new_shape.width*sizeof(char*));
  int i, j;
  for(i = 0; i < new_shape.width; i++) {
		new_shape.array[i] = (char*)malloc(new_shape.width*sizeof(char));
		for(j=0; j < new_shape.width; j++) {
			new_shape.array[i][j] = copyshape[i][j];
    }
  }
  return new_shape;
}

void DeleteShape(Shape shape){
  for(int i = 0; i < shape.width; i++) free(shape.array[i]);
  free(shape.array);
}

bool CheckPosition(Shape shape){ // Check the position of the copied shape
	char **array = shape.array;
	int i, j;
	for(i = 0; i < shape.width;i++) {
		for(j = 0; j < shape.width ;j++){
			if((shape.col+j < 0 || shape.col+j >= COLS || shape.row+i >= ROWS)){ //Out of borders
				if(array[i][j]) //but is it just a phantom?
					return false;
				
			}
			else if(Table[shape.row+i][shape.col+j] && array[i][j])
				return false;
		}
	}
	return true;
}

void SetNewRandomShape(){ //updates [current] with new shape
	Shape new_shape = CopyShape(ShapesArray[rand()%7]);
  char col = (rand() % 255) + 1;
  for(int i = 0; i < new_shape.width; i++) {
    for(int j = 0; j < new_shape.width; j++) {
      if(new_shape.array[i][j]) {
        new_shape.array[i][j] = col;
      }
    }
  }

  new_shape.col = rand()%(COLS-new_shape.width+1);
  new_shape.row = 0;
  DeleteShape(current);
  current = new_shape;
  if(!CheckPosition(current)) playing = false;
}

void RotateShape(Shape shape){ //rotates clockwise
	Shape temp = CopyShape(shape);
	int i, j, k, width;
	width = shape.width;
	for(i = 0; i < width ; i++){
		for(j = 0, k = width-1; j < width ; j++, k--){
				shape.array[i][j] = temp.array[k][i];
		}
	}
	DeleteShape(temp);
}

void WriteToTable(){
	for(int i = 0; i < current.width ;i++) {
		for(int j = 0; j < current.width ; j++) {
			if (current.array[i][j]) Table[current.row+i][current.col+j] = current.array[i][j];
		}
	}
}

void RemoveFullRowsAndUpdateScore(){
	int sum, count=0;
	for(int i=0;i<ROWS;i++){
		sum = 0;
		for(int j=0;j< COLS;j++) sum += Table[i][j] > 0;

		if(sum==COLS){
			count++;
      int k, l;
			for(k = i;k >=1;k--)
				for(l=0;l<COLS;l++)
					Table[k][l]=Table[k-1][l];
			for(l=0;l<COLS;l++)
				Table[k][l]=0;
			speed -= speed_step;
      if (speed < min_speed) speed = min_speed;
		}
	}
	score += 100*count;
}

void PrintTable(){
	char Buffer[ROWS][COLS] = {0};
	for(int i = 0; i < current.width ;i++){
		for(int j = 0; j < current.width ; j++){
			if(current.array[i][j])
				Buffer[current.row+i][current.col+j] = current.array[i][j];
		}
	}

  graphics.set_pen(0, 0, 0);
  graphics.clear();

	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
      char val = Table[i][j] + Buffer[i][j];
      pen_from_byte(val);
      graphics.pixel(Point(ROWS - i, j)); // flip top-bottom
		}
	}

  galactic_unicorn.update(&graphics);
}

void ClearTable() {
  for(int i = 0; i < ROWS; i++) for(int j = 0; j < COLS; j++) Table[i][j] = 0;
}

void ManipulateCurrent(int action){
	Shape temp = CopyShape(current);
	switch(action){
		case 's':
			temp.row++;  //move down
			if(CheckPosition(temp))
				current.row++;
			else {
				WriteToTable();
				RemoveFullRowsAndUpdateScore();
        SetNewRandomShape();
			}
			break;
		case 'd':
			temp.col++;  //move right
			if(CheckPosition(temp))
				current.col++;
			break;
		case 'a':
			temp.col--;  //move left
			if(CheckPosition(temp))
				current.col--;
			break;
		case 'w':
			RotateShape(temp); // rotate clockwise
			if(CheckPosition(temp))
				RotateShape(current);
			break;
	}
	DeleteShape(temp);
	PrintTable();
}


static bool do_auto_light = true;

void auto_adjust_brightness() {
  float light_level = ((float)galactic_unicorn.light())/4095.0f;
  galactic_unicorn.set_brightness(light_level + 0.15f);
  if (paused) galactic_unicorn.update(&graphics);
}

bool light_timer_callback(struct repeating_timer *t) {
  if (do_auto_light) auto_adjust_brightness();
  return true;
}

bool check_key() {
  return galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_A) ||
  galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_B) ||
  galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_C) ||
  galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_D);
}

void wait_key_animate(bool reverse = false) {
  uint8_t y = 0, old_y = 0;

  while(true) {
    if (reverse) {
      graphics.set_pen(255, 255, 255);
    } else {
      graphics.set_pen(0, 0, 0);
    }
    graphics.pixel(Point(0, old_y));

    if (!reverse) {
      graphics.set_pen(255, 255, 255);
    } else {
      graphics.set_pen(0, 0, 0);
    }
    graphics.pixel(Point(0, y));

    old_y = y;
    y+=2;
    if (y > 5) y = 0;

    galactic_unicorn.update(&graphics);
    for(int i = 0; i < 50; i++) {
      if (check_key()) return;
      sleep_ms(10);
    }
  }
}

int main() {
    stdio_init_all();
    galactic_unicorn.init();
    init_hue_map();

    graphics.set_pen(0, 0, 0);
    graphics.clear();
    galactic_unicorn.update(&graphics);

    auto_adjust_brightness();

    struct repeating_timer light_timer;
    add_repeating_timer_ms(1000, light_timer_callback, NULL, &light_timer);

    outline_text("Key 2 Start", false);
    galactic_unicorn.update(&graphics);
//    rainbow_text("Key 2 Start", 0, check_key);
    wait_key_animate();

    srand(millis());

    while(true) {
      pen_from_byte(0);
      graphics.clear();
      char main_colour = (rand()%255)+1;
      outline_text("PLAY!", true, main_colour);
      galactic_unicorn.update(&graphics);

      uint32_t last_update = millis();
      sleep_ms(1000);

      score = 0;
      speed = start_speed;
      playing = true;
      paused = false;

      ClearTable();
      SetNewRandomShape();
      PrintTable();

      while (playing) {
        bool brightness_up = galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_BRIGHTNESS_UP);
        bool brightness_down = galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_BRIGHTNESS_DOWN);
        if (brightness_up && brightness_down) {
          galactic_unicorn.set_brightness(0.0f);
          do_auto_light = true;
          sleep_ms(100);
          auto_adjust_brightness();
          printf("Auto-brightness re-enabled\n");
          sleep_ms(500);
          continue; // we've messed with everything this loop, restart
        } else if (brightness_up) {
          galactic_unicorn.adjust_brightness(+0.01f);
          if (paused) galactic_unicorn.update(&graphics);
          do_auto_light = false;
          sleep_ms(200);
        } else if (brightness_down) {
          galactic_unicorn.adjust_brightness(-0.01f);
          if (paused) galactic_unicorn.update(&graphics);
          do_auto_light = false;
          sleep_ms(200);
        }

        if (galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_SLEEP)) {
          paused = !paused;
          sleep_ms(300);
        }
        if (galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_VOLUME_UP)) {
          speed -= speed_step;
          if (speed < min_speed) speed = speed;
          sleep_ms(150);
        } else if (galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_VOLUME_DOWN)) {
          speed += speed_step;
          if (speed > start_speed) speed = start_speed;
          sleep_ms(150);
        }

        // use a vi-like keymap
        if (galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_A)) {
          ManipulateCurrent('a');
          sleep_ms(200);
        } else if (galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_D)) {
          ManipulateCurrent('d');
          sleep_ms(200);
        } else if (galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_C)) {
          ManipulateCurrent('w');
          sleep_ms(300); // delay more for rotate
        } else if (galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_B)) {
          ManipulateCurrent('s');
          sleep_ms(100); // less for down
        }

        if (!paused && millis() - last_update > speed) {
          ManipulateCurrent('s');
          last_update = millis();
        }
      }

      graphics.clear();

      outline_text("Game Over!", true, 0b01100101);
      galactic_unicorn.update(&graphics);
      sleep_ms(1000);

//      rainbow_text("Score", 500);

      graphics.clear();
      outline_text("Score", true, main_colour);
      galactic_unicorn.update(&graphics);
      sleep_ms(500);
      graphics.clear();
      outline_text(std::to_string(score));
      galactic_unicorn.update(&graphics);
      wait_key_animate();

//      rainbow_text(std::to_string(score), 0, check_key);
    } // while(true)

  return 0;
}
