#include "internal.h"
#include "library.h"
#include <stdlib.h>

point_t *
point_create (uint32_t x, uint32_t y)
{
  point_t *point = malloc (sizeof (struct point));
  if (point != NULL)
    {
      point->x = x;
      point->y = y;
    }

  return point;
}

bool
point_destroy (point_t *point)
{
  bool result = false;
  if (point != NULL)
    {
      free (point);
      result = true;
    }

  return result;
}

uint32_t
point_get_x (const point_t *point)
{
  uint32_t result = 0U;
  if (point != NULL)
    {
      result = point->x;
    }

  return result;
}

uint32_t
point_get_y (const point_t *point)
{
  uint32_t result = 0U;
  if (point != NULL)
    {
      result = point->y;
    }

  return result;
}
