#ifndef SMOKERGRAPHICS_H_
#define SMOKERGRAPHICS_H_

#include "lcd.h"
#include "temperature.h"
#include "smokerConfig.h"

void smokerGraphics_draw_init_screen();

void smokerGraphics_update_values(temperature_t set_temp, temperature_t smoker_temp, temperature_t meat_temp, fsm_dial_state_t dial_state, bool wifi_state);


#endif