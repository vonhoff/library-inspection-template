#include "internal.h"
#include "library.h"
#include <stdlib.h>

/**
 * @brief Create a new point with the specified coordinates.
 *
 * This function allocates memory for a new point structure and initializes its
 * coordinates based on the provided values. The caller is responsible for freeing
 * the memory allocated by this function when it is no longer needed.
 *
 * @param x The x-coordinate of the point.
 * @param y The y-coordinate of the point.
 *
 * @return A pointer to the newly created point if allocation succeeds, or NULL if
 * memory allocation fails.
 */
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

/**
 * @brief Destroy a point and release its associated memory.
 *
 * This function frees the memory allocated for the given point. It should be called
 * when the point is no longer needed to prevent memory leaks.
 *
 * @param point A pointer to the point to be destroyed.
 *
 * @return true if the point was successfully destroyed, false if the input point
 * pointer is NULL.
 */
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

/**
 * @brief Get the x-coordinate of a point.
 *
 * This function returns the x-coordinate of the given point. If the input point
 * pointer is NULL, the function returns 0.
 *
 * @param point A pointer to the point to retrieve the x-coordinate from.
 *
 * @return The x-coordinate of the point, or 0 if the input point pointer is NULL.
 */
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

/**
 * @brief Get the y-coordinate of a point.
 *
 * This function returns the y-coordinate of the given point. If the input point
 * pointer is NULL, the function returns 0.
 *
 * @param point A pointer to the point to retrieve the y-coordinate from.
 *
 * @return The y-coordinate of the point, or 0 if the input point pointer is NULL.
 */
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
