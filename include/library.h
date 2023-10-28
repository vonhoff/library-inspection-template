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

/* The following are for purposes of demonstration and testing. */
/* The formatting is intentionally poor to test the declaration tokenizer (please don't program like this). */

typedef struct point point_t;
typedef struct rectangle rectangle_t;
typedef struct circle circle_t;

API point_t *point_create (uint32_t x,    uint32_t y    );
API bool point_destroy (point_t *point);
API uint32_t point_get_x ( const point_t *point);
API uint32_t point_get_y (const point_t *point );

API rectangle_t *rectangle_create (uint32_t x,
                                   uint32_t y,
                                   uint32_t width, uint32_t height
                                   );
API bool rectangle_destroy (rectangle_t *rectangle);
 API uint32_t rectangle_get_x (const rectangle_t *rectangle);
API  uint32_t rectangle_get_y(const rectangle_t *rectangle);
API uint32_t   rectangle_get_width(const rectangle_t *rectangle);
API uint32_t rectangle_get_height  (const rectangle_t *rectangle); /*
API uint32_t should_be_ignored (const rectangle_t *rectangle);
API uint32_t should_be_ignored (void); */

API circle_t *circle_create (
    uint32_t x, uint32_t y, uint32_t radius);
API bool circle_destroy (circle_t *circle)   ;
API uint32_t circle_get_x (const circle_t *circle);
API
uint32_t circle_get_y (const circle_t *circle);
API uint32_t
circle_get_radius (const circle_t *circle);

// API uint32_t should_be_ignored (void);

   /*
API uint32_t should_be_ignored (void);
API uint32_t should_be_ignored (void);
API uint32_t should_be_ignored (void);
*/

#endif
