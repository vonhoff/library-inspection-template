#include "internal.h"
#include "library.h"
#include <stdlib.h>

rectangle_t *
rectangle_create (uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
  rectangle_t *rectangle = malloc (sizeof (struct rectangle));
  if (rectangle != NULL)
    {
      rectangle->x = x;
      rectangle->y = y;
      rectangle->width = width;
      rectangle->height = height;
    }

  return rectangle;
}

bool
rectangle_destroy (rectangle_t *rectangle)
{
  bool result = false;
  if (rectangle != NULL)
    {
      free (rectangle);
      result = true;
    }

  return result;
}

uint32_t
rectangle_get_x (const rectangle_t *rectangle)
{
  uint32_t result = 0U;
  if (rectangle != NULL)
    {
      result = rectangle->x;
    }

  return result;
}

uint32_t
rectangle_get_y (const rectangle_t *rectangle)
{
  uint32_t result = 0U;
  if (rectangle != NULL)
    {
      result = rectangle->y;
    }

  return result;
}

uint32_t
rectangle_get_width (const rectangle_t *rectangle)
{
  uint32_t result = 0U;
  if (rectangle != NULL)
    {
      result = rectangle->width;
    }

  return result;
}

uint32_t
rectangle_get_height (const rectangle_t *rectangle)
{
  uint32_t result = 0U;
  if (rectangle != NULL)
    {
      result = rectangle->height;
    }

  return result;
}
