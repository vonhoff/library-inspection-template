#define CLOVE_SUITE_NAME _inspection
#include "clove-unit.h"
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* Max array lengths */
#define MAX_PARAMETERS 8
#define MAX_PROTOTYPES 128
#define MAX_TEST_BRANCHES 16
#define MAX_TESTS (MAX_PROTOTYPES * MAX_TEST_BRANCHES)

/* Max string lengths */
#define LENGTH_LINE 512U
#define LENGTH_FUNCTION_NAME 128U
#define LENGTH_PARAMETER_NAME 128U

/* Project configuration */
#define LIBRARY_PREFIX_API "API"
#define LIBRARY_PATH_INCLUDE "include/library.h"
#define LIBRARY_PATH_SRC "src"
#define LIBRARY_PATH_TEST "test"
#define LIBRARY_EXTENSION_SRC ".c"
#define LIBRARY_EXTENSION_TEST ".test.c"

/* Test formatting */
#define FORMAT_TEST "CLOVE_TEST"
#define FORMAT_TEST_ANNOTATION "@covers %s"

/* String markers */
#define MARKER_COMMENT "//"
#define MARKER_COMMENT_START "/*"
#define MARKER_COMMENT_END "*/"

/* Character markers */
#define CHAR_IMPLEMENTATION_START '{'
#define CHAR_PROTOTYPE_END ';'
#define CHAR_TEST_END ')'

/* Report formatting */
#define REPORT_ALIGNMENT_DOTS 40U
#define REPORT_ERROR "\033[1;31m"
#define REPORT_WARN "\033[1;33m"
#define REPORT_END "\033[0m\n"

/**
 * Represents coverage information for a specific test path.
 */
typedef struct
{
  char test_path[FILENAME_MAX];
  uint32_t line;
  bool is_annotation;
} coverage_t;

/**
 * Represents a function declaration along with related information.
 */
typedef struct
{
  char function_name[LENGTH_FUNCTION_NAME];
  char source_path[FILENAME_MAX];
  char expected_test_path[FILENAME_MAX];
  char parameters[MAX_PARAMETERS][LENGTH_PARAMETER_NAME];
  coverage_t coverages[MAX_TEST_BRANCHES];
  uint32_t parameters_count;
  uint32_t coverages_count;
  uint32_t coverage_annotation_count;
  uint32_t definition_line_number;
  uint32_t declaration_line_number;
  bool is_prototype_match;
  bool has_test_file;
} declaration_t;

static char g_path_include_file[FILENAME_MAX] = { 0 };
static char g_path_src_directory[FILENAME_MAX] = { 0 };
static char g_path_test_directory[FILENAME_MAX] = { 0 };

static declaration_t g_declarations[MAX_PROTOTYPES] = { 0 };
static uint32_t g_declarations_count = 0U;

static coverage_t g_tests[MAX_TESTS] = { 0 };
static uint32_t g_tests_count = 0U;

/* Report Flags */
static bool g_invalid_setup = true;
static bool g_print_undefined = false;
static bool g_print_uncovered = false;
static bool g_print_missing_test_files = false;
static bool g_print_test_mismatches = false;
static bool g_print_prototype_mismatches = false;
static bool g_print_invalid_tests = false;

/* Miscellaneous Utilities */
static bool file_exists (const char *path);
static bool char_is_word_boundary (char target);

/* String Utilities */
static char *string_get_spacing_dots (int32_t length, char *buffer);
static void string_normalize_spaces (char *dest, const char *src);
static void string_remove_spaces (char *dest, const char *src);

/* Line Utilities */
static bool line_tokenize_parameters (const char *line, declaration_t *declaration);
static bool line_get_prototype (const char *line, char *out, bool *is_multiple);
static bool line_get_definition (const char *line, char *out, bool *is_multiple, const char *name);
static bool line_get_function_test (const char *line, char *out, bool *is_multiple, const char *name);
static bool line_get_function_name (const char *line, char *out);
static bool line_find_word (const char *line, const char *word);
static bool line_is_comment (const char *line, bool *is_block);

/* Global Declarations Management */
static bool g_declarations_append (const char *line, uint32_t line_number);
static bool g_declarations_update_tests (FILE *test_file, const char *test_path);
static void g_declarations_update_definitions (FILE *src_file, const char *src_path, const char *expected_test_path);

/* Global Invalid Test Management */
static bool g_tests_append (const char *test_path, uint32_t line_number);
static void g_tests_remove (uint32_t index);
static bool g_tests_update_data (FILE *test_file, const char *test_path);
static void g_tests_filter_defined (void);

/* Declaration Management */
static bool declaration_add_coverage (declaration_t *declaration, const char *test_path, uint32_t line_number, bool is_annotation);
static void declaration_update_validation (declaration_t *declaration, const char *buffer);

/* Project Loaders */
static bool load_prototypes (void);
static bool load_definitions (void);
static bool load_tests (void);

