#define CLOVE_SUITE_NAME inspection
#include "clove-unit.h"
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Max array lengths */
#define MAX_PARAMETERS 16
#define MAX_DECLARATIONS 128

/* Max string lengths */
#define MAX_PARAMETER_NAME_LENGTH 64U
#define MAX_LINE_LENGTH 512U
#define MAX_FUNCTION_NAME_LENGTH 128U

/* Project configuration */
#define LIBRARY_API_PREFIX "API"
#define LIBRARY_PATH_INCLUDE "include/library.h"
#define LIBRARY_PATH_SRC "src"
#define LIBRARY_PATH_TEST "tests/src"

typedef struct
{
  char function_name[MAX_FUNCTION_NAME_LENGTH];
  char parameters[MAX_PARAMETERS][MAX_PARAMETER_NAME_LENGTH];
  size_t parameters_count;
} declaration_info_t;

static char g_path_include_file[FILENAME_MAX] = { 0 };
static char g_path_src_directory[FILENAME_MAX] = { 0 };
static char g_path_test_directory[FILENAME_MAX] = { 0 };

static declaration_info_t g_declarations[MAX_DECLARATIONS] = { 0 };
static size_t g_declaration_count = 0U;

/**
 * Determines whether a line is a comment, including both single-line and block comments.
 *
 * @param line             The line to check for comments.
 * @param is_block_comment A flag indicating if the line is within a block comment.
 *
 * @return true if the line is a comment, false otherwise.
 */
static bool
is_comment (const char *line, bool *is_block_comment)
{
  bool result = false;

  /* Empty or newline line */
  if ((line[0] == '\0') || (line[0] == '\n'))
    {
      result = true;
    }
  else
    {
      /* Block comment opener at the start */
      if (strncmp (line, "/*", 2) == 0)
        {
          /* No block comment closer, set flag to true */
          if (strstr (line, "*/") == NULL)
            {
              *is_block_comment = true;
            }
          result = true;
        }
      else
        {
          /* Pointer to the last three characters */
          const char *line_end = line + (strlen (line) - 3U);

          /* Block comment opener at the end */
          if (strstr (line, "/*") == line_end)
            {
              /* Set flag to true and update result to false */
              *is_block_comment = true;
              result = false;
            }
          else
            {
              /* Block comment closer at the end or start and flag is true */
              if (((strstr (line, "*/") == line_end) || (strncmp (line, "*/", 2) == 0)) && (*is_block_comment))
                {
                  /* Set flag to false and update result to true */
                  *is_block_comment = false;
                  result = true;
                }
              else
                {
                  /* Set result to flag value if none of the conditions above are met */
                  result = *is_block_comment;
                }
            }
        }
    }

  return result;
}

/**
 * Removes leading and trailing spaces from a line of code and returns a pointer to the trimmed line.
 *
 * @param line The line of code to trim.
 * @return A pointer to the trimmed line.
 */
static char *
trim_spaces (const char *line)
{
  char *result = NULL;

  if (line != NULL)
    {
      char *start = line;
      char *end = line + (strlen (line) - 1);

      /* Trim leading spaces */
      while (isspace (*start) != 0)
        {
          ++start;
        }

      /* All spaces, return an empty string */
      if (start > end)
        {
          result = line;
        }
      else
        {
          /* Trim trailing spaces */
          while ((end > start) && (isspace (*end) != 0))
            {
              --end;
            }

          /* Null-terminate the trimmed string */
          end[1] = '\0';

          result = start;
        }
    }

  return result;
}

/**
 * Replaces newline characters with spaces and reduces adjacent spaces to a single space in a given string.
 *
 * @param dest   The destination character array where the modified string will be stored.
 * @param source The source string to process.
 */
static void
process_string (char *dest, const char *source)
{
  size_t len = strlen (source);
  size_t output_index = 0;
  bool prev_space = false;

  for (size_t i = 0; i < len; ++i)
    {
      if (source[i] == '\n')
        {
          dest[output_index++] = ' ';
          prev_space = false;
        }
      else if (source[i] == ' ')
        {
          if (prev_space == false)
            {
              dest[output_index++] = ' ';
              prev_space = true;
            }
        }
      else
        {
          dest[output_index++] = source[i];
          prev_space = false;
        }
    }

  dest[output_index] = '\0';
}

