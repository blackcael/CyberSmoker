#include "PID.h"
#include "esp_timer.h"

float pid_kp = 1.5;
float pid_ki = 0.05;
float pid_kd = 5.0;
int8_t pid_response_min = 15;
int8_t pid_response_max = 75;

int64_t last_time_us;
int64_t sample_period_us;

temperature_t error;
uint16_t error_sum;
temperature_t prev_error;

void pid_init(void){
    error_sum = 0;
    prev_error = 0;
    last_time_us = esp_timer_get_time();
}

int8_t proportional_response(){
    return pid_kp * error;
}

int8_t derivative_response(){
    return pid_kd * ((error - prev_error) / (sample_period_us));
}

int8_t integral_response(){
    return pid_ki * (error_sum + (0.5 * sample_period_us * (error + prev_error)));
}

int8_t constrain(int8_t response){
    if(response < pid_response_min){
        return pid_response_min;
    }else if(response > pid_response_max){
        return pid_response_max;
    }
    return response;
}

int8_t pid_calculate_duty_cycle(temperature_t current_temperature, temperature_t set_temperature){
    // Calculate Error
    error = set_temperature - current_temperature;
    error_sum += error;
    // Calculate time difference
    int64_t current_time_us = esp_timer_get_time();
    sample_period_us = current_time_us - last_time_us;
    last_time_us = current_time_us;
    // Calculate response
    int8_t response = proportional_response() + derivative_response() + integral_response();
    
    response = constrain(response);
    //update pre error
    prev_error = error;
    return response;
}