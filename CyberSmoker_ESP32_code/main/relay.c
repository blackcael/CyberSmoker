#include "relay.h"
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "RELAY";

#define RELAY_PWM_PERIOD_MS 60000

gpio_num_t pwm_relay;

static TimerHandle_t relay_timer;

uint8_t pwm_duty_cycle;

bool pwm_relay_on_flag = false;

void relay_init(gpio_num_t gpio_num)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Relay GPIO %d configured", gpio_num);
    } else {
        ESP_LOGE(TAG, "Failed to configure GPIO %d", gpio_num);
    }
}

void relay_init_pwm(gpio_num_t gpio_num, uint8_t def_duty_cycle){
    pwm_relay = gpio_num;
    pwm_duty_cycle = def_duty_cycle;
}

void relay_timer_callback(TimerHandle_t xTimer){
    relay_timer = NULL;
}

void start_relay_timer(int16_t period_ms){

    relay_timer = xTimerCreate("RelayTimer", pdMS_TO_TICKS(period_ms), pdTRUE, NULL, relay_timer_callback);
    if (relay_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create relay timer");
    }
    if (relay_timer != NULL) {
        xTimerStart(relay_timer, 0);
        ESP_LOGI(TAG, "Relay timer started with period %d ms", period_ms);
    }
}

void relay_toggle(gpio_num_t gpio_num, bool state)
{
    gpio_set_level(gpio_num, state ? 1 : 0);
    ESP_LOGI(TAG, "Relay on GPIO %d turned %s", gpio_num, state ? "ON" : "OFF");
}

void relay_set_duty_cycle(uint8_t duty_cycle)
{
    // Validate duty cycle and period
    if (duty_cycle > 100) {
        ESP_LOGE(TAG, "Invalid duty cycle or period");
        return;
    }

    // Calculate the on and off times
    uint32_t on_time = (duty_cycle * RELAY_PWM_PERIOD_MS) / 100;
    uint32_t off_time = RELAY_PWM_PERIOD_MS - on_time;

    // Timer / Toggle / Flag management
    if(!pwm_relay_on_flag){
        if (relay_timer == NULL) {
            relay_toggle(pwm_relay, true);
            pwm_relay_on_flag = true;
            start_relay_timer(on_time);
        }
    }else{
        if(relay_timer == NULL) {
            relay_toggle(pwm_relay, false);
            pwm_relay_on_flag = false;
            start_relay_timer(off_time);
        }
    }
    // Start the relay timer if not already started
    
}



// CURRENT RELAY PWM BREAKS EVERYTING, CREATES A STACK OVERFLOW, MUST FIX!