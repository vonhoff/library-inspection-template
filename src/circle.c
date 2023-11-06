#include "internal.h"
#include "library.h"
#include <stdlib.h>

/**
 * @brief Create a new circle with the specified center coordinates and radius.
 *
 * This function allocates memory for a new circle structure and initializes its
 * center coordinates and radius based on the provided values. The caller is
 * responsible for freeing the memory allocated by this function when it is no
 * longer needed.
 *
 * @param x      The x-coordinate of the center of the circle.
 * @param y      The y-coordinate of the center of the circle.
 * @param radius The radius of the circle.
 *
 * @return A pointer to the newly created circle if allocation succeeds, or NULL
 *         if memory allocation fails.
 */
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

/**
 * @brief Destroy a circle and release its associated memory.
 *
 * This function frees the memory allocated for the given circle. It should be
 * called when the circle is no longer needed to prevent memory leaks.
 *
 * @param circle A pointer to the circle to be destroyed.
 *
 * @return true if the circle was successfully destroyed, false if the input
 * circle pointer is NULL.
 */
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

/**
 * @brief Get the x-coordinate of the center of a circle.
 *
 * This function returns the x-coordinate of the center of the given circle. If
 * the input circle pointer is NULL, the function returns 0.
 *
 * @param circle A pointer to the circle to retrieve the x-coordinate from.
 *
 * @return The x-coordinate of the center of the circle, or 0 if the input
 * circle pointer is NULL.
 */
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

/**
 * @brief Get the y-coordinate of the center of a circle.
 *
 * This function returns the y-coordinate of the center of the given circle. If
 * the input circle pointer is NULL, the function returns 0.
 *
 * @param circle A pointer to the circle to retrieve the y-coordinate from.
 *
 * @return The y-coordinate of the center of the circle, or 0 if the input
 * circle pointer is NULL.
 */
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

/**
 * @brief Get the radius of a circle.
 *
 * This function returns the radius of the given circle. If the input circle
 * pointer is NULL, the function returns 0.
 *
 * @param circle A pointer to the circle to retrieve the radius from.
 *
 * @return The radius of the circle, or 0 if the input circle pointer is NULL.
 */
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
