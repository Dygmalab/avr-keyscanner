#include "ringbuf.h"

static struct {
    uint8_t start;
    uint8_t count;
    uint8_t buf[64];
} _ring = { 0, 0, { 0 }};

void ringbuf_append(uint8_t value) {
    if (_ring.count < sizeof(_ring.buf)) {
        _ring.buf[(_ring.start + _ring.count++) % sizeof(_ring.buf)] = value;
    }
}

uint8_t ringbuf_size(void) {
    return _ring.count;
}

bool ringbuf_empty(void) {
    return _ring.count ==0;
}

uint8_t ringbuf_pop(void) {
    if (__builtin_expect(_ring.count == 0, EXPECT_FALSE)) {
        return 0;
    }

    uint8_t result = _ring.buf[_ring.start];
    _ring.count--;
    _ring.start++;
    _ring.start %= sizeof(_ring.buf);
    return result;
}
