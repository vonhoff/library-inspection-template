#include "internal.h"
#include "library.h"
#include <stdlib.h>

circle_t *
circle_create (uint32_t x, uint32_t y, uint32_t radius)
{
  circle_t *circle = malloc (sizeof (struct circle));
  if (circle != NULL)
    {
      circle->x = x;
      circle->y = y;
      circle->radius = radius;
    }

  return circle;
}

bool
circle_destroy (circle_t *circle)
{
  bool result = false;
  if (circle != NULL)
    {
      free (circle);
      result = true;
    }

  return result;
}

uint32_t
circle_get_x (const circle_t *circle)
{
  uint32_t result = 0U;
  if (circle != NULL)
    {
      result = circle->x;
    }

  return result;
}

uint32_t
circle_get_y (const circle_t *circle)
{
  uint32_t result = 0U;
  if (circle != NULL)
    {
      result = circle->y;
    }

  return result;
}

uint32_t
circle_get_radius (const circle_t *circle)
{
  uint32_t result = 0U;
  if (circle != NULL)
    {
      result = circle->radius;
    }

  return result;
}
