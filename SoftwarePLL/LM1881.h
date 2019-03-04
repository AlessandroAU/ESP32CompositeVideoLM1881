#pragma once
#include "MovingAverage.h"
#include "DigitalFilters.h"

MovingAverage AVG_VSYNC_FREQ_ERROR(0.01f);


uint32_t HSYNCinterval = 0;
uint32_t VSYNCinterval = 0;
uint32_t HSYNClastCount;  //counts in CPU ticks from last pulse recv
uint32_t VSYNClastCount;

int32_t VSYNCPhaseError;

LowPassFilter3 lpf1_FreqError(0.05, 0.5 * 2 * 3.141);

int32_t VSYNCFreqError = 0;

void IRAM_ATTR ADJUST_APPL_FREQ();
