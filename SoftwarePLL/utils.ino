#include "utils.h"

bool doArraysDiffer(volatile uint16_t *array1, volatile uint16_t *array2,  byte len) { //compare arrays at depth of len
  bool result = false;

  for (int i = 0; i < len; i++) {

    if (array1[i] != array2[i]) {
      return true;
    }
  }
  return result;
}


bool doArraysDiffer(volatile uint8_t *array1, volatile uint8_t *array2,  uint8_t len) { //compare arrays at depth of len
  bool result = false;

  for (int i = 0; i < len; i++) {

    if (array1[i] != array2[i]) {
      return true;
    }
  }
  return result;
}

bool doArraysDiffer(uint16_t *array1, uint16_t *array2,  byte len) { //compare arrays at depth of len
  bool result = false;

  for (int i = 0; i < len; i++) {

    if (array1[i] != array2[i]) {
      return true;
    }
  }
  return result;
}


bool doArraysDiffer(uint8_t *array1, uint8_t *array2,  uint8_t len) { //compare arrays at depth of len
  bool result = false;

  for (int i = 0; i < len; i++) {

    if (array1[i] != array2[i]) {
      return true;
    }
  }
  return result;
}



