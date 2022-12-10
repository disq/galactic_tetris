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
bool autoplay = false;
bool exit_autoplay = false;

const uint32_t min_speed = 200;
const uint32_t start_speed = 400; // decrease this to make it faster
uint32_t speed;
int speed_step = 10;

typedef struct {
    char **array;
    int width, row, col;
} Shape;
Shape current;

#define NUM_SHAPES 7

const Shape ShapesArray[NUM_SHAPES]= {
	{(char *[]){(char []){0,1,1},(char []){1,1,0}, (char []){0,0,0}}, 3},                           //S shape
	{(char *[]){(char []){1,1,0},(char []){0,1,1}, (char []){0,0,0}}, 3},                           //Z shape
	{(char *[]){(char []){0,1,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //T shape
	{(char *[]){(char []){0,0,1},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //L shape
	{(char *[]){(char []){1,0,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //flipped L shape    
	{(char *[]){(char []){1,1},(char []){1,1}}, 2},                                                 //square shape
	{(char *[]){(char []){0,0,0,0}, (char []){1,1,1,1}, (char []){0,0,0,0}, (char []){0,0,0,0}}, 4} //long bar shape
	// you can add any shape like it's done above. Don't be naughty.
};

char shape_colors[NUM_SHAPES] = {0}; // starting colour for each different shape, randomly assigned on startup

bool check_key() {
  return galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_A) ||
         galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_B) ||
         galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_C) ||
         galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_D);
}

void key_animate() {
  static uint8_t y = 0, old_y = 0;
  static uint32_t last_check_ms = 0;

  graphics.set_pen(0, 0, 0);
  graphics.pixel(Point(0, old_y));

  graphics.set_pen(255, 255, 255);
  graphics.pixel(Point(0, y));

  if (last_check_ms + 500 > millis()) {
    return;
  }

  old_y = y;
  y+=2;
  if (y > 5) y = 0;

  last_check_ms = millis();
}

void wait_key_animate() {
  while(true) {
    key_animate();
    galactic_unicorn.update(&graphics);
    for(int i = 0; i < 50; i++) {
      if (check_key()) return;
      sleep_ms(10);
    }
  }
}

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
	for(i = 0; i < shape.width; i++) {
		for(j = 0; j < shape.width; j++){
			if((shape.col+j < 0 || shape.col+j >= COLS || shape.row+i >= ROWS)) { //Out of borders
				if(array[i][j] > 0) //but is it just a phantom?
					return false;
			}
			else if(Table[shape.row+i][shape.col+j] > 0 && array[i][j] > 0) return false;
		}
	}
	return true;
}

void SetNewRandomShape(){ //updates [current] with new shape
  int shape_index = rand()%NUM_SHAPES;
	Shape new_shape = CopyShape(ShapesArray[shape_index]);
  for(int i = 0; i < new_shape.width; i++) {
    for(int j = 0; j < new_shape.width; j++) {
      if(new_shape.array[i][j]) {
        new_shape.array[i][j] = shape_colors[shape_index];
      }
    }
  }

  new_shape.col = (COLS-new_shape.width+1) / 2;
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
		for(j = 0, k = width-1; j < width; j++, k--){
				shape.array[i][j] = temp.array[k][i];
		}
	}
	DeleteShape(temp);
}

void WriteToTable(){
	for(int i = 0; i < current.width; i++) {
		for(int j = 0; j < current.width; j++) {
			if (current.array[i][j] > 0) Table[current.row+i][current.col+j] = current.array[i][j];
		}
	}
}

void RemoveFullRowsAndUpdateScore() {
	int count = 0;
	for(int i = 0; i < ROWS; i++){
		int sum = 0;
		for(int j = 0; j < COLS; j++) sum += Table[i][j] > 0;

		if(sum == COLS) {
			count++;
      int k;
			for(int k = i; k >= 1; k--)
        for(int l = 0; l < COLS; l++)
					Table[k][l]=Table[k-1][l];
			for(int l = 0; l < COLS; l++) Table[k][l]=0;
			speed -= speed_step;
      if (speed < min_speed) speed = min_speed;
		}
	}
	score += 100*count;
}

void PrintTable() {
	char Buffer[ROWS][COLS] = {0};
	for(int i = 0; i < current.width ;i++){
		for(int j = 0; j < current.width ; j++){
			if(current.array[i][j] > 0)
				Buffer[current.row+i][current.col+j] = current.array[i][j];
		}
	}

  graphics.set_pen(0, 0, 0);
  graphics.clear();

	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
      char val = Table[i][j] + Buffer[i][j];
      if (val == 0) continue;
      pen_from_byte(val);
      graphics.pixel(Point(ROWS - i, j)); // flip top-bottom
		}
	}

  if (autoplay) key_animate();

  galactic_unicorn.update(&graphics);
}

