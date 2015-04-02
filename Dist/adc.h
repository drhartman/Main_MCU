#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>
#include "lm4f120h5qr.h"

#define POT_LEFT  1

uint32_t GetADCval(uint32_t Channel);
void initializeADC(void);

#endif