CLOVE_SUITE_SETUP_ONCE ()
{
  /* Builds full paths for the include file, source directory, and test directory. */
  (void)snprintf (g_path_include_file, sizeof (g_path_include_file), "%s/%s", PROJECT_ROOT, LIBRARY_PATH_INCLUDE);
  (void)snprintf (g_path_src_directory, sizeof (g_path_src_directory), "%s/%s", PROJECT_ROOT, LIBRARY_PATH_SRC);
  (void)snprintf (g_path_test_directory, sizeof (g_path_test_directory), "%s/%s", PROJECT_ROOT, LIBRARY_PATH_TEST);

  /* Collect data about the prototypes, definitions and tests. */
  g_invalid_setup = ((load_prototypes () == false) || (load_definitions () == false) || (load_tests () == false));

  /* Stop if the setup was not successful */
  if (g_invalid_setup)
    {
      return;
    }

  /* Filter defined tests, the remaining will be invalid. */
  g_tests_filter_defined ();
  g_print_invalid_tests = g_tests_count > 0;

  /* Determine report statuses */
  const declaration_t *declaration = NULL;
  for (uint32_t i = 0; i < g_declarations_count; ++i)
    {
      declaration = &g_declarations[i];

      /* Check for test definition mismatches, excluding annotations */
      for (uint32_t j = 0; (j < declaration->coverages_count) && (g_print_test_mismatches == false); ++j)
        {
          if ((declaration->coverages[j].is_annotation == false)
              && (strcmp (declaration->coverages[j].test_path, declaration->expected_test_path) != 0))
            {
              g_print_test_mismatches = true;
            }
        }

      /* If not all are annotated, tests should be defined within a test file */
      if (((declaration->coverages_count == 0) || (declaration->coverage_annotation_count < declaration->coverages_count))
          && (declaration->definition_line_number > 0) && (declaration->has_test_file == false))
        {
          g_print_missing_test_files = true;
        }

      /* Check if the prototype has been defined */
      if (declaration->definition_line_number == 0)
        {
          g_print_undefined = true;
        }

      /* Check if the prototype has any coverage at all */
      if ((declaration->coverages_count == 0) && (declaration->definition_line_number > 0))
        {
          g_print_uncovered = true;
        }

      /* Check if the definition fully matches the prototype */
      if ((declaration->definition_line_number > 0) && (declaration->is_prototype_match == false))
        {
          g_print_prototype_mismatches = true;
        }
    }
}

CLOVE_TEST (check_undefined)
{
  if (g_invalid_setup)
    {
      CLOVE_FAIL ();
      return;
    }

  if (g_print_undefined)
    {
      (void)printf ("\n");
      (void)printf (REPORT_ERROR "Define the following prototype declarations: " REPORT_END);
      char dots_buffer[REPORT_ALIGNMENT_DOTS + 1] = { 0 };
      const declaration_t *declaration = NULL;

      for (uint32_t i = 0; i < g_declarations_count; ++i)
        {
          declaration = &g_declarations[i];
          if (declaration->definition_line_number == 0)
            {
              int32_t name_count = printf (" - %s", declaration->function_name);
              (void)printf ("%s", string_get_spacing_dots (name_count, dots_buffer));
              (void)printf ("(%s:%u)\n", g_path_include_file, declaration->declaration_line_number);
            }
        }

      CLOVE_FAIL ();
    }

  CLOVE_PASS ();
}

CLOVE_TEST (check_uncovered)
{
  if (g_invalid_setup)
    {
      CLOVE_FAIL ();
      return;
    }

  if (g_print_uncovered)
    {
      (void)printf ("\n");
      (void)printf (REPORT_WARN "Create tests for the following declarations: " REPORT_END);
      char dots_buffer[REPORT_ALIGNMENT_DOTS + 1] = { 0 };
      const declaration_t *declaration = NULL;

      for (uint32_t i = 0; i < g_declarations_count; ++i)
        {
          declaration = &g_declarations[i];
          if ((declaration->coverages_count == 0) && (declaration->definition_line_number > 0))
            {
              int32_t name_count = printf (" - %s", declaration->function_name);
              (void)printf ("%s", string_get_spacing_dots (name_count, dots_buffer));
              (void)printf ("(%s:%u) => Expected in: %s\n", declaration->source_path, declaration->definition_line_number,
                            declaration->expected_test_path);
            }
        }

      CLOVE_FAIL ();
    }

  CLOVE_PASS ();
}

CLOVE_TEST (check_missing_test_files)
{
  if (g_invalid_setup)
    {
      CLOVE_FAIL ();
      return;
    }

  if (g_print_missing_test_files)
    {
      (void)printf ("\n");
      (void)printf (REPORT_ERROR "Create the following test files: " REPORT_END);
      bool visited[MAX_PROTOTYPES] = { false };
      const declaration_t *declaration = NULL;

      for (uint32_t i = 0; i < g_declarations_count; ++i)
        {
          declaration = &g_declarations[i];
          if (((declaration->coverages_count == 0) || (declaration->coverage_annotation_count < declaration->coverages_count))
              && (declaration->definition_line_number > 0) && (declaration->has_test_file == false) && (visited[i] == false))
            {
              (void)printf (" - %s\n", declaration->expected_test_path);
              for (uint32_t j = i + 1U; j < g_declarations_count; ++j)
                {
                  if (strcmp (declaration->expected_test_path, g_declarations[j].expected_test_path) == 0)
                    {
                      visited[j] = true;
                    }
                }
            }
        }

      CLOVE_FAIL ();
    }

  CLOVE_PASS ();
}

