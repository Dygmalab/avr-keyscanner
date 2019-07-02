#pragma once
#include <stdbool.h>

extern volatile uint8_t new_key_state;
extern volatile uint8_t key_state[5];

void keyscanner_init(void);
bool keyscanner_main(void);
void keyscanner_timer1_init(void);

