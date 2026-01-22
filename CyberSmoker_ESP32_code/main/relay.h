#ifndef RELAY_H
#define RELAY_H
#include <stdint.h>
#include "driver/gpio.h"
#include <stdbool.h>

void relay_init(gpio_num_t relay_num);

void relay_init_pwm(gpio_num_t gpio_num, uint8_t pwm_duty_cycle);

void relay_toggle(gpio_num_t relay_num, bool state);

void relay_set_duty_cycle(uint8_t duty_cycle);

#endif