CLOVE_TEST (check_test_mismatches)
{
  if (g_invalid_setup)
    {
      CLOVE_FAIL ();
      return;
    }

  if (g_print_test_mismatches)
    {
      (void)printf ("\n");
      (void)printf (REPORT_WARN "Move the following tests to their expected location: " REPORT_END);
      char dots_buffer[REPORT_ALIGNMENT_DOTS + 1] = { 0 };
      const declaration_t *declaration = NULL;

      for (uint32_t i = 0; i < g_declarations_count; ++i)
        {
          declaration = &g_declarations[i];
          for (uint32_t j = 0; j < declaration->coverages_count; ++j)
            {
              if ((declaration->coverages[j].is_annotation == false)
                  && (strcmp (declaration->coverages[j].test_path, declaration->expected_test_path) != 0))
                {
                  int32_t name_count = printf (" - %s", declaration->function_name);
                  (void)printf ("%s", string_get_spacing_dots (name_count, dots_buffer));
                  (void)printf ("(%s:%u) => Expected in: %s\n", declaration->coverages[j].test_path, declaration->coverages[j].line,
                                declaration->expected_test_path);
                }
            }
        }

      CLOVE_FAIL ();
    }

  CLOVE_PASS ();
}

CLOVE_TEST (check_prototype_mismatches)
{
  if (g_invalid_setup)
    {
      CLOVE_FAIL ();
      return;
    }

  if (g_print_prototype_mismatches)
    {
      (void)printf ("\n");
      (void)printf (REPORT_WARN "Make sure that the parameters of the following definitions match their prototype: " REPORT_END);
      char dots_buffer[REPORT_ALIGNMENT_DOTS + 1] = { 0 };
      const declaration_t *declaration = NULL;

      for (uint32_t i = 0; i < g_declarations_count; ++i)
        {
          declaration = &g_declarations[i];

          if ((declaration->definition_line_number > 0) && (declaration->is_prototype_match == false))
            {
              int32_t name_count = printf (" - %s", declaration->function_name);
              (void)printf ("%s", string_get_spacing_dots (name_count, dots_buffer));
              (void)printf ("(%s:%u)\n", declaration->source_path, declaration->definition_line_number);
            }
        }

      CLOVE_FAIL ();
    }

  CLOVE_PASS ();
}

CLOVE_TEST (check_invalid_tests)
{
  if (g_invalid_setup)
    {
      CLOVE_FAIL ();
      return;
    }

  if (g_print_invalid_tests)
    {
      (void)printf ("\n");
      (void)printf (REPORT_WARN "Make sure that the following test names match an API endpoint or follow the variation format: " REPORT_END);
      const coverage_t *invalid_test = NULL;

      for (uint32_t i = 0; i < g_tests_count; ++i)
        {
          invalid_test = &g_tests[i];
          (void)printf (" - %s:%d\n", invalid_test->test_path, invalid_test->line);
        }

      CLOVE_FAIL ();
    }

  CLOVE_PASS ();
}

/**
 * Check if a file exists.
 *
 * @param path The name of the file to be checked.
 * @return Returns true if the file exists, and false otherwise.
 */
static bool
file_exists (const char *path)
{
  struct stat buffer = { 0 };
  return (stat (path, &buffer) == 0);
}

/**
 * Determines whether a given character is a word boundary.
 *
 * @param target The character to check for word boundary.
 * @return true if the character is a word boundary, false otherwise.
 */
static bool
char_is_word_boundary (char target)
{
  return (isspace (target) != 0) || (target == '(') || (target == ')') || (target == '*');
}

/**
 * Generates a string of spacing dots based on the specified alignment requirements.
 *
 * @param length The length of the string.
 * @param buffer The buffer to store the spacing dots. Should be large enough to hold (at least) MAX_DOTS + 1 for the null
 * terminator.
 * @return A pointer to the buffer containing the required number of spacing dots for alignment.
 */
static char *
string_get_spacing_dots (int32_t length, char *buffer)
{
  uint32_t dots_needed = REPORT_ALIGNMENT_DOTS - (uint32_t)length;
  dots_needed = (dots_needed < 1) ? 1 : dots_needed;
  dots_needed = dots_needed > REPORT_ALIGNMENT_DOTS ? REPORT_ALIGNMENT_DOTS : dots_needed;

  memset (buffer, '.', REPORT_ALIGNMENT_DOTS);
  buffer[dots_needed] = '\0';

  return buffer;
}

/**
 * Replaces newline characters with spaces and reduces adjacent spaces to a single space in a given string.
 *
 * @param dest The destination character array where the modified string will be stored.
 * @param src The src string to process.
 */
