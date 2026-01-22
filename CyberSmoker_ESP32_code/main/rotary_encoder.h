#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdint.h>
#include <stdbool.h>

static volatile bool rotary_encoder_btn_flag = false;

void rotary_encoder_init();

int16_t rotary_encoder_get_count();

void rotary_encoder_reset_count();

void rotary_encoder_tick();

void rotary_encoder_register_inc_function(void(*func)(void));

void rotary_encoder_register_dec_function(void(*func)(void));

void rotary_encoder_register_btn_press_function(void(*func)(void));

#endif