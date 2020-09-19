#include <Arduboy2.h>
#include "bitmaps.h"

Arduboy2 arduboy;

int const grid_size = 4;
int grid_width = 0;
int grid_height = 0;

int const max_length = 64;
int snake_x [max_length] = {};
int snake_y [max_length] = {};

int snake_length = 3;
int head = snake_length-1;

int snake_dir_x = 1;
int snake_dir_y = 0;
int prev_dir_x = 1;
int prev_dir_y = 0;

int const r = grid_size/2;
int apple_x = 0;
int apple_y = 0;

bool paused = false;

bool flipped = false;
bool apple_flipped = false;
bool snake_flipped[max_length] = {};

int score = 0;

int room = 0;

int game_timer = 30;

int menu_item = 0;
String menu_items[4] = {"PLAY","CONTROLS","ABOUT"};

void setup() {
  arduboy.boot();
  arduboy.blank();
  arduboy.setFrameRate(30);
  arduboy.initRandomSeed();
  arduboy.display();

  Serial.begin(9600);

  grid_width = WIDTH/grid_size-1;
  grid_height = HEIGHT/grid_size-3;

  reset_snake();

  randomize_apple();
}
// Also checks for self-collision
void render_snake() {
  int head_x = snake_x[head];
  int head_y = snake_y[head];
  int current_x = head_x;
  int current_y = head_y;
  
  for (int i = head; i > head-snake_length; i--) {
    int index = i;
    if (index < 0)
      index += max_length;
      
    current_x = snake_x[index];
    current_y = snake_y[index];
    if (current_x == head_x && current_y == head_y && index!= head) room = 2;
    
    if (snake_flipped[index]){
      arduboy.drawRect(current_x*grid_size, current_y*grid_size, 
                       grid_size, grid_size);
    } else {
      arduboy.fillRect(current_x*grid_size, current_y*grid_size, 
                       grid_size, grid_size);
    }
  }
}

void render_apple() {
  if (apple_flipped){
      arduboy.drawRect(apple_x*grid_size, apple_y*grid_size, 
                       grid_size, grid_size);
    } else {
      arduboy.fillRect(apple_x*grid_size, apple_y*grid_size, 
                       grid_size, grid_size);
    }
}

void render_pause() {
  arduboy.fillRect(44,24,41,16,BLACK);
  arduboy.drawBitmap(44,24,paused_icon,41,16,1);
}

void render_2_digit_number(int x, int y, int n) {
  arduboy.drawBitmap(x+5,y,numbers[n%10],8,4,1);
  if (n > 9) {
    arduboy.drawBitmap(x,y,numbers[(n-n%10)/10],8,4,1);
  }
}

void render_bottom_bar() {
  arduboy.drawBitmap(0,56,bottom_bar,128,9,1);
  render_2_digit_number(29, 57, score);
  render_2_digit_number(119,57, game_timer);
}

void render_menu() {
  arduboy.drawBitmap(0,0,menu_background,128,64,1);
 
  int pos = 17;
  for (int i = 0; i < 3;i++) {
    int w = 5*menu_items[i].length();
    if(i==menu_item) {
      arduboy.drawRect(pos-2,43,w+3,10);
    }
    pos += w + 5;
  }
}

void render_game_over() {
  arduboy.fillRect(32,16,64,32,0);
  arduboy.drawBitmap(32,16,game_over,64,32,1);
}

void render_controls() {
  arduboy.drawBitmap(0,0,controls_background,128,64,1);
}

void render_about() {
  arduboy.drawBitmap(0,0,about_background,128,64,1);
}

