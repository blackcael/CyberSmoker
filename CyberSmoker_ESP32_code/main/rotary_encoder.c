#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/pcnt.h"
#include "esp_log.h"

#include "hw.h"
#include "rotary_encoder.h"


#define PCNT_UNIT PCNT_UNIT_0
#define PCNT_H_LIM_VAL 10000
#define PCNT_L_LIM_VAL -10000

#define MS_TO_US(x) ((x) * 1000)
#define BUTTON_DEBOUNCE_TIME_MS 500
#define DIAL_DEBOUNCE_TIME_MS 50

static const char *TAG = "PCNT_ENCODER";

void (*inc_function)(void) = NULL;
void (*dec_function)(void) = NULL;
void (*btn_press_function)(void) = NULL;

int16_t encoder_count = 0;
int16_t current_encoder_count = 0;

static volatile int64_t rotary_encoder_last_button_press_time_us = 0;

void rot_pcnt_init(){
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = HW_ROTARY_ENC_A,
        .ctrl_gpio_num = HW_ROTARY_ENC_B,
        .channel = PCNT_CHANNEL_0,
        .unit = PCNT_UNIT,
        .pos_mode = PCNT_COUNT_INC,   // Count up on rising edge if ctrl is high
        .neg_mode = PCNT_COUNT_DEC,   // Count down on falling edge if ctrl is high
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse if ctrl is low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep direction if ctrl is high
        .counter_h_lim = PCNT_H_LIM_VAL,
        .counter_l_lim = PCNT_L_LIM_VAL
    };

    pcnt_unit_config(&pcnt_config);

    // Filter out pulses that are too short
    pcnt_set_filter_value(PCNT_UNIT, pdMS_TO_TICKS(DIAL_DEBOUNCE_TIME_MS));
    pcnt_filter_enable(PCNT_UNIT);

    // Enable events on limits (optional)
    pcnt_event_enable(PCNT_UNIT, PCNT_EVT_H_LIM);
    pcnt_event_enable(PCNT_UNIT, PCNT_EVT_L_LIM);

    // Clear and start counter
    pcnt_counter_pause(PCNT_UNIT);
    pcnt_counter_clear(PCNT_UNIT);
    pcnt_counter_resume(PCNT_UNIT);

    ESP_LOGI(TAG, "PCNT initialized for encoder on GPIO %d and %d", HW_ROTARY_ENC_A, HW_ROTARY_ENC_B);
}

void IRAM_ATTR button_isr_handler(void *arg){
    int64_t current_time_us = esp_timer_get_time();
    if (current_time_us - rotary_encoder_last_button_press_time_us < MS_TO_US(BUTTON_DEBOUNCE_TIME_MS)) {
        return;  // Ignore if within debounce time
    }
    rotary_encoder_last_button_press_time_us = current_time_us;

    rotary_encoder_btn_flag = true;  // Set flag for main loop
}

void encoder_button_init(){
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << HW_ROTARY_ENC_BTN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE   // Trigger on falling edge
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);  // Only once in project
    gpio_isr_handler_add(HW_ROTARY_ENC_BTN, button_isr_handler, NULL);
}

void rotary_encoder_init(){
    rot_pcnt_init();
    encoder_button_init();
}

int16_t rotary_encoder_get_count() {
    pcnt_get_counter_value(PCNT_UNIT, &encoder_count);
    return encoder_count;
}

void rotary_encoder_tick(){
    int16_t new_count = rotary_encoder_get_count();
    if (new_count != current_encoder_count) {
        if (new_count > current_encoder_count) {
            if (inc_function) inc_function();
        } else {
            if (dec_function) dec_function();
        }
        current_encoder_count = new_count;
    }
    if (btn_press_function && rotary_encoder_btn_flag) {
        btn_press_function();
        rotary_encoder_btn_flag = false;
    }
}

void rotary_encoder_reset_count() {
    pcnt_counter_clear(PCNT_UNIT);
    encoder_count = 0;
    ESP_LOGI(TAG, "Encoder count reset to 0");
}

void rotary_encoder_register_inc_function(void(*func)(void)){
    inc_function = func;
}

void rotary_encoder_register_dec_function(void(*func)(void)){
    dec_function = func;
}

void rotary_encoder_register_btn_press_function(void(*func)(void)){
    btn_press_function = func;
}