/**
 * Checks if a line of code is part of a function out_declaration and returns the out_declaration.
 *
 * @param line The line of code to check.
 * @param out_declaration A buffer to store the function out_declaration.
 * @param is_multi_line A pointer to a boolean flag indicating if the function out_declaration spans multiple lines.
 * @return true if the line is part of a function out_declaration, false otherwise.
 */
static bool
is_function_declaration (const char *line, char *out_declaration, bool *is_multi_line)
{
  bool result = false;

  if (*is_multi_line)
    {
      /* Continue capturing lines until the end of the out_declaration */
      strcat (out_declaration, line);

      /* Check for the end of a multi-line out_declaration */
      if (strchr (out_declaration, ';') != NULL)
        {
          *is_multi_line = false;
          result = true;
        }
    }
  else
    {
      if (strstr (line, LIBRARY_API_PREFIX) == line)
        {
          /* Check for the start of a function out_declaration */
          (void)strcpy (out_declaration, line);

          /* Check for the end of a single-line out_declaration */
          if (strchr (line, ';') != NULL)
            {
              result = true;
            }
          else
            {
              *is_multi_line = true;
            }
        }
    }

  return result;
}

/**
 * Builds full paths for the include file, source directory, and test directory.
 *
 * @note Uses PROJECT_ROOT, LIBRARY_PATH_INCLUDE, LIBRARY_PATH_SRC,
 *       and LIBRARY_PATH_TEST to construct the paths.
 */
static void
build_project_paths (void)
{
  /* Construct the full path to the include file */
  (void)snprintf (g_path_include_file, sizeof (g_path_include_file), "%s/%s", PROJECT_ROOT, LIBRARY_PATH_INCLUDE);

  /* Construct the full path to the source directory */
  (void)snprintf (g_path_src_directory, sizeof (g_path_src_directory), "%s/%s", PROJECT_ROOT, LIBRARY_PATH_SRC);

  /* Construct the full path to the test directory */
  (void)snprintf (g_path_test_directory, sizeof (g_path_test_directory), "%s/%s", PROJECT_ROOT, LIBRARY_PATH_TEST);
}

/**
 * Tokenizes a function signature into its name and parameters.
 *
 * This function assumes that the signature is valid and follows the format:
 * <return_type> <function_name> (<parameters>)
 * Example: int add(int x, int y)
 *
 * @param signature The function signature to tokenize.
 * @param declaration Pointer to the declaration information structure where the results will be stored.
 * @return true if the function signature was successfully tokenized, false otherwise.
 */
static bool
tokenize_declaration_parameters (const char *signature, declaration_info_t *declaration)
{
  bool result = true;

  /* Find the position of the opening parenthesis "(" and the closing parenthesis ")" */
  const char *open_parenthesis = strchr (signature, '(');
  const char *close_parenthesis = strchr (signature, ')');

  if ((open_parenthesis != NULL) && (close_parenthesis != NULL) && (open_parenthesis < close_parenthesis))
    {
      char args_copy[MAX_PARAMETER_NAME_LENGTH] = { 0 };
      (void)strncpy (args_copy, open_parenthesis + 1, (size_t)(close_parenthesis - open_parenthesis) - 1U);

      size_t num_params = 0;
      char *save_ptr = NULL;

      const char *param = strtok_r (args_copy, ",", &save_ptr);
      while (result && (param != NULL))
        {
          const char *trimmed_param = trim_spaces (param);

          if ((trimmed_param != NULL) && (strlen (trimmed_param) < MAX_PARAMETER_NAME_LENGTH))
            {
              if (num_params < MAX_PARAMETERS)
                {
                  (void)strcpy (declaration->parameters[num_params], trimmed_param);
                  param = strtok_r (NULL, ",", &save_ptr);
                  ++num_params;
                }
              else
                {
                  (void)fprintf (stderr, "Error: Maximum number of parameters reached for '%s'\n", signature);
                  result = false;
                }
            }
          else
            {
              (void)fprintf (stderr, "Error: Parameter name is too long '%s'\n", trimmed_param);
              result = false;
            }
        }

      declaration->parameters_count = num_params;
    }
  else
    {
      declaration->parameters[0][0] = '\0';
      (void)fprintf (stderr, "Error: Invalid function signature '%s'\n", signature);
      result = false;
    }

  return result;
}

