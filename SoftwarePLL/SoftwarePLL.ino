#include "MovingAverage.h"
#include "RX5808.h"
#include "LM1881.h"
#include "CompositeGraphics.h"
#include "CompositeOutput.h"
#include "DigitalFilters.h"
#include "C:\Users\Alessandro\.platformio\packages\framework-espidf\components\soc\esp32\i2c_apll.h"
#include "C:\Users\Alessandro\.platformio\packages\framework-espidf\components\soc\esp32\i2c_bbpll.h"
#include "C:\Users\Alessandro\.platformio\packages\framework-espidf\components\soc\esp32\i2c_rtc_clk.h"


bool ticktok = false;

const int XRES = 324;
const int YRES = 224;

//const int XRES = 200;
//const int YRES = 200;


uint32_t lastTimerAdjustment = 0;
uint32_t TimerAdjustmentInterval = 100;


void VSYNC_ISR_ADJUST();

void AdjustTimer() {
  uint32_t currTime = millis();
  if ((currTime - lastTimerAdjustment) >= TimerAdjustmentInterval) {
    VSYNC_ISR_ADJUST();
    Serial.println(".");
    lastTimerAdjustment = currTime;
  }
}

//Graphics using the defined resolution for the backbuffer
CompositeGraphics graphics(XRES, YRES);
//Composite output using the desired mode (PAL/NTSC) and twice the resolution.
//It will center the displayed image automatically
CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);
//font is based on ASCII starting from char 32 (space), width end height of the monospace characters.
//All characters are staored in an image vertically. Value 0 is background.
Font<CompositeGraphics> font(8, 8, font8x8::pixels);

#include <soc/rtc.h>

//void compositeCore(void *data)
//{
//  while (true)
//  {
//    //just send the graphics frontbuffer whithout any interruption
//    composite.sendFrameHalfResolution(&graphics.frame);
//  }
//}

void VSYNC_ISR_ADJUST();

void draw()
{
  //clearing background and starting to draw
  graphics.begin(0);
  //drawing an image


  //drawing a frame
  //graphics.fillRect(0, 0, 324, 224, 20);
  graphics.rect(0, 0, 320, 220, 20);
  //graphics.rect(27, 18, 160, 30, 20);


  //setting text color, transparent back ground
  graphics.setTextColor(50);
  //text starting position
  graphics.setCursor(30, 20);
  //printing some lines of text
  graphics.print((char *)"hello!");
  graphics.print((char *)" free memory: ");
  graphics.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  graphics.print((char *)"\nrendered frame: ");
  static int frame = 0;
  graphics.print(frame, 10, 4); //base 10, 6 characters

  graphics.print((char *)"\n\nHSYNC interval: ");
  graphics.print((int)HSYNCinterval);

  graphics.print((char *)"\nVSYNC interval: ");
  graphics.print((int)VSYNCinterval);

  frame++;

  //drawing some lines
  //for (int i = 0; i <= 100; i++)
  //  {
  //    graphics.line(50, i + 60, 50 + i, 160, i / 2);
  //    graphics.line(150, 160 - i, 50 + i, 60, i / 2);
  //  }
  //
  //  //draw single pixel
  //  graphics.dot(20, 190, 10);

  //finished drawing, swap back and front buffer to display it
  graphics.end();
}

void compositeCore(void *data)
{
  while (true)
  {
    //just send the graphics frontbuffer whithout any interruption
    composite.sendFrameHalfResolution(&graphics.frame);
  }
}

void setup() {

  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);

  Serial.begin(115200);
  delay(100);
  InitSPI();
  delay(100);
  initAnalogSwitches();

  //initializing DMA buffers and I2S
  composite.init();
  //initializing graphics double buffer
  graphics.init();
  //select font
  graphics.setFont(font);

  //running composite output pinned to first core

  initLM1881();
  
  //rendering the actual graphics in the main loop is done on the second core by default

  //running composite output pinned to first core
  xTaskCreatePinnedToCore(compositeCore, "c", 1024, NULL, 1, NULL, 0);
  //rendering the actual graphics in the main loop is done on the second core by default
  //delay(3000);
  //rtc_clk_apll_enable(1, 0, 0, 6, 0);


}

void loop() {
  draw();
  RXcheckIfUpdateReq();
  //AdjustTimer();

  //delay(5000);
  //rtc_clk_apll_enable(1, 0, 0, 6, 0);
  //VSYNCFreqError = AVG_VSYNC_FREQ_ERROR.update(VSYNCinterval - I2S_VSYNC_INTERVAL);
  //ADJUST_APPL_FREQ();
//  Serial.print(I2S_VSYNC_INTERVAL);
//  Serial.print(" ");
//  Serial.println(VSYNCinterval);


  //digitalWrite(OSDswitch, digitalRead(progPin));


  //  if (digitalRead(progPin)) {
  //    rtc_clk_apll_enable(1, 0, 50, 6, 0);
  //  } else {
  //    rtc_clk_apll_enable(1, 0, 0, 6, 0);
  //  }

  //  for (uint8_t i = 0; i < 100; i++) {
  //    //rtc_clk_apll_enable(1, 0, i, 6, 0);
  //    //I2C_WRITEREG_MASK_RTC(I2C_APLL, I2C_APLL_DSDM1, (unsigned char)i);
  //    delay(50);
  //  }

  //  if (ticktok) {
  //    composite.samplesLine = 848;
  //    composite.samplesActive = composite.samplesLine - composite.samplesSync - composite.samplesBlank - composite.samplesBack;
  //
  //    composite.targetXres = XRES*2 < composite.samplesActive ? XRES*2 : composite.samplesActive;
  //
  //    composite.samplesVSyncShort = samplesPerMicro * composite.properties.shortVSyncMicros + 0.5;
  //    composite.samplesBlackLeft = (composite.samplesActive - composite.targetXres) / 2;
  //    composite.samplesBlackRight = composite.samplesActive - composite.targetXres - composite.samplesBlackLeft;
  //    lineoffset = 0;
  //  } else {
  //    composite.samplesLine = 800;
  //    composite.samplesActive = composite.samplesLine - composite.samplesSync - composite.samplesBlank - composite.samplesBack;
  //
  //    composite.targetXres = XRES*2 < composite.samplesActive ? XRES*2 : composite.samplesActive;
  //
  //    composite.samplesVSyncShort = samplesPerMicro * composite.properties.shortVSyncMicros + 0.5;
  //    composite.samplesBlackLeft = (composite.samplesActive - composite.targetXres) / 2;
  //    composite.samplesBlackRight = composite.samplesActive - composite.targetXres - composite.samplesBlackLeft;
  //  }
  //  ticktok = !ticktok;



}
