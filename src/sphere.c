#include "internal.h"
#include "library.h"
#include <stdlib.h>

/**
 * @brief Creates a new sphere.
 *
 * @param x The x-coordinate of the sphere.
 * @param y The y-coordinate of the sphere.
 * @param z The z-coordinate of the sphere.
 * @param r The radius of the sphere.
 * @return A pointer to the newly created sphere.
 */
sphere_t *
sphere_create (uint32_t x, uint32_t y, uint32_t z, uint32_t r)
{
  sphere_t *sphere = malloc (sizeof (sphere_t));
  if (sphere)
    {
      sphere->x = x;
      sphere->y = y;
      sphere->z = z;
      sphere->radius = r;
    }
  return sphere;
}

/**
 * @brief Destroys a sphere and frees the associated memory.
 *
 * @param sphere A pointer to the sphere to be destroyed.
 * @return true if the sphere is successfully destroyed, false otherwise.
 */
bool
sphere_destroy (sphere_t *sphere)
{
  if (sphere)
    {
      free (sphere);
      return true;
    }
  return false;
}

/**
 * @brief Gets the x-coordinate of the sphere.
 *
 * @param sphere A pointer to the sphere.
 * @return The x-coordinate of the sphere or 0 if the sphere is NULL.
 */
uint32_t
sphere_get_x (const sphere_t *sphere)
{
  return sphere ? sphere->x : 0;
}

/**
 * @brief Gets the y-coordinate of the sphere.
 *
 * @param sphere A pointer to the sphere.
 * @return The y-coordinate of the sphere or 0 if the sphere is NULL.
 */
uint32_t
sphere_get_y (const sphere_t *sphere)
{
  return sphere ? sphere->y : 0;
}

/**
 * @brief Gets the z-coordinate of the sphere.
 *
 * @param sphere A pointer to the sphere.
 * @return The z-coordinate of the sphere or 0 if the sphere is NULL.
 */
uint32_t
sphere_get_z (const sphere_t *sphere)
{
  return sphere ? sphere->z : 0;
}

/**
 * @brief Gets the radius of the sphere.
 *
 * @param sphere A pointer to the sphere.
 * @return The radius of the sphere or 0 if the sphere is NULL.
 */
uint32_t
sphere_get_radius (const sphere_t *sphere)
{
  return sphere ? sphere->radius : 0;
}
