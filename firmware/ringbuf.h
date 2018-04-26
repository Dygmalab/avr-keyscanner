#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

void ringbuf_append(uint8_t value);
bool ringbuf_empty(void);
uint8_t ringbuf_pop(void);
void ringbuf_pop_to(uint8_t *bufptr, uint8_t size);
uint8_t ringbuf_size(void);
