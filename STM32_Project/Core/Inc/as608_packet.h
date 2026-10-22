#ifndef AS608_PACKET_H
#define AS608_PACKET_H
#include <stdint.h>
#include <stddef.h>

/* ACK length includes the confirmation byte, payload and two checksum bytes. */
static inline uint8_t *as608_find_ack(uint8_t *data, size_t size,
                                    uint32_t address, size_t minimum_payload)
{
    for (size_t i = 0; i + 12 <= size; ++i) {
        uint8_t *p = data + i;
        if (p[0] != 0xef || p[1] != 0x01 || p[6] != 0x07) continue;
        uint32_t actual = ((uint32_t)p[2] << 24) | ((uint32_t)p[3] << 16) |
                          ((uint32_t)p[4] << 8) | p[5];
        size_t length = ((size_t)p[7] << 8) | p[8];
        if (actual != address || length < 3 ||
            length > size - i - 9) continue;
        if (p[9] == 0 && length < minimum_payload + 2) continue;
        uint16_t sum = 0;
        for (size_t j = 6; j < 9 + length - 2; ++j) sum += p[j];
        uint16_t received = ((uint16_t)p[9 + length - 2] << 8) | p[9 + length - 1];
        if (sum == received) return p;
    }
    return NULL;
}
#endif
