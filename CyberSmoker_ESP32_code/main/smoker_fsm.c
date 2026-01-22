#include "smoker_fsm.h"

#include "smokerConfig.h"
#include "smokerGraphics.h"
#include "thermometer.h"
#include "relay.h"
#include "rotary_encoder.h"
#include "wifi_wrapper.h"
#include "hw.h"
#include "PID.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#define SMOKER_PERIOD_MS 250

#define SET_TEMP_CONSTRAIN_UPPER 100
#define SET_TEMP_CONSTRAIN_LOWER 0

#define DEFAULT_AUGAR_PWM_DUTY_CYCLE 25

#define SET_TEMP_STARTING_VAL 25
#define SET_TEMP_INC_DEC_VAL 5

static const char *TAG = "SMOKER_FSM";

temperature_t meat_temp;
temperature_t ambient_temp;
temperature_t set_temp;

int32_t rotary_encoder_value = 0;
fsm_dial_state_t dial_state = FSM_DIAL_UNLOCKED;

void smoker_fsm_init(void) {
    // init default value variables
    set_temp = SET_TEMP_STARTING_VAL;

    // init helper functions
    void inc_set_temp(void){
        set_temp += SET_TEMP_INC_DEC_VAL;
    }
    wifi_wrapper_register_inc_temp_cmd(inc_set_temp);

    void dec_set_temp(void){
        set_temp -= SET_TEMP_INC_DEC_VAL;
    }
    wifi_wrapper_register_dec_temp_cmd(dec_set_temp);

    void inc_set_temp_check_lock(void){
        if (dial_state == FSM_DIAL_UNLOCKED) {
            inc_set_temp();
        }
    }

    void dec_set_temp_check_lock(void){
        if (dial_state == FSM_DIAL_UNLOCKED) {
            dec_set_temp();
        }
    }

    void toggle_dial_state(void){
        if (dial_state == FSM_DIAL_UNLOCKED) {
            dial_state = FSM_DIAL_LOCKED;
        } else {
            dial_state = FSM_DIAL_UNLOCKED;
        }
    }

    // Init two relays
    relay_init(HW_RELAY_1);
    relay_init_pwm(HW_RELAY_1, DEFAULT_AUGAR_PWM_DUTY_CYCLE);
    relay_init(HW_RELAY_2);
    relay_init(HW_RELAY_3);

    // Init rotary encoder
    rotary_encoder_init();
    rotary_encoder_register_inc_function(inc_set_temp_check_lock);
    rotary_encoder_register_dec_function(dec_set_temp_check_lock);
    rotary_encoder_register_btn_press_function(toggle_dial_state);

    // Init thermometer
    thermometer_init();

    // Init Display
    smokerGraphics_draw_init_screen();

    // Init wifi and wifi functions
    wifi_wrapper_init();

    void turn_smoker_on(void){
        ESP_LOGI(TAG, "Turning smoker on");
        relay_toggle(HW_RELAY_1, true); // Assuming HW_RELAY_1 is the augar relay
        relay_toggle(HW_RELAY_2, true); // Assuming HW_RELAY_2 is the power relay
        relay_toggle(HW_RELAY_3, true); // Assuming HW_RELAY_3 is the fan relay
    }
    wifi_wrapper_register_turn_on_cmd(turn_smoker_on);

    void turn_smoker_off(void){
        ESP_LOGI(TAG, "Turning smoker off");
        relay_toggle(HW_RELAY_1, false); // Assuming HW_RELAY_1 is the augar relay
        relay_toggle(HW_RELAY_2, false); // Assuming HW_RELAY_2 is the power relay
        relay_toggle(HW_RELAY_3, false); // Assuming HW_RELAY_3 is the fan relay
    }
    wifi_wrapper_register_turn_off_cmd(turn_smoker_off);
    wifi_wrapper_register_dec_temp_cmd(dec_set_temp);

    // Init PID
    // pid_init();
}

temperature_t set_temperature_constrain(temperature_t temp){
    if (temp < SET_TEMP_CONSTRAIN_LOWER){
        return (temperature_t)SET_TEMP_CONSTRAIN_LOWER;
    }
    else if(temp > SET_TEMP_CONSTRAIN_UPPER){
        return (temperature_t)SET_TEMP_CONSTRAIN_UPPER;
    }
    else return temp;
}


void smoker_fsm_run(void) {
    // Main loop for the smoker FSM
    while (1) {
        // Read temperatures
        meat_temp = thermometer_read_meat();
        ambient_temp = thermometer_read_ambient();

        // Log temperatures
        ESP_LOGI("SMOKER_FSM", "Meat Temp: %.2f, Ambient Temp: %.2f", meat_temp, ambient_temp);

        // Calculate PWM duty cycle based on temperature
        // uint8_t duty_cycle = pid_calculate_duty_cycle(ambient_temp, set_temp);

        // Check rotary encoder
        rotary_encoder_tick();

        // TEMPORARY - instead of set temp being set temp, we are going to use it as duty cycle for now
        set_temp = set_temperature_constrain(set_temp);
        relay_set_duty_cycle((uint8_t)set_temp);

        // Poll Wifi Connection Status
        bool wifi_state = wifi_wrapper_get_connection_status();

        // Update display with new temperature values
        smokerGraphics_update_values(set_temp, ambient_temp, meat_temp, dial_state, wifi_state);

        // Publish data to MQTT
        wifi_wrapper_publish_data(meat_temp, ambient_temp, set_temp, 0); // Set temp and pellet level as needed

        // Simulate some delay for the FSM loop
        vTaskDelay(pdMS_TO_TICKS(SMOKER_PERIOD_MS));
    }
}