#ifndef PID_H
#define PID_H

//PID controller thinkering
#include <stdint.h>
#include "temperature.h"

void pid_init(void);

int8_t pid_calculate_duty_cycle(temperature_t current_temperature, temperature_t set_temperature);

#endif
