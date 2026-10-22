/* Freestanding regression: compile with clang --target=wasm32 and run in Node. */
#include "as608_packet.h"
static uint8_t b[64];
static void packet(uint32_t addr, unsigned payload) {
    for (unsigned i=0;i<64;i++) b[i]=0;
    b[0]=0xef;b[1]=1;b[2]=addr>>24;b[3]=addr>>16;b[4]=addr>>8;b[5]=addr;
    b[6]=7;b[8]=payload+2;
    uint16_t sum=7+b[8];
    b[9+payload]=sum>>8;b[10+payload]=sum;
}
int run_tests(void) {
    packet(0,1); if (!as608_find_ack(b,12,0,1)) return 1; /* embedded NUL address */
    if (as608_find_ack(b,11,0,1)) return 2;
    if (as608_find_ack(b,12,1,1)) return 3;
    b[11]++; if (as608_find_ack(b,12,0,1)) return 4;
    packet(0xffffffff,1); if (as608_find_ack(b,12,0xffffffff,5)) return 5;
    packet(0xffffffff,5); if (!as608_find_ack(b,16,0xffffffff,5)) return 6;
    b[8]=255; if (as608_find_ack(b,16,0xffffffff,1)) return 7;
    packet(0,1); b[9]=9;b[11]+=9; if (!as608_find_ack(b,12,0,5)) return 8;
    return 0;
}
