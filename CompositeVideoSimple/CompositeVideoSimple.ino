//code by bitluni (send me a high five if you like the code)

#include "CompositeGraphics.h"
#include "Image.h"
#include "CompositeOutput.h"

#include "luni.h"
#include "font6x8.h"

//PAL MAX, half: 324x268 full: 648x536
//NTSC MAX, half: 324x224 full: 648x448
const int XRES = 324;
const int YRES = 224;

//Graphics using the defined resolution for the backbuffer
CompositeGraphics graphics(XRES, YRES);
//Composite output using the desired mode (PAL/NTSC) and twice the resolution. 
//It will center the displayed image automatically
CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);

//image and font from the included headers created by the converter. Each iamge uses its own namespace.
Image<CompositeGraphics> luni0(luni::xres, luni::yres, luni::pixels);

//font is based on ASCII starting from char 32 (space), width end height of the monospace characters. 
//All characters are staored in an image vertically. Value 0 is background.
Font<CompositeGraphics> font(6, 8, font6x8::pixels);

#include <soc/rtc.h>



void setup()
{
  //highest clockspeed needed
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);

  Serial.begin(115200);
  
  //initializing DMA buffers and I2S
  composite.init();
  //initializing graphics double buffer
  graphics.init();
  //select font
  graphics.setFont(font);

  //running composite output pinned to first core
  xTaskCreatePinnedToCore(compositeCore, "c", 1024, NULL, 1, NULL, 0);
  //rendering the actual graphics in the main loop is done on the second core by default
}

void compositeCore(void *data)
{
  while (true)
  {
    //just send the graphics frontbuffer whithout any interruption 
    composite.sendFrameHalfResolution(&graphics.frame);
  }
}

void draw()
{
  //clearing background and starting to draw
  graphics.begin(0);
  //drawing an image
  luni0.draw(graphics, 200, 10);

  //drawing a frame
  //graphics.fillRect(0, 0, 324, 224, 20);
  //graphics.rect(0, 0, 324, 224, 20);
  graphics.rect(27, 18, 160, 30, 20);
  

  //setting text color, transparent back ground
  graphics.setTextColor(50);
  //text starting position
  graphics.setCursor(30, 20);
  //printing some lines of text
  graphics.print("hello!");
  graphics.print(" free memory: ");
  graphics.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  graphics.print("\nrendered frame: ");
  static int frame = 0;
  graphics.print(frame, 10, 4); //base 10, 6 characters 
  graphics.print("\n        in hex: ");
  graphics.print(frame, 16, 4);
  frame++;

  //drawing some lines
  for(int i = 0; i <= 100; i++)
  {
    graphics.line(50, i + 60, 50 + i, 160, i / 2);
    graphics.line(150, 160 - i, 50 + i, 60, i / 2);
  }
  
  //draw single pixel
  graphics.dot(20, 190, 10);
  
  //finished drawing, swap back and front buffer to display it
  graphics.end();
}

void loop()
{
  draw();
  //delay(1000);
  Serial.println(ESP.getFreeHeap());
}


