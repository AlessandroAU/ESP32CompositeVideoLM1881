#ifndef RX5808_H
#define RX5808_H

#include "HardwareConfig.h"

#define MIN_TUNE_TIME 30
#define NumRecievers 2

#define MAX_BAND 6 //highest rx band allowed
#define MAX_CHANNEL 7 //highest rc channel allowed

volatile uint16_t RXfrequency[NumRecievers];
volatile uint8_t RXBand[NumRecievers] = {4, 4};
volatile uint8_t RXChannel[NumRecievers] = {4, 4};

volatile uint16_t RXfrequencyPrev[NumRecievers];   //holds the previous values of the settings to detect if a change is REQ)
volatile uint8_t RXBandPrev[NumRecievers] = {0, 0};
volatile uint8_t RXChannelPrev[NumRecievers] = {0, 0};

const uint16_t channelFreqTable[] = {
  //     // Channel 1 - 8
  5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // Raceband
  5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // Band A
  5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // Band B
  5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // Band E
  5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // Band F / Airwave
  5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621, // Band D / 5.3
  5180, 5200, 5220, 5240, 5745, 5765, 5785, 5805, // connex
  5825, 5845, 5845, 5845, 5845, 5845, 5845, 5845  // even more connex, last 6 unused!!!
};

void IncBand();
void IncChannel();

void DecBand();
void DecChannel();

void RXcheckIfUpdateReq();


#endif