static void
string_normalize_spaces (char *dest, const char *src)
{
  uint32_t len = (uint32_t)strlen (src);
  uint32_t output_index = 0;
  bool prev_space = false;

  for (uint32_t i = 0; i < len; ++i)
    {
      if (src[i] == '\n')
        {
          dest[output_index++] = ' ';
          prev_space = false;
        }
      else if (src[i] == ' ')
        {
          if (prev_space == false)
            {
              dest[output_index++] = ' ';
              prev_space = true;
            }
        }
      else
        {
          dest[output_index++] = src[i];
          prev_space = false;
        }
    }

  dest[output_index] = '\0';
}

/**
 * Removes all spaces in a given string.
 *
 * @param dest The destination character array where the modified string will be stored.
 * @param src The src string to process.
 */
static void
string_remove_spaces (char *dest, const char *src)
{
  uint32_t len = (uint32_t)strlen (src);
  uint32_t output_index = 0;

  for (uint32_t i = 0; i < len; ++i)
    {
      if (isspace ((int32_t)src[i]) == 0)
        {
          dest[output_index++] = src[i];
        }
    }

  dest[output_index] = '\0';
}

/**
 * Tokenizes a declaration into its parameters.
 *
 * This function assumes that the input is valid and follows the format:
 * \<function_name\> (\<parameters\>)
 * Example: add(int x, int y)
 *
 * @param line The line to tokenize.
 * @param declaration Pointer to the declaration information structure where the results will be stored.
 * @return true if the parameters were successfully tokenized, false otherwise.
 */
static bool
line_tokenize_parameters (const char *line, declaration_t *declaration)
{
  bool result = true;

  /* Find the position of the opening parenthesis "(" and the closing parenthesis ")" */
  const char *open_parenthesis = strchr (line, '(');
  const char *close_parenthesis = strchr (line, ')');

  if ((open_parenthesis != NULL) && (close_parenthesis != NULL) && (open_parenthesis < close_parenthesis))
    {
      char args_copy[LENGTH_PARAMETER_NAME] = { 0 };
      (void)strncpy (args_copy, open_parenthesis + 1, (uint32_t)(close_parenthesis - open_parenthesis) - 1U);

      uint32_t num_params = 0;
      char *save_ptr = NULL;

      const char *param = strtok_r (args_copy, ",", &save_ptr);
      while (result && (param != NULL))
        {
          char trimmed_param[LENGTH_PARAMETER_NAME] = { 0 };
          string_remove_spaces (trimmed_param, param);

          if (num_params < MAX_PARAMETERS)
            {
              (void)strcpy (declaration->parameters[num_params], trimmed_param);
              param = strtok_r (NULL, ",", &save_ptr);
              ++num_params;
            }
          else
            {
              (void)fprintf (stderr, "Error: Maximum number of parameters reached for '%s'\n", line);
              result = false;
            }
        }

      declaration->parameters_count = num_params;
    }
  else
    {
      declaration->parameters[0][0] = '\0';
      (void)fprintf (stderr, "Error: Invalid function declaration '%s'\n", line);
      result = false;
    }

  return result;
}

/**
 * Checks if a line of code is part of a function prototype.
 *
 * @param line The line of code to check.
 * @param out A buffer to store the function out.
 * @param is_multiple A pointer to a boolean flag indicating if the function out spans multiple lines.
 * @return true if the line is part of a function out, false otherwise.
 */
static bool
line_get_prototype (const char *line, char *out, bool *is_multiple)
{
  bool result = false;

  if (*is_multiple)
    {
      /* Continue capturing lines until the end of the out */
      (void)strcat (out, line);

      /* Check for the end of a multi-line out */
      if (strchr (out, CHAR_PROTOTYPE_END) != NULL)
        {
          *is_multiple = false;
          result = true;
        }
    }
  else
    {
      if (strstr (line, LIBRARY_PREFIX_API) == line)
        {
          /* Check for the start of a function out */
          (void)strcpy (out, line);

          /* Check for the end of a single-line out */
          if (strchr (line, CHAR_PROTOTYPE_END) != NULL)
            {
              result = true;
            }
          else
            {
              *is_multiple = true;
            }
        }
    }

  return result;
}

/**
 * Checks if a line of code is part of a function prototype.
 *
 * @param line The line of code to check.
 * @param out A buffer to store the function out.
 * @param is_multiple A pointer to a boolean flag indicating if the function out spans multiple lines.
 * @return true if the line is part of a function out, false otherwise.
 */
static bool
line_get_definition (const char *line, char *out, bool *is_multiple, const char *name)
{
  bool result = false;

  if (*is_multiple)
    {
      /* Continue capturing lines until the end of the out */
      (void)strcat (out, line);

      /* If it contains a semicolon, we should stop immediately. */
      if (strchr (line, CHAR_PROTOTYPE_END) != NULL)
        {
          *is_multiple = false;
          result = false;
        }
      else
        {
          /* Check for the end of a multi-line out */
          if (strchr (out, CHAR_IMPLEMENTATION_START) != NULL)
            {
              *is_multiple = false;
              result = true;
            }
        }
    }
  else
    {
      if (line_find_word (line, name))
        {
          /* Check for the start of a function out */
          (void)strcpy (out, line);

          /* If it contains a semicolon, we should stop immediately. */
          if (strchr (line, CHAR_PROTOTYPE_END) == NULL)
            {
              /* Check for the end of a single-line out */
              if (strchr (line, CHAR_IMPLEMENTATION_START) != NULL)
                {
                  result = true;
                }
              else
                {
                  *is_multiple = true;
                }
            }
        }
    }

  return result;
}

