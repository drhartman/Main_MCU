#ifndef __TIMER0_H__
#define __TIMER0_H__

#define TIMER0_ENABLE_INTS          true
#define TIMER0_PERIODIC             true
#define TIMER0_PRIORITY             0

#define MICRO_SEC_12P5              1000


void timer0Init(uint32_t clkCycles, bool periodic, bool enInts, uint8_t priority);

#endif
