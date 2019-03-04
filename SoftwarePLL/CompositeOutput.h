#pragma once
#include "driver/i2s.h"
#include "DigitalFilters.h"
#include "LM1881.h"

volatile unsigned int lineoffset = 0;

double samplesPerSecond = 100000000.0 / 2.0 / 2.0 / 2.0;
double samplesPerMicro = samplesPerSecond * 0.000001;

uint32_t I2S_COUNTER_VSYNC = 0;
uint32_t I2S_COUNTER_VSYNC_PREV = 0;
uint32_t I2S_VSYNC_INTERVAL = 0;

LowPassFilter lpf1_I2S_VSYNC(0.01666, 0.5 * 2 * 3.141);





typedef struct
{
  float lineMicros;
  float syncMicros;
  float blankEndMicros;
  float backMicros;
  float shortVSyncMicros;
  float overscanLeftMicros;
  float overscanRightMicros;
  float syncVolts;
  float blankVolts;
  float blackVolts;
  float whiteVolts;
  short lines;
  short linesFirstTop;
  short linesOverscanTop;
  short linesOverscanBottom;
  float imageAspect;

  float syncTime_VSYNC1;
  float syncTime_VSYNC2;
  float syncTime_VSYNC3;

} TechProperties;

TechProperties PALProperties = {
  .lineMicros = 64,
  .syncMicros = 4.7,
  .blankEndMicros = 10.4,
  .backMicros = 1.65,
  .shortVSyncMicros = 2.35,
  .overscanLeftMicros = 1.6875,
  .overscanRightMicros = 1.6875,
  .syncVolts = -0.3,
  .blankVolts = 0.0,
  .blackVolts =  0.005,//specs 0.0,
  .whiteVolts = 0.7,
  .lines = 625,
  .linesFirstTop = 23,
  .linesOverscanTop = 9,
  .linesOverscanBottom = 9,
  .imageAspect = 4. / 3.
};

TechProperties NTSCProperties = {
  .lineMicros = 63.492,
  .syncMicros = 4.7,
  .blankEndMicros = 9.2,
  .backMicros = 1.5,
  .shortVSyncMicros = 2.3,
  .overscanLeftMicros = 0,//1.3,
  .overscanRightMicros = 0,//1,
  .syncVolts = -0.286,
  .blankVolts = 0.0,
  .blackVolts = 0.05, //specs 0.054,
  .whiteVolts = 0.714,
  .lines = 525,
  .linesFirstTop = 20,
  .linesOverscanTop = 6,
  .linesOverscanBottom = 9,
  .imageAspect = 4. / 3.,


  .syncTime_VSYNC1 = 29.4,
  .syncTime_VSYNC2 = 27.077,
  .syncTime_VSYNC3 = 31.777

};

class CompositeOutput
{
  public:
    int samplesLine;
    int samplesSync;
    int samplesBlank;
    int samplesBack;
    int samplesActive;
    int samplesBlackLeft;
    int samplesBlackRight;

    int samplesSyncTime_VSYNC1;
    int samplesSyncTime_VSYNC2;
    int samplesSyncTime_VSYNC3;

    int samplesVSyncShort;
    int samplesVSyncLong;

    char levelSync;
    char levelBlank;
    char levelBlack;
    char levelWhite;
    char grayValues;

    int targetXres;
    int targetYres;
    int targetYresEven;
    int targetYresOdd;

    int linesEven;
    int linesOdd;
    int linesEvenActive;
    int linesOddActive;
    int linesEvenVisible;
    int linesOddVisible;
    int linesEvenBlankTop;
    int linesEvenBlankBottom;
    int linesOddBlankTop;
    int linesOddBlankBottom;

    float pixelAspect;

    unsigned short *line;

    static const i2s_port_t I2S_PORT = (i2s_port_t)I2S_NUM_0;

    enum Mode
    {
      PAL,
      NTSC
    };

    TechProperties &properties;

