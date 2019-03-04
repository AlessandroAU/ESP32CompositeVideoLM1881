#define SPI_ADDRESS_SYNTH_B 0x01
#define SPI_ADDRESS_POWER   0x0A
#define SPI_ADDRESS_STATE   0x0F

#include "HardwareConfig.h"
#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include <driver/timer.h>
#include "RX5808.h"


void InitSPI() {
  pinMode(SCK, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);;
  pinMode(MISO, INPUT);
  SPI.begin(SCK, MISO, MOSI, -1);
}

void rxWrite(uint8_t addressBits, uint32_t dataBits, uint8_t CSpin) {

  uint32_t data = addressBits | (1 << 4) | (dataBits << 5);

  SPI.beginTransaction(SPISettings(100000, LSBFIRST, SPI_MODE0));
  digitalWrite(CSpin, LOW);
  SPI.transferBits(data, NULL, 25);
  digitalWrite(CSpin, HIGH);
  SPI.endTransaction();
  delayMicroseconds(MIN_TUNE_TIME);

}

uint16_t getSynthRegisterBFreq(uint16_t f) {
  return ((((f - 479) / 2) / 32) << 7) | (((f - 479) / 2) % 32);
}


void setChannel(uint8_t channel, uint8_t NodeAddr) {
  Serial.println(channel);

  if (channel >= 0 && channel <= 7) {
    Serial.println("setChannel");
    RXChannel[NodeAddr] = channel;
    uint8_t band = RXBand[NodeAddr];
    uint16_t SetFreq = setModuleChannelBand(channel, band, NodeAddr);
  }
}

void setBand(uint8_t band, uint8_t NodeAddr) {
  Serial.println(band);

  if (band >= 0 && band <= MAX_BAND) {
    Serial.println("setBand");
    RXBand[NodeAddr] = band;
    uint8_t channel = RXChannel[NodeAddr];
    uint16_t SetFreq = setModuleChannelBand(channel, band, NodeAddr);
  }
}

uint16_t setModuleChannelBandfromGVAR(uint8_t NodeAddr) {
  Serial.println("setModuleChannelBand");
  Serial.print(RXChannel[NodeAddr]);
  Serial.print(",");
  Serial.println(RXBand[NodeAddr]);

  uint8_t index = RXChannel[NodeAddr] + (8 * RXBand[NodeAddr]);
  Serial.println(index);
  uint16_t frequency = channelFreqTable[index];
  RXfrequency[NodeAddr] = frequency;
  //return setModuleFrequency(frequency);

  switch (NodeAddr) {
    case 0:
      rxWrite(SPI_ADDRESS_SYNTH_B, getSynthRegisterBFreq(frequency), CS1);
      break;

    case 1:
      rxWrite(SPI_ADDRESS_SYNTH_B, getSynthRegisterBFreq(frequency), CS2);
      break;
  }


  printf("a = %" PRIu16 "\n", frequency);
  return frequency;
}

uint16_t setModuleChannelBand(uint8_t channel, uint8_t band, uint8_t NodeAddr) {
  Serial.println("setModuleChannelBand");
  Serial.print(channel);
  Serial.print(",");
  Serial.println(band);

  uint8_t index = channel + (8 * band);
  Serial.println(index);
  uint16_t frequency = channelFreqTable[index];
  //return setModuleFrequency(frequency);

  switch (NodeAddr) {
    case 0:
      rxWrite(SPI_ADDRESS_SYNTH_B, getSynthRegisterBFreq(frequency), CS1);
      break;

    case 1:
      rxWrite(SPI_ADDRESS_SYNTH_B, getSynthRegisterBFreq(frequency), CS2);
      break;

  }


  //printf("a = %" PRIu16 "\n", frequency);
  return frequency;
}

void IncBand() { //incriments the band

  if (RXBand[0] < MAX_BAND) {   //for now we are lazy and just check the first index
    for (int i = 0; i < NumRecievers; i++) {
      RXBand[i] = RXBand[i] + 1;
    }
  } else {
    for (int i = 0; i < NumRecievers; i++) {
      RXBand[i] = 0;
    }
  }
}

void DecBand() { //incriments the band

  if (RXBand[0] != 0) {   //for now we are lazy and just check the first index
    for (int i = 0; i < NumRecievers; i++) {
      RXBand[i] = RXBand[i] - 1;
    }
  } else {
    for (int i = 0; i < NumRecievers; i++) {
      RXBand[i] = MAX_BAND;
    }
  }
}

void IncChannel() { //incriments the channel

  if (RXChannel[0] < MAX_CHANNEL) {   //for now we are lazy and just check the first index
    for (int i = 0; i < NumRecievers; i++) {
      RXChannel[i] = RXChannel[i] + 1;
    }
  } else {
    for (int i = 0; i < NumRecievers; i++) {
      RXChannel[i] = 0;
    }
  }
}

void DecChannel() { //incriments the channel

  if (RXChannel[0] != 0) {   //for now we are lazy and just check the first index
    for (int i = 0; i < NumRecievers; i++) {
      RXChannel[i] = RXChannel[i] - 1;
    }
  } else {
    for (int i = 0; i < NumRecievers; i++) {
      RXChannel[i] = MAX_CHANNEL;
    }
  }
}


uint16_t setModuleFrequency(uint16_t frequency, uint8_t NodeAddr) {

  switch (NodeAddr) {
    case 0:
      rxWrite(SPI_ADDRESS_SYNTH_B, getSynthRegisterBFreq(frequency), CS1);
      break;

    case 1:
      rxWrite(SPI_ADDRESS_SYNTH_B, getSynthRegisterBFreq(frequency), CS2);
      break;

  }
  return frequency;
}

void RXcheckIfUpdateReq() { //checks if an update to the RX5808 regs is required.

  if (doArraysDiffer(RXBandPrev, RXBand, NumRecievers) or doArraysDiffer(RXChannelPrev, RXChannel, NumRecievers)) { //this means we need to now do and update
    for (int i = 0; i < NumRecievers; i++) {
      setModuleChannelBandfromGVAR(i);  //sets the channel and band from the global variables RXBand RXChannel;
      
    }
  }

  /// Update the "old" values

  for (int i = 0; i < NumRecievers; i++) {
    RXfrequencyPrev[i] = RXfrequency[i];
    RXBandPrev[i] = RXBand[i];
    RXChannelPrev[i] = RXChannel[i];
  }
}
