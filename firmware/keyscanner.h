#pragma once

void keyscanner_init(void);
void keyscanner_main(void);
void keyscanner_record_state(void);
void keyscanner_ringbuf_update(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4);
void keyscanner_timer1_init(void);