    CompositeOutput(Mode mode, int xres, int yres, double Vcc = 3.3)
      : properties((mode == NTSC) ? NTSCProperties : PALProperties)
    {
      int linesSyncTop = 5;
      int linesSyncBottom = 3;

      linesOdd = properties.lines / 2;
      linesEven = properties.lines - linesOdd;
      linesEvenActive = linesEven - properties.linesFirstTop - linesSyncBottom;
      linesOddActive = linesOdd - properties.linesFirstTop - linesSyncBottom;
      linesEvenVisible = linesEvenActive - properties.linesOverscanTop - properties.linesOverscanBottom;
      linesOddVisible = linesOddActive - properties.linesOverscanTop - properties.linesOverscanBottom;

      targetYresOdd = (yres / 2 < linesOddVisible) ? yres / 2 : linesOddVisible;
      targetYresEven = (yres - targetYresOdd < linesEvenVisible) ? yres - targetYresOdd : linesEvenVisible;
      targetYres = targetYresEven + targetYresOdd;

      linesEvenBlankTop = properties.linesFirstTop - linesSyncTop + properties.linesOverscanTop + (linesEvenVisible - targetYresEven) / 2;
      linesEvenBlankBottom = linesEven - linesEvenBlankTop - targetYresEven - linesSyncBottom;
      linesOddBlankTop = linesEvenBlankTop;
      linesOddBlankBottom = linesOdd - linesOddBlankTop - targetYresOdd - linesSyncBottom;

      //      double samplesPerSecond = 160000000.0 / 3.0 / 2.0 / 2.0;
      //      double samplesPerMicro = samplesPerSecond * 0.000001;
      samplesLine = (int)(samplesPerMicro * properties.lineMicros + 1.5) & ~1;
      samplesSync = samplesPerMicro * properties.syncMicros + 0.5;
      samplesBlank = samplesPerMicro * (properties.blankEndMicros - properties.syncMicros + properties.overscanLeftMicros) + 0.5;
      samplesBack = samplesPerMicro * (properties.backMicros + properties.overscanRightMicros) + 0.5;
      samplesActive = samplesLine - samplesSync - samplesBlank - samplesBack;

      samplesSyncTime_VSYNC1  = samplesPerMicro * properties.syncTime_VSYNC1 + 0.5;
      samplesSyncTime_VSYNC2  = samplesPerMicro * properties.syncTime_VSYNC2 + 0.5;
      samplesSyncTime_VSYNC3 = samplesPerMicro * properties.syncTime_VSYNC3 + 0.5;

      targetXres = xres < samplesActive ? xres : samplesActive;

      samplesVSyncShort = samplesPerMicro * properties.shortVSyncMicros + 0.5;
      samplesBlackLeft = (samplesActive - targetXres) / 2;
      samplesBlackRight = samplesActive - targetXres - samplesBlackLeft;
      double dacPerVolt = 255.0 / Vcc;
      levelSync = 0;
      levelBlank = (properties.blankVolts - properties.syncVolts) * dacPerVolt + 0.5;
      levelBlack = (properties.blackVolts - properties.syncVolts) * dacPerVolt + 0.5;
      levelWhite = (properties.whiteVolts - properties.syncVolts) * dacPerVolt + 0.5;
      grayValues = levelWhite - levelBlack + 1;

      pixelAspect = (float(samplesActive) / (linesEvenVisible + linesOddVisible)) / properties.imageAspect;
    }

    void init()
    {
      line = (unsigned short*)malloc(sizeof(unsigned short) * samplesLine);
      i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = 10000000,  //not really used
        .bits_per_sample = (i2s_bits_per_sample_t)I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = samplesLine  //a buffer per line
        //.use_apll = true

      };

