#pragma once

#include <stdint.h>
#include "keyscanner.h"

// No debouncing, for debugging purposes
typedef struct {
    uint8_t state;  // debounced state
} debounce_t;

static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {

    uint8_t changes = sample ^ debouncer->state;
    if (changes)
        debouncer->state = sample;
    return changes;
}
