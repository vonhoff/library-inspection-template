#include "internal.h"
#include "library.h"
#include <stdlib.h>

/**
 * @brief Create a new rectangle with the specified parameters.
 *
 * This function allocates memory for a new rectangle structure and initializes its
 * coordinates and dimensions based on the provided values. The caller is responsible
 * for freeing the memory allocated by this function when it is no longer needed.
 *
 * @param x The x-coordinate of the rectangle.
 * @param y The y-coordinate of the rectangle.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 *
 * @return A pointer to the newly created rectangle if allocation succeeds, or NULL
 *         if memory allocation fails.
 */
rectangle_t *
rectangle_create (uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
  rectangle_t *rectangle = malloc (sizeof (struct rectangle));
  if (rectangle != NULL)
    {
      rectangle->x = x;
      rectangle->y = y;
      rectangle->width = w;
      rectangle->height = h;
    }

  return rectangle;
}

/**
 * @brief Destroy a rectangle and release its associated memory.
 *
 * This function frees the memory allocated for the given rectangle. It should be
 * called when the rectangle is no longer needed to prevent memory leaks.
 *
 * @param rectangle A pointer to the rectangle to be destroyed.
 *
 * @return true if the rectangle was successfully destroyed, false if the input
 *         rectangle pointer is NULL.
 */
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

/**
 * @brief Get the x-coordinate of a rectangle.
 *
 * This function returns the x-coordinate of the given rectangle. If the input
 * rectangle pointer is NULL, the function returns 0.
 *
 * @param rectangle A pointer to the rectangle to retrieve the x-coordinate from.
 *
 * @return The x-coordinate of the rectangle, or 0 if the input rectangle pointer is NULL.
 */
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

/**
 * @brief Get the y-coordinate of a rectangle.
 *
 * This function returns the y-coordinate of the given rectangle. If the input
 * rectangle pointer is NULL, the function returns 0.
 *
 * @param rectangle A pointer to the rectangle to retrieve the y-coordinate from.
 *
 * @return The y-coordinate of the rectangle, or 0 if the input rectangle pointer is NULL.
 */
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

/**
 * @brief Get the width of a rectangle.
 *
 * This function returns the width of the given rectangle. If the input rectangle
 * pointer is NULL, the function returns 0.
 *
 * @param rectangle A pointer to the rectangle to retrieve the width from.
 *
 * @return The width of the rectangle, or 0 if the input rectangle pointer is NULL.
 */
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

/**
 * @brief Get the height of a rectangle.
 *
 * This function returns the height of the given rectangle. If the input rectangle
 * pointer is NULL, the function returns 0.
 *
 * @param rectangle A pointer to the rectangle to retrieve the height from.
 *
 * @return The height of the rectangle, or 0 if the input rectangle pointer is NULL.
 */
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