void handle_game_input() {
  arduboy.pollButtons();
  switch (room) {
    case 0:
      if (arduboy.justPressed(UP_BUTTON))
        menu_item = 2-menu_item;
      if (arduboy.justPressed(DOWN_BUTTON)) 
        menu_item = 2-menu_item;
      if (arduboy.justPressed(LEFT_BUTTON)) 
        menu_item -= 1;
      if (arduboy.justPressed(RIGHT_BUTTON)) 
        menu_item += 1;
      menu_item = (menu_item+3) %3;
      if (arduboy.justPressed(A_BUTTON)) {
        switch(menu_item) {
          case 0:
            room = 1;
            break;
          case 1:
            room = 3;
            break;
          case 2:
            room = 4;
            break;
        }
      }
      break;
    case 1:
      if (!paused) {
        if (arduboy.justPressed(UP_BUTTON) && prev_dir_y == 0) {
          snake_dir_x = 0;
          snake_dir_y = -1+2*flipped; 
        }
        if (arduboy.justPressed(DOWN_BUTTON) && prev_dir_y == 0) {
          snake_dir_x = 0;
          snake_dir_y = 1-2*flipped;
        }
        if (arduboy.justPressed(LEFT_BUTTON) && prev_dir_x == 0) {
          snake_dir_x = -1+2*flipped;
          snake_dir_y = 0;
        }
        if (arduboy.justPressed(RIGHT_BUTTON) && prev_dir_x == 0) {
          snake_dir_x = 1-2*flipped;
          snake_dir_y = 0;
        }
      }
      if (arduboy.justPressed(A_BUTTON)) {
        paused = !paused;
      }
      break;
    case 2:
      if (arduboy.justPressed(A_BUTTON)) {
        room = 0;
        score = 0;
        flipped = false;
        reset_snake();
        randomize_apple();
      }
      if (arduboy.justPressed(B_BUTTON)) {
        arduboy.clear();
        render_snake();
        render_apple();
        render_bottom_bar();
        Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
      }
      break;
    default:
      if (arduboy.justPressed(B_BUTTON)) {
        room = 0;
      }
      break;
  }
}

void update_snake() { 
  head += 1;
  int prev = head - 1;
  
  if (head > max_length - 1)
    head = 0;
    
  int next_x = snake_x[prev]+snake_dir_x;
  int next_y = snake_y[prev]+snake_dir_y;

  prev_dir_x = snake_dir_x;
  prev_dir_y = snake_dir_y;
  
  if (next_y<0){
    next_y=grid_height;
    next_x=grid_width-next_x;
    flipped = !flipped;
  }
  else if (next_y>grid_height){
    next_y=0;
    next_x=grid_width-next_x;
    flipped = !flipped;
  }
  
  if (next_x<0)
    next_x=grid_width;
  else if (next_x>grid_width)
    next_x=0;
    
  snake_x[head] = next_x;
  snake_y[head] = next_y;
  snake_flipped[head] = flipped;

  if (apple_flipped == flipped && next_x == apple_x && next_y == apple_y){
    snake_length+=1;
    randomize_apple();
    score+=1;
  }
}

void reset_snake() {
  snake_length = 3;
  head = snake_length-1;

  snake_dir_x = 1;
  snake_dir_y = 0;
  prev_dir_x = 1;
  prev_dir_y = 0;
  
  snake_x[0] = grid_width/2;
  snake_x[1] = grid_width/2+1;
  snake_x[2] = grid_width/2+2;
  
  snake_y[0] = grid_height/2;
  snake_y[1] = grid_height/2;
  snake_y[2] = grid_height/2;
  
  snake_flipped[0] = false;
  snake_flipped[1] = false;
  snake_flipped[2] = false;
}

void randomize_apple() {
  apple_x = random(1,grid_width);
  apple_y = random(1,grid_height);
  apple_flipped = random(0,2);
  game_timer = max(30-score,10);
 }

void loop() {
  
  if (!arduboy.nextFrame()) return;

  handle_game_input();

  switch (room) {
    case 0:
      arduboy.clear();
      render_menu();
      break;
    case 1:
      if (paused) {
        render_pause();
      } else {
        if (arduboy.everyXFrames(30)){ game_timer--;}
        
        if (game_timer == 0){
          render_bottom_bar();
          room = 2;}
        
        if (arduboy.everyXFrames(4)) {
          update_snake();
          
          arduboy.clear();
          
          render_snake();
          render_apple();
          render_bottom_bar();
        }
      }
      break;
    case 2:
      render_game_over();
      break;
    case 3:
      arduboy.clear();
      render_controls();
      break;
    case 4:
      arduboy.clear();
      render_about();
      break;
  }
  
  arduboy.display(); 
}
