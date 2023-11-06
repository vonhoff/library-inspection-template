#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#include <stdint.h>

struct point
{
  uint32_t x;
  uint32_t y;
};

struct rectangle
{
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
};

struct circle
{
  uint32_t x;
  uint32_t y;
  uint32_t radius;
};

typedef struct sphere
{
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint32_t radius;
} sphere_t;

typedef struct triangle
{
  uint32_t x1;
  uint32_t y1;
  uint32_t x2;
  uint32_t y2;
  uint32_t x3;
  uint32_t y3;
} triangle_t;

#endif