void ClearTable() {
  for(int i = 0; i < ROWS; i++) for(int j = 0; j < COLS; j++) Table[i][j] = 0;
}

bool ManipulateCurrent(int action){
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
        return true;
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
  return false;
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

bool SolveCurrent(int *NumRotations, int *Row, int *Column){
  Shape temp = CopyShape(current);

  int good_col = -1;
  int good_rot = 0;
  int max_full_rows = 0; // most important
  int min_holes_below = COLS * ROWS; // second most important
  int max_fill_cols = 0; // third most important
  int max_row = temp.row; // least important

  bool valid = false;

  for(int rot_index = 0; rot_index < 4; rot_index++) {
    for (int col_index = 0; col_index < COLS; col_index++) {
      bool had_valid = false;
      temp.col = col_index;

      // if we start checking from the lowest row and go up we can't get into a situation where we can't move down
      for (int i = temp.row; i < ROWS; i++) {
        temp.row = i;
        if (!CheckPosition(temp)) break;
        had_valid = true;
      }
      if (!had_valid) continue; // try next column

      temp.row--; // our position isn't valid, move one row up

      int full_rows = 0, fill_cols = 0;

      char tableCopy[ROWS][COLS]; // copy the table: value is 1 for already filled, 0 for empty, 2 for shape
      for (int i = 0; i < ROWS; i++) for (int j = 0; j < COLS; j++) tableCopy[i][j] = Table[i][j] > 0 ? 1 : 0;

      // check maximum fill in table
      for (int i = 0; i < temp.width; i++) {

        for (int j = 0; j < temp.width; j++) if (temp.array[i][j] > 0) tableCopy[temp.row + i][temp.col + j] = temp.array[i][j] > 0 ? 2 : 0;

        int sum = 0, shape_sum = 0, nonshape_sum = 0;
        // for every row, we check if it's full
        // we also calculate how many cells are filled by the shape in this position, but we ignore if the row only has cells filled by the shape and the rest is empty
        for (int j = 0; j < COLS; j++) {
          sum += tableCopy[temp.row+i][j] > 0;
          nonshape_sum += tableCopy[temp.row+i][j] == 1;
          shape_sum += tableCopy[temp.row+i][j] == 2;
        }
        if (sum == COLS) full_rows++;

        if (shape_sum > 0 && nonshape_sum == 0) {
          // if the row is only filled by the shape, we don't count it
          continue;
        }
        fill_cols += shape_sum + nonshape_sum;
      }

      int holes_below = 0;
      for (int i = temp.row; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
          if (tableCopy[i][j] == 0) holes_below++;

      printf("[rot:%d] full rows:%d fill_cols:%d holes:%d\n", rot_index, full_rows, fill_cols, holes_below);

      if (full_rows > max_full_rows ||
      (full_rows == max_full_rows && holes_below < min_holes_below) ||
      (full_rows == max_full_rows && holes_below == min_holes_below && fill_cols > max_fill_cols) ||
      (full_rows == max_full_rows && holes_below == min_holes_below && fill_cols == max_fill_cols && temp.row > max_row)) {
        max_full_rows = full_rows;
        max_fill_cols = fill_cols;
        min_holes_below = holes_below;
        max_row = temp.row;
        good_col = temp.col;
        good_rot = rot_index;
        valid = true;
        printf("Solve candidate: Row:%d Col:%d Rot:%d // Full:%d Fill:%d Holes:%d\n", max_row, good_col, good_rot, full_rows, fill_cols, holes_below);
      }
    }

    temp.col = current.col;
    temp.row = current.row;
    RotateShape(temp);
  }
  DeleteShape(temp);

  *NumRotations = good_rot;
  *Row = max_row;
  *Column = good_col;
  printf("Solved: Row:%d Col:%d Rot:%d Full:%d Fill:%d Holes:%d valid:%c\n\n", max_row, good_col, good_rot, max_full_rows, max_fill_cols, min_holes_below, valid ? 'Y' : 'N');
  return valid;
}

bool loop_things(bool paused_check = true) {
  bool brightness_up = galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_BRIGHTNESS_UP);
  bool brightness_down = galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_BRIGHTNESS_DOWN);
  if (brightness_up && brightness_down) {
    galactic_unicorn.set_brightness(0.0f);
    do_auto_light = true;
    sleep_ms(100);
    auto_adjust_brightness();
    printf("Auto-brightness re-enabled\n");
    sleep_ms(500);
    return false;
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
    if (speed < min_speed) speed = min_speed;
    else sleep_ms(150);
  } else if (galactic_unicorn.is_pressed(GalacticUnicorn::SWITCH_VOLUME_DOWN)) {
    speed += speed_step;
    if (speed > start_speed) speed = start_speed;
    else sleep_ms(150);
  }

  if (autoplay && check_key()) {
    exit_autoplay = true;
  }

  if (paused_check) {
    while(paused) {
      loop_things(false);
      galactic_unicorn.update(&graphics);
      sleep_ms(10);
    }
  }

  return exit_autoplay;
}

