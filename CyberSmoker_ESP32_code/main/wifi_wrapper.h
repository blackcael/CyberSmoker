#ifndef WIFI_WRAPPER_H
#define WIFI_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "temperature.h"

void wifi_wrapper_init();

void wifi_wrapper_publish_data(temperature_t meat_temp, temperature_t ambient_temp, temperature_t set_temp, uint8_t pellet_level);

void wifi_wrapper_register_turn_off_cmd(void(*cmd)(void));

void wifi_wrapper_register_turn_on_cmd(void(*cmd)(void));

void wifi_wrapper_register_inc_temp_cmd(void(*cmd)(void));

void wifi_wrapper_register_dec_temp_cmd(void(*cmd)(void));

bool wifi_wrapper_get_connection_status(void);


#endif