/**
 * Extracts the function name from a C function declaration.
 *
 * This function assumes that the declaration is valid and has the format:
 * <return_type> <function_name> (<arguments>)
 * Example: int add(int x, int y)
 *
 * @param signature The input C function declaration.
 * @param out_name Pointer to a character buffer where the function name will be stored.
 * @return true if a function name was successfully extracted, false otherwise.
 */
static bool
extract_function_name (const char *signature, char *out_name)
{
  bool result = false;

  /* Find the position of the opening parenthesis */
  const char *start = strchr (signature, '(');

  if (start != NULL)
    {
      /* Move back one character to the potential end of function name */
      --start;

      /* Skip any spaces or tabs before the function name */
      while ((start >= signature) && ((*start == ' ') || (*start == '\t')))
        {
          --start;
        }

      const char *end = start;

      /* Move back to find the start of the function name */
      while (strchr (" \t*&(", *end) == NULL)
        {
          --end;
        }

      size_t length = (size_t)(start - end);
      if (length > 0)
        {
          /* Copy the function name to the output buffer and null-terminate it */
          (void)strncpy (out_name, end + 1, length);
          out_name[length] = '\0';
          result = true;
        }
    }

  return result;
}

/**
 * Adds a function declaration line to an array if space is available.
 *
 * This function extracts function signature information from the provided line and adds it to an array of declarations.
 *
 * @param line A string with the function declaration line to add. It should include LIBRARY_API_PREFIX for correct
 * processing.
 * @note The line should follow the format: <return_type> <function_name> (<arguments>) as per C function declarations.
 * @note If MAX_DECLARATIONS is reached, the line won't be added.
 * @return true if the function declaration is successfully added, false otherwise.
 */
static bool
add_function_declaration (const char *line)
{
  bool result = false;

  char signature[MAX_LINE_LENGTH] = { 0 };
  process_string (signature, line + strlen (LIBRARY_API_PREFIX));

  if (g_declaration_count < MAX_DECLARATIONS)
    {
      declaration_info_t *declaration = &g_declarations[g_declaration_count];
      if (tokenize_declaration_parameters (signature, declaration)
          && extract_function_name (signature, declaration->function_name))
        {
          ++g_declaration_count;
          result = true;
        }
    }
  else
    {
      fprintf (stderr, "Error: Maximum number of declarations reached.\n");
    }

  return result;
}

/**
 * Loads declarations from a include_file.
 *
 * @param include_file A pointer to the include_file to read from.
 */
static bool
load_function_declarations (void)
{
  bool result = true;

  FILE *include_file = fopen (g_path_include_file, "r");
  if (include_file == NULL)
    {
      (void)fprintf (stderr, "Error: Unable to open include file at '%s'\n", g_path_include_file);
    }
  else
    {
      char line[MAX_LINE_LENGTH] = { 0 };
      char declaration[MAX_LINE_LENGTH] = { 0 };
      bool in_multi_line_declaration = false;
      bool in_block_comment = false;

      while (result && (fgets (line, sizeof (line), include_file) != NULL))
        {
          /* Check if the line is a comment or inside a block comment */
          if (is_comment (line, &in_block_comment))
            {
              continue;
            }

          /* Check if the line is part of a function declaration */
          if (is_function_declaration (line, declaration, &in_multi_line_declaration))
            {
              /* Add the function declaration to the array without the prefix, store the result */
              result = add_function_declaration (declaration);
            }
        }

      fclose (include_file);
    }

  return result;
}

static bool
load_implementation_statuses (void)
{
  bool result = false;

  const DIR *source_directory = opendir (g_path_src_directory);
  if (source_directory == NULL)
    {
      (void)fprintf (stderr, "Error: Unable to open source directory at '%s'\n", g_path_src_directory);
    }
  else
    {
      (void)closedir (source_directory);
      result = true;
    }

  return result;
}

CLOVE_TEST (test)
{
  build_project_paths ();

  if ((load_function_declarations () == false) || (load_implementation_statuses () == false))
    {
      CLOVE_FAIL ();
      return;
    }

  for (size_t i = 0; i < g_declaration_count; ++i)
    {
      printf ("\nFunction: %s\n", g_declarations[i].function_name);
      for (size_t j = 0; j < g_declarations[i].parameters_count; ++j)
        {
          printf ("Argument %d: %s\n", j, g_declarations[i].parameters[j]);
        }
    }

  CLOVE_PASS ();
}
