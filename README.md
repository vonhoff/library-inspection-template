# CLove-Unit C Library Template With Embedded Inspection

This template serves as a robust foundation for C libraries, incorporating a comprehensive set of checks for unit
testing and consistency. Specifically designed for seamless integration
with [CLove-Unit](https://github.com/fdefelici/clove-unit/), this template adheres to a set of rules to enhance the
reliability and maintainability of your project.

## Inspection Rules

1. **Full API Coverage:**
    - Ensure every API is covered, either through annotations or a dedicated Clove Unit test.

2. **Association of Tests with APIs:**
    - Every test should be directly related to a specific API.

3. **Consistent Test Naming:**
    - Adopt a consistent naming convention for tests, mirroring API function names. In cases of test variations, employ
      double underscores (e.g., "point_create," "point_create__on_null").

4. **Organized Test Files:**
    - Test file names should replicate the corresponding implementation file names, with the addition of ".test.c"
      instead of ".c".

5. **Matching Prototypes and Implementations:**
    - Ensure each API prototype declaration has a corresponding implementation.

6. **Parameter Consistency:**
    - Ensure that parameters in prototypes match their definitions exactly.

## Screenshot

![Report example](example.png)

## Project Structure

The root directory is organized into three key folders:

1. **Include Folder:**
    - Hosts a single header file providing the API for the library, defining interfaces for external use.

2. **Source Folder:**
    - Contains essential source code files necessary for implementing the library's functionality.

3. **Tests Folder:**
    - Encompasses CLove-Unit tests featuring an embedded API inspection suite.

## License

This template is distributed under the [MIT License](LICENSE), which grants you the freedom to use, modify, and
redistribute it according to your project's needs. Feel free to replace the license with one that better suits your
project's needs.

## Contribution

Contributions are welcome! If you have ideas for improving this template, please create issues or submit pull requests.