/**
 * Checks if a line of code is part of a function test.
 *
 * @param line The line of code to check.
 * @param out A buffer to store the function out.
 * @param is_multiple A pointer to a boolean flag indicating if the function out spans multiple lines.
 * @param name The function name to search for.
 * @return true if the line is part of a function out, false otherwise.
 */
static bool
line_get_function_test (const char *line, char *out, bool *is_multiple, const char *name)
{
  bool result = false;

  if (*is_multiple)
    {
      /* Continue capturing lines until the end of the out */
      (void)strcat (out, line);
      string_remove_spaces (out, out);

      /* Check for the end of a multi-line out by name */
      if (line_find_word (out, name))
        {
          *is_multiple = false;
          result = true;
        }

      /* Check for the end of a multi-line out */
      if ((result == false) && (strchr (out, CHAR_TEST_END) != NULL))
        {
          *is_multiple = false;
          result = false;
        }
    }
  else
    {
      if (strstr (line, FORMAT_TEST) != NULL)
        {
          /* Check for the start of a function out */
          (void)strcpy (out, line);
          string_remove_spaces (out, out);

          /* Check for the end of a single-line out */
          if (line_find_word (out, name))
            {
              result = true;
            }
          else
            {
              *is_multiple = true;
            }
        }
    }

  return result;
}

/**
 * Extracts the function name from a declaration.
 *
 * This function assumes that the input is valid and has the format:
 * \<function_name\> (\<arguments\>)
 * Example: add(int x, int y)
 *
 * @param line The input C function declaration.
 * @param out Pointer to a character buffer where the function name will be stored.
 * @return true if a function name was successfully extracted, false otherwise.
 */
static bool
line_get_function_name (const char *line, char *out)
{
  bool result = false;

  /* Find the position of the opening parenthesis */
  const char *start = strchr (line, '(');

  if (start != NULL)
    {
      /* Move back one character to the potential end of function name */
      --start;

      /* Skip any spaces or tabs before the function name */
      while ((start >= line) && ((*start == ' ') || (*start == '\t')))
        {
          --start;
        }

      const char *end = start;

      /* Move back to find the start of the function name */
      while (strchr (" \t*&(", *end) == NULL)
        {
          --end;
        }

      uint32_t length = (uint32_t)(start - end);
      if (length > 0)
        {
          /* Copy the function name to the output buffer and null-terminate it */
          (void)strncpy (out, end + 1, length);
          out[length] = '\0';
          result = true;
        }
    }

  return result;
}

/**
 * Finds a word in a given line, considering word boundaries.
 *
 * @param line A pointer to the null-terminated string representing the line to search.
 * @param word A pointer to the null-terminated string representing the word to find.
 * @return true if the word is found as a whole word (surrounded by spaces, parentheses, or ending with double
 * underscores) in the word, false otherwise.
 */
static bool
line_find_word (const char *line, const char *word)
{
  bool result = false;

  /* Get the length of the word and the line */
  uint32_t word_len = (uint32_t)strlen (word);
  uint32_t line_len = (uint32_t)strlen (line);

  /* Calculate the maximum index to iterate */
  uint32_t max_index = word_len > line_len ? 0 : (line_len - word_len);

  /* Loop through the line string */
  for (uint32_t i = 0; i <= max_index; ++i)
    {
      /* Check if the current substring matches the word */
      if (strncmp (line + i, word, word_len) == 0)
        {
          const char *before = &line[i - 1];
          const char *after = &line[i + word_len];

          if (((i == 0) || char_is_word_boundary (*before)) && (char_is_word_boundary (*after) || (strncmp (after, "__", 2) == 0)))
            {
              result = true;
              break;
            }
        }
    }

  return result;
}

/**
 * Determines whether a line is a comment, including both single-line and block comments.
 *
 * @param line The line to check for comments.
 * @param is_block A flag indicating if the line is within a block comment.
 *
 * @return true if the line is a comment, false otherwise.
 */
static bool
line_is_comment (const char *line, bool *is_block)
{
  bool result = false;

  /* Empty or newline line */
  if ((line[0] == '\0') || (line[0] == '\n') || (strncmp (line, MARKER_COMMENT, 2) == 0))
    {
      result = true;
    }
  else
    {
      /* Block comment opener at the start */
      if (strncmp (line, MARKER_COMMENT_START, 2) == 0)
        {
          /* No block comment closer, set flag to true */
          if (strstr (line, MARKER_COMMENT_END) == NULL)
            {
              *is_block = true;
            }
          result = true;
        }
      else
        {
          /* Pointer to the last three characters */
          const char *line_end = line + (strlen (line) - 3U);

          /* Block comment opener at the end */
          if (strstr (line, MARKER_COMMENT_START) == line_end)
            {
              /* Set flag to true and update result to false */
              *is_block = true;
              result = false;
            }
          else
            {
              /* Block comment closer at the end or start and flag is true */
              if ((*is_block) && ((strstr (line, MARKER_COMMENT_END) == line_end) || (strncmp (line, MARKER_COMMENT_END, 2) == 0)))
                {
                  /* Set flag to false and update result to true */
                  *is_block = false;
                  result = true;
                }
              else
                {
                  /* Set result to flag value if none of the conditions above are met */
                  result = *is_block;
                }
            }
        }
    }

  return result;
}

