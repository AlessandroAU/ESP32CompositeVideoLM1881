
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


uint32_t BackPorchPinCount;

uint32_t Timer1LastTriggered;

int32_t PhaseError = 0;
int32_t PhaseErrorFiltered = 0;

bool EvenOddField = false; //false = even field.

static SemaphoreHandle_t timer_sem;
static TaskHandle_t SendFrame_handle;



int32_t TimerADJvalue = 0;

uint8_t i = 0;
uint8_t j = 0;

uint32_t VSYNCtimerCounter() {
}

LowPassFilter lpf1_VSYNC(0.01666, 1 * 2 * 3.141);




void IRAM_ATTR HSYNC_ISR() {
  portENTER_CRITICAL_ISR(&mux);
  uint32_t currCount = xthal_get_ccount();


  HSYNCinterval = currCount - HSYNClastCount;
  HSYNClastCount = currCount;
  //Serial.println(HSYNCinterval);
  portEXIT_CRITICAL_ISR(&mux);

}

void IRAM_ATTR FID() {
  EvenOddField = digitalRead(OddEvenFieldPin);
}

void IRAM_ATTR VSYNC_ISR() {
  portENTER_CRITICAL_ISR(&mux);

  if (EvenOddField) {
    uint32_t currCount = xthal_get_ccount();
    VSYNCinterval = lpf1_VSYNC.update(currCount - VSYNClastCount);
    VSYNCFreqError = lpf1_FreqError.update(VSYNCinterval - I2S_VSYNC_INTERVAL);
    //VSYNCFreqError = (VSYNCinterval - I2S_VSYNC_INTERVAL);
    //VSYNCPhaseError
    VSYNClastCount = currCount;


    //VSYNCinterval = ;

  }

  //timerWrite(timer, 0);
  //  timerAlarmWrite(timer, floor(VSYNCinterval / (3.0)), true);
  //  timerAlarmEnable(timer);
  //Serial.println(VSYNCinterval);
  portEXIT_CRITICAL_ISR(&mux);

}

void startVSYNCtimerTask();



void initLM1881() {
  pinMode(HsyncPin, INPUT);
  pinMode(VsyncPin, INPUT);
  pinMode(OddEvenFieldPin, INPUT);
  pinMode(BackPorchPin, INPUT);

  ////attach interrupts//////
  attachInterrupt(digitalPinToInterrupt(HsyncPin), HSYNC_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(VsyncPin), VSYNC_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(OddEvenFieldPin), FID, CHANGE);

  //startVSYNCtimerTask();

}


void IRAM_ATTR VSYNC_TIMER_ISR() {
  //
  //  composite.sendFrameHalfResolution(&graphics.frame);
  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  Timer1LastTriggered = xthal_get_ccount();
  //  if (digitalRead(OddEvenFieldPin)) {
  //    PhaseError =  Timer1LastTriggered - VSYNClastCount;
  //    //int32_t TimerADJvalue = 0;
  //    PhaseErrorFiltered = (lpf1_PHASE.update(PhaseError)) / 3;
  //  }
  xSemaphoreGiveFromISR(timer_sem, &xHigherPriorityTaskWoken);
  if ( xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR(); // this wakes up sample_timer_task immediately

  }

  //    timerAlarmWrite(timer, (uint64_t)(VSYNCinterval)/4, false);
  //    timerAlarmEnable(timer);
  //
}

byte APLL_sdm0 = 0;
byte APLL_sdm1 = 58;
byte APLL_sdm2 = 6;

void IRAM_ATTR APLL_INC(long val) {

  if (APLL_sdm0 + val <= 255) {
    APLL_sdm0 = APLL_sdm0 + val;
  } else {
    int32_t Sum = val + APLL_sdm0;
    APLL_sdm0 = Sum % 256;
    int32_t Sum2 = APLL_sdm1 + floor(Sum / 256);
    if (Sum2 <= 255) {
      APLL_sdm1 = Sum2;
    } else {
      APLL_sdm1 = Sum2 % 256;
      APLL_sdm2 = APLL_sdm2 + floor(Sum2 / 256);
    }
  }
  //APLL_sdm0++;

  if (APLL_sdm2 >= 6 and APLL_sdm1 > 80) {
    APLL_sdm2 = 6;
    APLL_sdm1 = 80;
    APLL_sdm0 = 0;
  }

  rtc_clk_apll_enable(1, APLL_sdm0, APLL_sdm1, APLL_sdm2, 0);

  Serial.print(I2S_VSYNC_INTERVAL);
  Serial.print(" ");
  Serial.print(VSYNCinterval);
  Serial.print("    ");
  Serial.print((int32_t)VSYNCFreqError);
  Serial.print("    ");
  Serial.print((int32_t)VSYNCPhaseError);



  Serial.print("  +  ");
  Serial.print(APLL_sdm0);
  Serial.print(" ");
  Serial.print(APLL_sdm1);
  Serial.print(" ");
  Serial.println(APLL_sdm2);


}

