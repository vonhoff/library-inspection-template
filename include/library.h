#ifndef _LIBRARY_H_
#define _LIBRARY_H_

#ifdef _MSC_VER
#if _MSC_VER < 1800
#error "This code requires at least Visual C++ 2013 or higher."
#endif
#ifdef LIB_EXPORT
#define API __declspec (dllexport)
#else
#define API __declspec (dllimport)
#endif
#else
#ifdef LIB_EXPORT
#define API __attribute__ ((visibility ("default")))
#else
#define API
#endif
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct point point_t;

API point_t *point_create (uint32_t x, uint32_t y);
API bool point_destroy (point_t *point);
API uint32_t point_get_x (const point_t *point);
API uint32_t point_get_y (const point_t *point);

#endif