/**
 * Adds a function prototype line to an array if space is available.
 *
 * This function extracts function signature information from the provided line and adds it to an array of prototypes.
 *
 * @param line A string with the function prototype line to add. It should include LIBRARY_PREFIX_API for correct
 * processing.
 * @note The line should follow the format: \<return_type\> \<function_name\> (\<arguments\>) as per C function
 * prototypes.
 * @note If MAX_PROTOTYPES is reached, the line won't be added.
 * @return true if the function prototype is successfully added, false otherwise.
 */
static bool
g_declarations_append (const char *line, uint32_t line_number)
{
  bool result = false;

  char signature[LENGTH_LINE] = { 0 };
  string_normalize_spaces (signature, line + strlen (LIBRARY_PREFIX_API));

  if (g_declarations_count < MAX_PROTOTYPES)
    {
      declaration_t *prototype = &g_declarations[g_declarations_count];
      if (line_get_function_name (signature, prototype->function_name) && line_tokenize_parameters (signature, prototype))
        {
          prototype->declaration_line_number = line_number;
          ++g_declarations_count;
          result = true;
        }
    }
  else
    {
      (void)fprintf (stderr, "Error: Maximum number of prototypes reached.\n");
    }

  return result;
}

/**
 * Updates test coverage information for function declarations.
 *
 * @param test_file A pointer to the FILE structure representing the test file to read.
 * @param test_path A pointer to the null-terminated string representing the path to the test.
 * @return true if the test coverage information is successfully updated, false otherwise.
 */
static bool
g_declarations_update_tests (FILE *test_file, const char *test_path)
{
  bool result = true;

  for (uint32_t i = 0; i < g_declarations_count; ++i)
    {
      declaration_t *declaration = &g_declarations[i];

      char annotation[LENGTH_LINE] = { 0 };
      (void)snprintf (annotation, LENGTH_LINE, FORMAT_TEST_ANNOTATION, declaration->function_name);

      bool in_block_comment = false;
      bool in_multi_line = false;
      char line_buffer[LENGTH_LINE] = { 0 };
      char declaration_buffer[LENGTH_LINE] = { 0 };

      uint32_t test_line_number = 0U;
      uint32_t line_number = 0U;
      int32_t line_size = (int32_t)sizeof (line_buffer);

      while (result && (fgets (line_buffer, line_size, test_file) != NULL))
        {
          ++line_number;

          /* Check if the line is a comment or inside a block comment */
          if (line_is_comment (line_buffer, &in_block_comment))
            {
              /* Check if the line contains a coverage annotation */
              if ((strstr (line_buffer, annotation) != NULL) && (declaration_add_coverage (declaration, test_path, line_number, true) == false))
                {
                  result = false;
                }
            }
          else
            {
              if (strstr (line_buffer, FORMAT_TEST) == line_buffer)
                {
                  test_line_number = line_number;
                }

              /* Check if the line is part of a function declaration */
              if (line_get_function_test (line_buffer, declaration_buffer, &in_multi_line, declaration->function_name)
                  && (declaration_add_coverage (declaration, test_path, test_line_number, false) == false))
                {
                  result = false;
                }
            }
        }

      rewind (test_file);
    }

  return result;
}

/**
 * Maps prototype definitions from a source file to their implementations.
 *
 * @param src_file A pointer to the source file from which to extract function prototypes.
 * @param src_path A path to the source file.
 * @param extension_position The position of the source extension in path.
 */
static void
g_declarations_update_definitions (FILE *src_file, const char *src_path, const char *expected_test_path)
{
  for (uint32_t i = 0; i < g_declarations_count; ++i)
    {
      declaration_t *declaration = &g_declarations[i];
      if (declaration->definition_line_number > 0)
        {
          continue;
        }

      bool in_block_comment = false;
      bool in_multi_line = false;
      char line_buffer[LENGTH_LINE] = { 0 };
      char declaration_buffer[LENGTH_LINE] = { 0 };

      uint32_t line_number = 0;
      int32_t line_size = (int32_t)sizeof (line_buffer);

      while (fgets (line_buffer, line_size, src_file) != NULL)
        {
          ++line_number;

          if ((line_is_comment (line_buffer, &in_block_comment) == false)
              && line_get_definition (line_buffer, declaration_buffer, &in_multi_line, declaration->function_name))
            {
              declaration->definition_line_number = line_number;
              declaration_update_validation (declaration, declaration_buffer);
              (void)strcpy (declaration->source_path, src_path);
              (void)strcpy (declaration->expected_test_path, expected_test_path);
              declaration->has_test_file = file_exists (g_declarations[i].expected_test_path);

              break;
            }
        }

      rewind (src_file);
    }
}

