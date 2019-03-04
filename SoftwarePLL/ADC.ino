#include <driver/adc.h>
#include <driver/timer.h>


hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


#define ADC1 ADC1_CHANNEL_0
#define ADC2 ADC1_CHANNEL_6

#define ADCvbat ADC1_CHANNEL_4

int ADC1value;
int ADC2value;
int VbatValue;


void IRAM_ATTR readADCs() {

}

void ConfigureADC() {

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1, ADC_ATTEN_6db);
  adc1_config_channel_atten(ADC2, ADC_ATTEN_6db);
  adc1_config_channel_atten(ADCvbat, ADC_ATTEN_6db);

}

void InitADCtimer() {

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &readADCs, true);
  timerAlarmWrite(timer, 10000, true);
  timerAlarmEnable(timer);

}
