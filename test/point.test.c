#define CLOVE_SUITE_NAME library
#include "clove-unit.h"
#include "library.h"

CLOVE_TEST (point_create)
{
  point_t *point = point_create (10, 20);
  CLOVE_NOT_NULL (point);
  (void)point_destroy (point);
}

CLOVE_TEST (point_destroy)
{
  point_t *point = point_create (10U, 20U);
  CLOVE_IS_TRUE (point_destroy (point));
}

CLOVE_TEST (point_destroy__on_null)
{
  bool result = point_destroy (NULL);
  CLOVE_IS_FALSE (result);
}

CLOVE_TEST (point_get_x)
{
  point_t *point = point_create (10U, 20U);
  CLOVE_UINT_EQ (10U, point_get_x (point));
  (void)point_destroy (point);
}

CLOVE_TEST (point_get_y)
{
  point_t *point = point_create (10U, 20U);
  CLOVE_UINT_EQ (20U, point_get_y (point));
  (void)point_destroy (point);
}