      i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);    //start i2s driver
      i2s_set_pin(I2S_PORT, NULL);                           //use internal DAC
      i2s_set_sample_rates(I2S_PORT, 10000000);               //dummy sample rate, since the function fails at high values

      //this is the hack that enables the highest sampling rate possible 13.333MHz, have fun
      SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKM_DIV_A_V, 1, I2S_CLKM_DIV_A_S);
      SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKM_DIV_B_V, 1, I2S_CLKM_DIV_B_S);
      SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKM_DIV_NUM_V, 1, I2S_CLKM_DIV_NUM_S);

      SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKA_ENA_V, 1, I2S_CLKA_ENA_S); //use APLL?

      SET_PERI_REG_BITS(I2S_SAMPLE_RATE_CONF_REG(I2S_PORT), I2S_TX_BCK_DIV_NUM_V, 1, I2S_TX_BCK_DIV_NUM_S);


      //debug some stuff
      Serial.println(samplesLine);
    }

    void sendLine()
    {
      //size_t *bytes_written1 = 0;
      esp_err_t i2s_zero_dma_buffer(I2S_PORT);
      i2s_write_bytes(I2S_PORT, (char*)line, samplesLine * sizeof(unsigned short), portMAX_DELAY);
      //esp_err_t i2s_write(I2S_PORT, (char*)line, samplesLine * sizeof(unsigned short), bytes_written1, portMAX_DELAY);
    }

    inline void fillValues(int &i, unsigned char value, int count)
    {
      for (int j = 0; j < count; j++)
        line[i++ ^ 1] = value << 8;
    }

    void fillLine(char *pixels)
    {
      int i = 0;
      fillValues(i, levelSync, samplesSync);
      fillValues(i, levelBlank, samplesBlank);
      fillValues(i, levelBlack, samplesBlackLeft);
      for (int x = 0; x < targetXres / 2; x++)
      {
        short pix = (levelBlack + pixels[x]) << 8;
        line[i++ ^ 1] = pix;
        line[i++ ^ 1]   = pix;
      }
      //i = i - lineoffset;
      fillValues(i, levelBlack, samplesBlackRight);
      fillValues(i, levelBlank, samplesBack);
    }

    void fillLong(int &i)
    {
      fillValues(i, levelSync, samplesLine / 2 - samplesVSyncShort);
      fillValues(i, levelBlank, samplesVSyncShort);
    }

    void fillShort(int &i)
    {
      fillValues(i, levelSync, samplesVSyncShort);
      fillValues(i, levelBlank, samplesLine / 2 - samplesVSyncShort);
    }

    void fillBlank()
    {
      int i = 0;
      fillValues(i, levelSync, samplesSync);
      fillValues(i, levelBlank, samplesBlank);
      fillValues(i, levelBlack, samplesActive);
      //i = i - lineoffset;
      fillValues(i, levelBlank, samplesBack);
    }

    void fillHalfBlank()
    {
      int i = 0;
      fillValues(i, levelSync, samplesSync);
      fillValues(i, levelBlank, samplesLine / 2 - samplesSync);
    }

    void fill_VSYNC_A()
    {
      int i = 0;
      //fillValues(i, levelSync, samplesSync);
      fillValues(i, levelSync, samplesSyncTime_VSYNC1);
      fillValues(i, levelBlank, samplesLine - samplesSyncTime_VSYNC1);
    }


    void fill_VSYNC_B()
    {
      int i = 0;
      fillValues(i, levelSync, samplesSync);
      fillValues(i, levelBlank, samplesSyncTime_VSYNC2);
      fillValues(i, levelSync, samplesSyncTime_VSYNC1);
      fillValues(i, levelSync, samplesSyncTime_VSYNC3 - samplesSyncTime_VSYNC1);
    }

    void fillZeros() {
      int i = 0;
      fillValues(i, levelSync, samplesLine);
    }

    void fillZerosHalf() {
      int i = 0;
      fillValues(i, levelSync, samplesLine / 2);
    }

    void fillBlankVoltage() {
      int i = 0;
      fillValues(i, levelBlank, samplesLine);
    }

    void fillBlankVoltageHalf() {
      int i = 0;
      fillValues(i, levelBlank, samplesLine / 2);
    }

    void sendFrameHalfResolution(char ***frame)
    {
      //Even Halfframe

      I2S_COUNTER_VSYNC = xthal_get_ccount();

      I2S_VSYNC_INTERVAL = lpf1_I2S_VSYNC.update(I2S_COUNTER_VSYNC - I2S_COUNTER_VSYNC_PREV);

      I2S_COUNTER_VSYNC_PREV = I2S_COUNTER_VSYNC;

      VSYNCPhaseError = VSYNClastCount - I2S_COUNTER_VSYNC;
      
      ADJUST_APPL_FREQ();
      //ADJUST_APPL_FREQ();





      int i = 0;
      fillLong(i); fillLong(i);
      sendLine(); sendLine();
      i = 0;
      fillLong(i); fillShort(i);
      sendLine();
      i = 0;
      fillShort(i); fillShort(i);
      sendLine(); sendLine();




      fillBlank();
      for (int y = 0; y < linesEvenBlankTop; y++)
        sendLine();
      for (int y = 0; y < targetYresEven; y++)
      {
        char *pixels = (*frame)[y];
        fillLine(pixels);
        sendLine();
      }
      fillBlank();
      for (int y = 0; y < linesEvenBlankBottom; y++)
        sendLine();
      i = 0;
      fillShort(i); fillShort(i);
      sendLine(); sendLine();
      i = 0;
      fillShort(i);
      //odd half frame
      fillLong(i);
      sendLine();
      i = 0;
      fillLong(i); fillLong(i);
      sendLine(); sendLine();
      i = 0;
      fillShort(i); fillShort(i);
      sendLine(); sendLine();
      i = 0;
      fillShort(i); fillValues(i, levelBlank, samplesLine / 2);
      sendLine();

      fillBlank();
      for (int y = 0; y < linesOddBlankTop; y++)
        sendLine();
      for (int y = 0; y < targetYresOdd; y++)
      {
        char *pixels = (*frame)[y];
        fillLine(pixels);
        sendLine();
      }
      fillBlank();
      for (int y = 0; y < linesOddBlankBottom; y++)
        sendLine();
      i = 0;
      fillHalfBlank(); fillShort(i);
      sendLine();
      i = 0;
      fillShort(i); fillShort(i);
      sendLine(); sendLine();
    }



    void sendVSYNCeven() {

      int i = 0;
      fillShort(i); sendLine();
      fillShort(i); sendLine();
      i = 0;
      fillShort(i); sendLine();
      //odd half frame
      fillLong(i); sendLine();
      i = 0;
      fillLong(i); sendLine();
      fillLong(i); sendLine();
      i = 0;
      fillShort(i); sendLine();
      fillShort(i); sendLine();
      i = 0;
      fillShort(i); sendLine();
      fillShort(i); sendLine();


    }

    void sendVSYNCodd() {

      int i = 0;
      fillShort(i); sendLine();
      fillShort(i); sendLine();
      i = 0;
      fillShort(i); sendLine();
      //odd half frame
      fillLong(i); sendLine();
      i = 0;
      fillLong(i); sendLine();
      fillLong(i); sendLine();
      i = 0;
      fillShort(i); sendLine();
      fillShort(i); sendLine();
      i = 0;
      fillShort(i); sendLine();
      fillValues(i, levelBlank, samplesLine / 2); sendLine();


    }



    void sendOddFrame(char ***frame) {

      fillBlank();
      for (int y = 0; y < linesOddBlankTop; y++) sendLine();

      for (int y = 0; y < targetYresOdd; y++) {
        char *pixels = (*frame)[y];
        fillLine(pixels);
        sendLine();
      }
      fillBlank();
      for (int y = 0; y < linesOddBlankBottom; y++) sendLine();

    }

    void sendEvenFrame(char ***frame) {

      fillBlank();
      for (int y = 0; y < linesEvenBlankTop; y++) sendLine();

      for (int y = 0; y < targetYresEven; y++)
      {
        char *pixels = (*frame)[y];
        fillLine(pixels);
        sendLine();
      }
      fillBlank();
      for (int y = 0; y < linesEvenBlankBottom; y++) sendLine();

    }

    void sendFrameSimpleVSYNC(char ***frame) {


      fill_VSYNC_A(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();

      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();

      sendEvenFrame(frame);

      fill_VSYNC_B(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();

      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZerosHalf(); sendLine();

      sendOddFrame(frame);


    }

    void sendFrameSimpleVSYNC_Progressive(char ***frame) {

      fill_VSYNC_A(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();

      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();

      sendEvenFrame(frame);

    }

    void sendFrameHalfResolution2(char ***frame) {


      //sendVSYNCeven();

      fill_VSYNC_A(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();

      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();
      fillZeros(); sendLine();

      sendEvenFrame(frame);



      //esp_err_t i2s_zero_dma_buffer(I2S_PORT);


      //      fill_VSYNC_B(); sendLine();
      //      fillZeros(); sendLine();
      //      fillZeros(); sendLine();
      //      fillZeros(); sendLine();
      //
      //      fillZeros(); sendLine();
      //      fillZeros(); sendLine();
      //      fillZeros(); sendLine();
      //      fillZeros(); sendLine();
      //      fillZerosHalf(); sendLine();

      fillBlank(); sendLine();





      // fillBlank();sendLine();
      //
      //
      //      //sendVSYNCodd();
      //sendOddFrame(frame);



    }



};
