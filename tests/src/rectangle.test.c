#define CLOVE_SUITE_NAME rectangle
#include "clove-unit.h"
#include "library.h"

CLOVE_TEST(rectangle_create)
{
  rectangle_t *rectangle = rectangle_create (10U, 20U, 30U, 40U);
  CLOVE_NOT_NULL(rectangle);
  (void)rectangle_destroy (rectangle);
}

CLOVE_TEST(rectangle_destroy)
{
  rectangle_t *rectangle = rectangle_create (10U, 20U, 30U, 40U);
  CLOVE_IS_TRUE(rectangle_destroy (rectangle));
}

CLOVE_TEST(rectangle_get_x)
{
  rectangle_t *rectangle = rectangle_create (10U, 20U, 30U, 40U);
  CLOVE_UINT_EQ(10U, rectangle_get_x (rectangle));
  (void)rectangle_destroy (rectangle);
}

CLOVE_TEST(rectangle_get_y)
{
  rectangle_t *rectangle = rectangle_create (10U, 20U, 30U, 40U);
  CLOVE_UINT_EQ(20U, rectangle_get_y (rectangle));
  (void)rectangle_destroy (rectangle);
}

CLOVE_TEST(rectangle_get_width)
{
  rectangle_t *rectangle = rectangle_create (10U, 20U, 30U, 40U);
  CLOVE_UINT_EQ(30U, rectangle_get_width (rectangle));
  (void)rectangle_destroy (rectangle);
}

CLOVE_TEST(rectangle_get_height)
{
  rectangle_t *rectangle = rectangle_create (10U, 20U, 30U, 40U);
  CLOVE_UINT_EQ(40U, rectangle_get_height (rectangle));
  (void)rectangle_destroy (rectangle);
}