void IRAM_ATTR APLL_DEC(long val) {
  if (APLL_sdm0 - val >= 0) {
    APLL_sdm0 = APLL_sdm0 - val;
  } else {
    int32_t Sum = APLL_sdm0 - val;
    APLL_sdm0 = Sum % 256;
    int32_t Sum2 = APLL_sdm1 + int(floor(Sum / 256.0));
    //    Serial.println(Sum);
    //    Serial.println(Sum2);
    //    Serial.println(APLL_sdm1);
    //    Serial.println(int(floor(Sum / 256.0)));

    if (Sum2 >= 0) {
      APLL_sdm1 = Sum2;
    } else {
      APLL_sdm1 = Sum2 % 256;
      APLL_sdm2 = APLL_sdm2 + int(floor(Sum2 / 256.0));
    }
  }

  if (APLL_sdm2 <= 6 and APLL_sdm1 < 40) {
    APLL_sdm2 = 6;
    APLL_sdm1 = 40;
    APLL_sdm0 = 0;
  }

  rtc_clk_apll_enable(1, APLL_sdm0, APLL_sdm1, APLL_sdm2, 0);

  Serial.print(I2S_VSYNC_INTERVAL);
  Serial.print(" ");
  Serial.print(VSYNCinterval);
  Serial.print("    ");
  Serial.print((int32_t)VSYNCFreqError);
  Serial.print("    ");
  Serial.print((int32_t)VSYNCPhaseError);



  Serial.print("  -  ");
  Serial.print(APLL_sdm0);
  Serial.print(" ");
  Serial.print(APLL_sdm1);
  Serial.print(" ");
  Serial.println(APLL_sdm2);
}


void IRAM_ATTR ADJUST_APPL_FREQ() {
  //  Serial.print(I2S_VSYNC_INTERVAL);
  //  Serial.print(" ");
  //  Serial.println(VSYNCinterval);
  //VSYNCFreqError = VSYNCinterval - I2S_VSYNC_INTERVAL;



  if (VSYNCFreqError > 10) {
    APLL_DEC(1);
  } else if (VSYNCFreqError < -10) {
    APLL_INC(1);
  } else {
    if (VSYNCPhaseError > 0) {
      APLL_DEC(1);
    } else if (VSYNCPhaseError < 0) {
      APLL_INC(1);
    }
  }


}


void IRAM_ATTR VSYNC_ISR_ADJUST() {

  if (digitalRead(OddEvenFieldPin)) {
    PhaseError =  VSYNClastCount - Timer1LastTriggered;
    //int32_t TimerADJvalue = 0;
    //    PhaseErrorFiltered = (lpf1_PHASE.update(PhaseError)) / 3;
  }

  if (PhaseErrorFiltered > 10000) {
    TimerADJvalue = -1000;
  } else if (PhaseErrorFiltered < -10000) {
    TimerADJvalue = 1000;
  }  else {
    TimerADJvalue = -PhaseErrorFiltered / 500;
    //TimerADJvalue = 0;
  }

  //  uint64_t Timer1Count = timerRead(timer);
  //  if (Timer1Count > 2000) {
  //    timerWrite(timer, Timer1Count - 1000);
  //  }

  //timerAlarmDisable(timer);
  //  timerAlarmWrite(timer, (VSYNCinterval / (3)) - TimerADJvalue, true);
  //  timerAlarmEnable(timer);

  Serial.println((VSYNCinterval / (3 * 2)));
  Serial.println(PhaseErrorFiltered);
  Serial.println(TimerADJvalue);
  //timerAlarmEnable(timer);
  //timerAttachInterrupt(timer, &VSYNC_TIMER_ISR, true);
}

void IRAM_ATTR SendFrame(void *param) {
  timer_sem = xSemaphoreCreateBinary();

  while (1) {
    xSemaphoreTake(timer_sem, portMAX_DELAY);

    composite.sendFrameHalfResolution(&graphics.frame);

  }
}


void startVSYNCtimerTask() {

  //attachInterrupt(digitalPinToInterrupt(SX127x_dio0), TXnbISR, RISING);

  //  SetMode(SX127X_STANDBY);
  //  setRegValue(SX127X_REG_DIO_MAPPING_1, SX127X_DIO0_TX_DONE, 7, 6);
  //
  xTaskCreatePinnedToCore(
    SendFrame,          /* Task function. */
    "SendFrame",        /* String with name of task. */
    4096,            /* Stack size in words. */
    NULL,             /* Parameter passed as input of the task */
    -1,                /* Priority of the task. */
    &SendFrame_handle,
    0);            /* Task handle. */

  timer = timerBegin(0, 2, true);
  timerAttachInterrupt(timer, &VSYNC_TIMER_ISR, true);
  timerAlarmWrite(timer, 607285, true);
  //timerAlarmEnable(timer);

}

//
//void IRAM_ATTR SX127xDriver::TimerTaskTX_ISRhandler(){
//  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//  if(RadioState==RADIO_IDLE){
//  xSemaphoreGiveFromISR(timer_sem, &xHigherPriorityTaskWoken);
//  if ( xHigherPriorityTaskWoken) {
//    SX127xDriver::TXstartMicros = micros();
//    portYIELD_FROM_ISR(); // this wakes up sample_timer_task immediately
//  }
//  }
//}
//
//

//
//void SX127xDriver::StopContTX(){
//  detachInterrupt(digitalPinToInterrupt(SX127x_dio0));
//  vTaskDelete(TimerTaskTX_handle);
//
//}
//
//void SX127xDriver::StartContTX() {
//
//  attachInterrupt(digitalPinToInterrupt(SX127x_dio0), TXnbISR, RISING);
//
//  SetMode(SX127X_STANDBY);
//  setRegValue(SX127X_REG_DIO_MAPPING_1, SX127X_DIO0_TX_DONE, 7, 6);
//
//  xTaskCreate(
//    TimerTaskTX,          /* Task function. */
//    "TimerTaskTX",        /* String with name of task. */
//    1000,            /* Stack size in words. */
//    NULL,             /* Parameter passed as input of the task */
//    10,                /* Priority of the task. */
//    &TimerTaskTX_handle);            /* Task handle. */
//
//    timer = timerBegin(0, 40, true);
//    timerAttachInterrupt(timer, &TimerTaskTX_ISRhandler, true);
//    timerAlarmWrite(timer, TXContInterval, true);
//    timerAlarmEnable(timer);
//}
//////////////////////////////////////////////////////////////////////////////////////////
