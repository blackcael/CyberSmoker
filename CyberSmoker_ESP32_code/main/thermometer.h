#ifndef THERMOMETER_H
#define THERMOMETER_H

#include "temperature.h"
#include "driver/gpio.h"

void thermometer_init();

temperature_t thermometer_read_meat();

temperature_t thermometer_read_ambient();

#endif