/**
 * Appends a test to the list of defined tests.
 *
 * This function adds a test with the specified test path and line number to the
 * list of defined tests, if the maximum limit has not been reached.
 *
 * @param test_path The path of the test.
 * @param line_number The line number of the test.
 * @return true if the test was successfully appended, false if the maximum limit
 * has been reached.
 */
static bool
g_tests_append (const char *test_path, uint32_t line_number)
{
  bool result = false;

  if (g_tests_count < MAX_TESTS)
    {
      coverage_t *test = &g_tests[g_tests_count];
      (void)strcpy (test->test_path, test_path);
      test->line = line_number;
      ++g_tests_count;

      result = true;
    }
  else
    {
      (void)fprintf (stderr, "Error: Maximum number of tests reached.");
    }

  return result;
}

/**
 * Removes a test from the list of defined tests.
 *
 * This function removes the test at the specified index from the list of defined
 * tests, adjusting the list accordingly.
 *
 * @param index The index of the test to be removed.
 */
static void
g_tests_remove (uint32_t index)
{
  for (uint32_t i = index; i < (g_tests_count - 1U); ++i)
    {
      g_tests[i] = g_tests[i + 1U];
    }

  --g_tests_count;
}

/**
 * Updates all test coverage information.
 *
 * @param test_file A pointer to the FILE structure representing the test file to read.
 * @param test_path A pointer to the null-terminated string representing the path to the test.
 * @return true if the test coverage information is successfully updated, false otherwise.
 */
static bool
g_tests_update_data (FILE *test_file, const char *test_path)
{
  bool result = true;

  bool in_block_comment = false;
  char line_buffer[LENGTH_LINE] = { 0 };

  uint32_t line_number = 0U;
  int32_t line_size = (int32_t)sizeof (line_buffer);

  while (result && (fgets (line_buffer, line_size, test_file) != NULL))
    {
      ++line_number;

      /* Check if the line is a comment or inside a block comment */
      if (line_is_comment (line_buffer, &in_block_comment))
        {
          continue;
        }
      else
        {
          if (strstr (line_buffer, FORMAT_TEST) == line_buffer)
            {
              result = g_tests_append (test_path, line_number);
            }
        }
    }

  rewind (test_file);

  return result;
}

/**
 * Filters defined tests based on coverage information.
 *
 * This function iterates through declarations and their coverage information,
 * removing tests that match the same test path and line number. It skips
 * tests associated with annotations.
 */
static void
g_tests_filter_defined (void)
{
  const declaration_t *declaration = NULL;
  for (uint32_t i = 0; i < g_declarations_count; ++i)
    {
      declaration = &g_declarations[i];

      for (uint32_t j = 0; j < g_declarations[i].coverages_count; ++j)
        {
          for (uint32_t k = 0; k < g_tests_count; ++k)
            {
              if (declaration->coverages[j].is_annotation)
                {
                  continue;
                }

              if ((strcmp (declaration->coverages[j].test_path, g_tests[k].test_path) == 0) && (g_tests[k].line == declaration->coverages[j].line))
                {
                  g_tests_remove (k);
                  break;
                }
            }
        }
    }
}

/**
 * Adds coverage information to a declaration if space is available.
 *
 * @param declaration Pointer to declaration_t structure.
 * @param test_path Test path string.
 * @param line_number Line number where the test was executed.
 * @param is_annotation Marks the coverage as an annotation.
 * @return True if coverage added successfully, false if array is full.
 */
static bool
declaration_add_coverage (declaration_t *declaration, const char *test_path, uint32_t line_number, bool is_annotation)
{
  bool result = false;

  if (declaration->coverages_count < MAX_TEST_BRANCHES)
    {
      coverage_t *coverage = &declaration->coverages[declaration->coverages_count];
      (void)strcpy (coverage->test_path, test_path);
      coverage->line = line_number;
      coverage->is_annotation = is_annotation;
      ++declaration->coverages_count;

      if (is_annotation)
        {
          ++declaration->coverage_annotation_count;
        }

      result = true;
    }
  else
    {
      (void)fprintf (stderr, "Error: Maximum number of coverage reached.");
    }

  return result;
}

/**
 * Validates the parameters of a function declaration.
 *
 * @param declaration A pointer to the `declaration_t` structure to validate.
 * @param buffer A pointer to a string containing the function declaration to compare against.
 */
static void
declaration_update_validation (declaration_t *declaration, const char *buffer)
{
  bool result = true;

  declaration_t tmp_declaration = { 0 };
  char line_buffer[LENGTH_LINE] = { 0 };
  string_normalize_spaces (line_buffer, buffer);

  if (line_tokenize_parameters (line_buffer, &tmp_declaration) && (tmp_declaration.parameters_count == declaration->parameters_count))
    {
      for (uint32_t j = 0; j < declaration->parameters_count; ++j)
        {
          const char *tmp_parameter = tmp_declaration.parameters[j];
          const char *parameter = declaration->parameters[j];

          if (strcmp (tmp_parameter, parameter) != 0)
            {
              result = false;
              break;
            }
        }
    }

  declaration->is_prototype_match = result;
}