bool loopy_sleep(int ms) {
  int i;
  for (i = 0; i < ms; i+=10) {
    if (loop_things()) return true;
    sleep_ms(10);
  }
  return false;
}

void auto_play() {
  int num_rotations, row, col;
  if (!SolveCurrent(&num_rotations, &row, &col)) return;

  const int start_moves_after_rows = 5;

  int moves = num_rotations + abs(current.col - col);

  int row_diff = row - current.row;
  if (moves > 0 && row_diff > moves + start_moves_after_rows) {
    if (row_diff > start_moves_after_rows) row_diff = start_moves_after_rows;
    for(int i=0;i<row_diff;i++) {
      if (ManipulateCurrent('s')) return;
      if (loopy_sleep(100)) return;
    }
  }

  for (int i = 0; i < num_rotations; i++) {
    if (ManipulateCurrent('w')) return;
    if (loopy_sleep(250)) return;
  }

  while (current.col != col) {
    if (current.col < col) {
      if (ManipulateCurrent('d')) return;
    } else {
      if (ManipulateCurrent('a')) return;
    }
    if (loopy_sleep(100)) return;
  }

  while (current.row != row) {
    if (ManipulateCurrent('s')) return;
    if (loopy_sleep(50)) return;
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

    for(int i=0;i<NUM_SHAPES;i++) shape_colors[i] = random_color();

    autoplay = true;

    while(true) {
      pen_from_byte(0);
      graphics.clear();
      char main_colour = random_color();
      outline_text(autoplay ? "DEMO" : "PLAY!", true, main_colour);
      galactic_unicorn.update(&graphics);

      uint32_t last_update = millis();
      sleep_ms(700);

      score = 0;
      speed = start_speed;
      playing = true;
      paused = false;

      ClearTable();
      SetNewRandomShape();
      PrintTable();

      while (playing) {
        loop_things();

        if (autoplay) {
          auto_play();
          if (exit_autoplay) break;
        } else {

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
        }

        if (!paused && millis() - last_update > speed) {
          ManipulateCurrent('s');
          last_update = millis();
        }
      }

      graphics.clear();

      if (autoplay && exit_autoplay) {
        exit_autoplay = false;
        autoplay = false;
        continue;
      }


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
