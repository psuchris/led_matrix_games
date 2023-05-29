
#define PxMATRIX_double_buffer true

#include <Arduino.h>
#include <PxMatrix.h>       // if issues down the road, PlatformIO Library manager installs latest release, 
                            // but ArduinoIDE installs the lastest version of the master branch
                            // only the latest version works, maybe consider manually adding library to project
#include <SnakeGame.h>
#include <InputsFonts.h>

// Pins for LED MATRIX
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t display_draw_time = 1; // 30-60 is usually fine              brightness but only used if fastupdate isn't used

uint8_t MATRIX_WIDTH = 64;
uint8_t MATRIX_HEIGHT = 64;
PxMATRIX display(MATRIX_WIDTH, MATRIX_HEIGHT, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

uint16_t c_red = display.color565(255, 0, 0);
uint16_t c_green = display.color565(0, 255, 0);
uint16_t c_blue = display.color565(0, 0, 255);
uint16_t c_white = display.color565(255, 255, 255);
uint16_t c_yellow = display.color565(255, 255, 0);
uint16_t c_cyan = display.color565(0, 255, 255);
uint16_t c_magenta = display.color565(255, 0, 255);
uint16_t c_black = display.color565(0, 0, 0);
// uint16_t colors[8] = { c_red, c_green, c_blue, c_white, c_yellow, c_cyan, c_magenta, c_black };

uint16_t loops = 0;
uint16_t fps = 0;
unsigned long loop_time = 0;

Inputs inputs;

SnakeGame *snake_game;      // display wouldn't work right if defining this in setup()

void IRAM_ATTR display_updater()
{
    // Increment the counter and set the time of ISR
    portENTER_CRITICAL_ISR(&timerMux);
    display.display(display_draw_time);
    portEXIT_CRITICAL_ISR(&timerMux);
}

void display_update_enable(bool is_enable)
{
    if (is_enable)
    {
        timer = timerBegin(0, 80, true);
        timerAttachInterrupt(timer, &display_updater, true);
        timerAlarmWrite(timer, 2000, true);
        timerAlarmEnable(timer);
    }
    else
    {
        timerDetachInterrupt(timer);
        timerAlarmDisable(timer);
    }
}

const char *game_strings[] = {"Snake", "Tetris"};
uint8_t num_games = 2;
const char *player_strings[] = {"1 Player", "2 Players"};
uint8_t num_player_options = 2;
uint8_t selected = 1;
uint8_t selected_game = 1;
uint8_t selected_players = 1;

void displaySelectMenu(const char *items[], uint8_t num_items, uint8_t selected)
{
    display.clearDisplay();
    for (int i = 0; i < num_items; i++)
    {
        display.setCursor(1, (i * 8) + 2);
        if ((i + 1) == selected)
        {
            display.setTextColor(c_white);
            display.print(">");
        }
        else
        {
            display.setTextColor(c_cyan);
            display.print(" ");
        }
        display.print(items[i]);
    }
}

uint8_t selectMenuItem(const char *items[], uint8_t num_items)
{
    uint8_t selected = 1;
    displaySelectMenu(items, num_items, selected);
    while (true)
    {
        inputs.update();
        bool _up = inputs.UP_P1 || inputs.UP_P2;
        bool _down = inputs.DOWN_P1 || inputs.DOWN_P2;
        bool _start = inputs.START;
        if (_start)
        {
            return selected;
        }
        if (_up)
        {
            if (selected > 1)
            {
                selected -= 1;
                displaySelectMenu(items, num_items, selected);
            }
        }
        else if (_down)
        {
            if (selected < num_items)
            {
                selected += 1;
                displaySelectMenu(items, num_items, selected);
            }
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Serial connection started");

    // Define your display layout here, e.g. 1/8 step, and optional SPI inputs begin(row_pattern, CLK, MOSI, MISO, SS) //display.begin(8, 14, 13, 12, 4);
    display.begin(32);
    display.setFastUpdate(true);

    display.clearDisplay();
    display.setTextColor(c_cyan);
    display.setCursor(2, 0);
    display.print("Snake");
    display_update_enable(true);
    delay(2000);
    Serial.println("finished setup");

    snake_game = new SnakeGame(MATRIX_WIDTH, MATRIX_HEIGHT, display, inputs);

    selected_game = selectMenuItem(game_strings, num_games);
    selected_players = selectMenuItem(player_strings, num_player_options);

    if (selected_game == 1)
    {
        // set up Snake
        snake_game->setPlayers(selected_players);
        Serial.println("players were set");
    }
    else if (selected_game == 1)
    {
        // set up Tetris
    }
}

void loop()
{
    snake_game->loopGame();
}

// void displayFPS()
// {
//     display.setTextColor(c_white);
//     display.setCursor(1, 1);
//     display.setFont();
//     display.print(fps);
// }

// void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB) {
//   uint16_t text_length = text.length();
//   display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
//   display.setTextSize(1);
//   display.setRotation(0);
//   display.setTextColor(display.color565(colorR, colorG, colorB));

//   // Asuming 5 pixel average character width
//   for (int xpos = MATRIX_WIDTH; xpos > -(MATRIX_WIDTH + text_length * 5); xpos--) {
//     display.setTextColor(display.color565(colorR, colorG, colorB));
//     display.clearDisplay();
//     display.setCursor(xpos, ypos);
//     display.println(text);
//     delay(scroll_delay);
//     yield();

//     // This might smooth the transition a bit if we go slow
//     // display.setTextColor(display.color565(colorR/4,colorG/4,colorB/4));
//     // display.setCursor(xpos-1,ypos);
//     // display.println(text);

//     delay(scroll_delay / 5);
//     yield();
//   }
// }

// unsigned long ms_current = 0;
// unsigned long ms_previous = 0;
// unsigned long ms_wait_to_update = 100;
// unsigned long next_frame = 0;

// ms_current = millis();
// if ((ms_current - ms_previous) > ms_wait_to_update) {
//   up_touch = touchRead(32);
//   down_touch = touchRead(33);
//   left_touch = touchRead(27);
//   right_touch = touchRead(12);

//   if (up_touch < touch_threshold) {
//     up_text = "Up";
//   } else {
//     up_text = "";
//   }
//   if (left_touch < touch_threshold) {
//     left_text = "Left";
//   } else {
//     left_text = "";
//   }
//   if (right_touch < touch_threshold) {
//     right_text = "Right";
//   } else {
//     right_text = "";
//   }
//   if (down_touch < touch_threshold) {
//     down_text = "Down";
//   } else {
//     down_text = "";
//   }
//   display.clearDisplay();
//   display.setTextColor(c_cyan);
//   display.setCursor(26, 1);
//   display.print(up_text);
//   display.setCursor(1, 28);
//   display.print(left_text);
//   display.setCursor(33, 28);
//   display.print(right_text);
//   display.setCursor(20, 58);
//   display.print(down_text);
//   ms_previous = ms_current;
// }

// if (next_frame < ms_current) {
//   //next_frame = patterns.drawFrame() + ms_current;
//   next_frame = ms_current + 1;
//   display.showBuffer();
// }

// check for pause
// if (last_temp_state != temp_state) {
//   last_debounce_time = millis();
// }
// if (millis() - last_debounce_time > 50) {
//   if (pause_button_state != temp_state) {
//     pause_button_state = temp_state;
//     if (pause_button_state == LOW) {
//       paused = !paused;
//       Serial.print("Paused: ");
//       Serial.println(paused);
//       display.clearDisplay();
//       renderSnake();
//       renderApple();
//       ms_previous = millis();
//     }
//   }
// }