/**
 * Loads prototypes from a include_file.
 *
 * @param include_file A pointer to the include_file to read from.
 */
static bool
load_prototypes (void)
{
  bool result = true;

  FILE *include_file = fopen (g_path_include_file, "r");
  if (include_file == NULL)
    {
      (void)fprintf (stderr, "Error: Unable to open include file at '%s'\n", g_path_include_file);
    }
  else
    {
      bool in_multi_line = false;
      bool in_block_comment = false;
      char line_buffer[LENGTH_LINE] = { 0 };
      char declaration_buffer[LENGTH_LINE] = { 0 };

      uint32_t line_number = 0;
      int32_t line_size = (int32_t)sizeof (line_buffer);

      while (result && (fgets (line_buffer, line_size, include_file) != NULL))
        {
          ++line_number;

          /* Check if the line is a comment or inside a block comment */
          if (line_is_comment (line_buffer, &in_block_comment))
            {
              continue;
            }

          /* Check if the line is part of a function declaration */
          if (line_get_prototype (line_buffer, declaration_buffer, &in_multi_line))
            {
              result = g_declarations_append (declaration_buffer, line_number);
            }
        }

      (void)fclose (include_file);
    }

  return result;
}

/**
 * Loads and processes function definitions from source files in a directory.
 *
 * @return `true` if the loading and processing of function definitions were
 * successful; otherwise, it returns `false`. In case of any errors,
 * error messages are printed to stderr.
 */
static bool
load_definitions (void)
{
  bool result = true;

  DIR *source_directory = opendir (g_path_src_directory);
  if (source_directory == NULL)
    {
      (void)fprintf (stderr, "Error: Unable to open source directory at '%s'\n", g_path_src_directory);
      result = false;
    }
  else
    {
      const struct dirent *entry = readdir (source_directory);
      FILE *src_file = NULL;

      while (entry != NULL)
        {
          /* Only process files that end with LIBRARY_EXTENSION_SRC */
          const char *extension_position = strstr (entry->d_name, LIBRARY_EXTENSION_SRC);
          if (extension_position != NULL)
            {
              /* Attempt to open source file */
              char path_buffer[FILENAME_MAX] = { 0 };
              (void)snprintf (path_buffer, sizeof (path_buffer), "%s/%s", g_path_src_directory, entry->d_name);
              src_file = fopen (path_buffer, "r");

              if (src_file != NULL)
                {
                  char expected_test_path[FILENAME_MAX] = { 0 };
                  char test_name[LENGTH_FUNCTION_NAME] = { 0 };
                  (void)strncpy (test_name, entry->d_name, (uint32_t)(extension_position - entry->d_name));
                  test_name[extension_position - entry->d_name] = '\0';

                  (void)strncat (test_name, LIBRARY_EXTENSION_TEST, sizeof (test_name) - strlen (test_name) - 1U);
                  (void)snprintf (expected_test_path, FILENAME_MAX, "%s/%s", g_path_test_directory, test_name);

                  g_declarations_update_definitions (src_file, path_buffer, expected_test_path);
                  (void)fclose (src_file);
                }
              else
                {
                  (void)fprintf (stderr, "Error: Unable to open source file at '%s'\n", path_buffer);
                }
            }

          entry = readdir (source_directory);
        }

      (void)closedir (source_directory);
    }

  return result;
}

/**
 * Loads and processes test files from the specified test directory.
 *
 * This function attempts to open and process each file in the test directory
 * whose name ends with LIBRARY_EXTENSION_TEST. For each valid test file, it calls
 * the g_declarations_update_tests function to update the test declarations.
 *
 * @return true if all test files were successfully processed; false otherwise.
 *
 * @note The function prints error messages to stderr in case of failures, such as
 * being unable to open the test directory or a specific test file.
 */
static bool
load_tests (void)
{
  bool result = true;

  DIR *test_directory = opendir (g_path_test_directory);
  if (test_directory == NULL)
    {
      (void)fprintf (stderr, "Error: Unable to open test directory at '%s'\n", g_path_test_directory);
      result = false;
    }
  else
    {
      const struct dirent *entry = readdir (test_directory);
      FILE *test_file = NULL;

      while (result && (entry != NULL))
        {
          /* Only process files that end with LIBRARY_EXTENSION_TEST */
          const char *extension_position = strstr (entry->d_name, LIBRARY_EXTENSION_TEST);

          if (extension_position != NULL)
            {
              /* Attempt to open test file */
              char path_buffer[FILENAME_MAX] = { 0 };
              (void)snprintf (path_buffer, sizeof (path_buffer), "%s/%s", g_path_test_directory, entry->d_name);
              test_file = fopen (path_buffer, "r");

              if (test_file != NULL)
                {
                  result = g_tests_update_data (test_file, path_buffer) && g_declarations_update_tests (test_file, path_buffer);

                  (void)fclose (test_file);
                }
              else
                {
                  (void)fprintf (stderr, "Error: Unable to open test file at '%s'\n", path_buffer);
                }
            }

          entry = readdir (test_directory);
        }

      (void)closedir (test_directory);
    }

  return result;
}
