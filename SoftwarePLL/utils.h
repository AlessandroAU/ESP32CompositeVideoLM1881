#ifndef UTILS_H
#define UTILS_H

bool doArraysDiffer(volatile uint16_t *array1, volatile uint16_t *array2,  uint8_t len);
bool doArraysDiffer(volatile uint8_t *array1, volatile uint8_t *array2,  uint8_t len);

bool doArraysDiffer(uint16_t *array1, uint16_t *array2,  uint8_t len);
bool doArraysDiffer(uint8_t *array1, uint8_t *array2,  uint8_t len);

#endif
