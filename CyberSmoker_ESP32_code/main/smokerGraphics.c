#include "smokerGraphics.h"
#include "cyber_smoker_logo.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define BACKGROUND_COLOR WHITE
#define THERMOMETER_MAX_SMOKER 500
#define THERMOMETER_MIN_SMOKER 0
#define THERMOMETER_MAX_MEAT 500
#define THERMOMETER_MIN_MEAT 0
#define THERMOMETER_RADIUS 50
#define THERMOMETER_WIDTH 10
#define TEMP_STRING_BUFFER_SIZE 40

#define PI 3.14159265
#define D2R(INPUT) (double) (INPUT  * PI / 180)

const angle_t base_keepout = 30;
const angle_t angle_range = 360 - 2 * base_keepout; 

typedef struct {
    char* label;
    coord_t x_pos;
    coord_t y_pos;
    temperature_t min;
    temperature_t max;
    temperature_t goal_temp;
    temperature_t current_temp;
} thermometer_t;

thermometer_t meat_thermometer = {
    "MEAT", 
    3*LCD_W/4, LCD_H/2,
    THERMOMETER_MIN_MEAT, THERMOMETER_MAX_MEAT,
    0,
    0
};

thermometer_t smoker_thermometer = {
    "SMOKER", 
    LCD_W/4, LCD_H/2,
    THERMOMETER_MIN_SMOKER, THERMOMETER_MAX_SMOKER,
    0,
    0
};


void draw_temp_string(coord_t x_pos, coord_t y_pos, temperature_t temperature){
    char buffer[TEMP_STRING_BUFFER_SIZE];
    uint8_t string_len = sprintf(buffer, "%d", (int)temperature);
    x_pos = x_pos - (string_len *LCD_CHAR_W * LCD_FONT_SIZE / 2);
    y_pos = y_pos - (LCD_CHAR_H * LCD_FONT_SIZE / 2);
    lcd_drawString(x_pos, y_pos, buffer, BLACK);
}

void draw_title_string(coord_t x_pos, coord_t y_pos, char* title_string){
    size_t string_length = strlen(title_string);
    x_pos = x_pos - (string_length * LCD_CHAR_W * LCD_FONT_SIZE / 2);
    y_pos = y_pos + (THERMOMETER_RADIUS  * 0.85);
    lcd_drawString(x_pos, y_pos, title_string, BLACK);
}


//draws circular thermometers centered at (xpos, ypos) with given color and a given label
void draw_thermomometer(thermometer_t thermometer, color_t color){
    coord_t x1 = thermometer.x_pos; //center point
    coord_t y1 = thermometer.y_pos; //center point
    coord_t x2;
    coord_t y2;
    coord_t x3;
    coord_t y3;

    //draw circle one with radius 
    lcd_fillCircle(x1, y1, THERMOMETER_RADIUS, color);
    //draw inner circile
    lcd_fillCircle(x1, y1, THERMOMETER_RADIUS - THERMOMETER_WIDTH, BACKGROUND_COLOR);
    //draw base_keepout triangle
    coord_t bkx = x1 - (THERMOMETER_RADIUS * sqrtf(2) * sin(D2R(base_keepout)));
    coord_t bky = y1 + (THERMOMETER_RADIUS * sqrtf(2) * cos(D2R(base_keepout)));
    lcd_fillTriangle(x1, y1, x1, y1 + THERMOMETER_RADIUS, bkx, bky, BACKGROUND_COLOR);
    //draw blocks and triangle
    angle_t current_angle = base_keepout + ((angle_range * thermometer.current_temp) / (thermometer.max - thermometer.min));
    x2 = x1;
    y2 = y1 + sqrtf(2) * THERMOMETER_RADIUS;
    if(current_angle <= 270){
        //drawblockQ4
        lcd_fillRect2(x1, y1, x1 + THERMOMETER_RADIUS, y1 + THERMOMETER_RADIUS, BACKGROUND_COLOR);
        //set x2y2
        x2 = x1 + sqrtf(2) * THERMOMETER_RADIUS;
        y2 = y1;
    }
    if(current_angle <= 180){
        //drawblockQ3
        lcd_fillRect2(x1, y1, x1 + THERMOMETER_RADIUS, y1 - THERMOMETER_RADIUS, BACKGROUND_COLOR);
        //set x2y2
        x2 = x1;
        y2 = y1 - sqrtf(2) * THERMOMETER_RADIUS;
    }
    if(current_angle <= 90){
        //drawblockQ2
        lcd_fillRect2(x1, y1, x1 - THERMOMETER_RADIUS, y1 - THERMOMETER_RADIUS, BACKGROUND_COLOR);
        //set x2y2
        x2 = x1 - sqrtf(2) * THERMOMETER_RADIUS;
        y2 = y1;
    }
    x3 = x1 + (THERMOMETER_RADIUS * sqrtf(2) * cos(D2R(-1 *(current_angle + 90))));
    y3 = y1 - (THERMOMETER_RADIUS * sqrtf(2) * sin(D2R(-1 *(current_angle + 90))));
    lcd_fillTriangle(x1, y1, x2, y2, x3, y3, BACKGROUND_COLOR);

    //draw little black tip at temp
    //draw string giving exact temp
    draw_temp_string(x1, y1, thermometer.current_temp);
   
    //draw title string (meat or smoker)
    draw_title_string(x1, y1, thermometer.label);

}

void display_bison_logo_screen(){
    lcd_fillScreen(DARK_GRAY);
    lcd_drawBitmap(0, 0, &cyber_smoker_logo_320x240, CYBER_SMOKER_LOGO_WIDTH, CYBER_SMOKER_LOGO_HEIGHT, WHITE);
}

void smokerGraphics_draw_init_screen(){
    lcd_init();
    lcd_frameEnable();
    display_bison_logo_screen();
    lcd_writeFrame();
}


void smokerGraphics_update_values(temperature_t set_temp, temperature_t smoker_temp, temperature_t meat_temp, fsm_dial_state_t dial_state, bool wifi_state){
    lcd_fillScreen(WHITE);
    meat_thermometer.current_temp = meat_temp;
    smoker_thermometer.goal_temp = set_temp;
    smoker_thermometer.current_temp = smoker_temp;

    // Draw set temperature string
    char buffer[TEMP_STRING_BUFFER_SIZE];
    if(dial_state == FSM_DIAL_LOCKED){
        sprintf(buffer, "DUTY CYCLE:%d -LOCK", (int)set_temp); // TEMPORARY CHANGE
    }else{
        sprintf(buffer, "DUTY CYCLE:%d", (int)set_temp);  // TEMPORARY CHANGE
    }
    lcd_drawString(4, 4 * LCD_CHAR_H, buffer, BLACK);

    // Draw Wifi Connected / Disconnected
    if (wifi_state) {
        lcd_drawString(4, 4, "WiFi: CONNECTED", BLACK);
    } else {
        lcd_drawString(4, 4, "WiFi: DISCONNECTED", RED);
    }

    // Draw Thermometers
    draw_thermomometer(meat_thermometer, RED);
    draw_thermomometer(smoker_thermometer, RED);
    lcd_writeFrame();
}