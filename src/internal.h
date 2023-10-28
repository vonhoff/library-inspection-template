#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#include <stdint.h>

struct point {
  uint32_t x;
  uint32_t y;
};

struct rectangle {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
};

struct circle {
  uint32_t x;
  uint32_t y;
  uint32_t radius;
};

#endif