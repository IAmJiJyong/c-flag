# flag.h

A simple, header-only C library for parsing command-line arguments.

## Features

- **Header-only:** Just include `flag.h` in your project.
- **Supported Types:** `bool`, `int32_t`, `int64_t`, `float`, `double`, `string`, and lists of strings.
- **Short and Long Options:** Supports both `-f` and `--flag` style arguments.
- **Automatic Help Generation:** Generates a usage message from your flag definitions.
- **Error Reporting:** Provides simple error messages for parsing failures.
- **Cross-Platform:** Written in standard C.

## Usage

To use the library, simply include the `flag.h` header file in your C source code.

### Example

Here is a simple example demonstrating how to define and parse flags:

```c
#include "flag.h"
#include <stdbool.h>
#include <stdio.h>

// Helper function to print a list of strings
void
print_list(const char** items, size_t count)
{
    printf("[");
    for (size_t i = 0; i < count; ++i) {
        if (i > 0)
            printf(", ");
        printf("\"%s\"", items[i]);
    }
    printf("]\n");
}

int
main(int argc, char** argv)
{
    // Define flags
    bool*     help   = flag_bool("help", false, "Show this usage message", 'h');
    int*      port   = flag_int32("port", 8080, "Port to listen on", 'p');
    char**    name   = flag_string("name", "world", "A name to greet", 'n');
    char***   files  = flag_list("file", "Files to process", 'f');

    // Parse command-line arguments
    if (!flag_parse(argc, argv)) {
        flag_print_error(stderr);
        return 1;
    }

    // If -h or --help was passed, print usage and exit
    if (*help) {
        flag_print_options(stdout);
        return 0;
    }

    // Print the values of the flags
    printf("Configuration:\n");
    printf("  Port: %d\n", *port);
    printf("  Name: %s\n", *name);
    printf("  Files: ");
    print_list((const char**)(*files), flag_list_len(*files));

    // Print the rest of the arguments
    printf("\nRemaining arguments:\n  ");
    print_list((const char**)flag_rest_argv(), flag_rest_argc());

    return 0;
}
```

### Building the Example

You can compile the example with a C compiler like `gcc` or `clang`.

```sh
# Using gcc
gcc your_program.c -o your_program

# Using clang
clang your_program.c -o your_program
```

### Running the Example

Here are a few ways you can run the compiled program:

```sh
# Run with default values
$ ./your_program
Configuration:
  Port: 8080
  Name: world
  Files: []

Remaining arguments:
  []

# Run with custom values
$ ./your_program --port 9000 -n "Gemini" -f one.txt --file two.txt other_arg
Configuration:
  Port: 9000
  Name: Gemini
  Files: ["one.txt", "two.txt"]

Remaining arguments:
  ["other_arg"]

# Show the help message
$ ./your_program --help
Usage: ./your_program [OPTIONS] [--] [ARGS]
Options:
  -h, --help
    Show this usage message
    Default: false
  -p, --port <int32>
    Port to listen on
    Default: 8080
  -n, --name <string>
    A name to greet
    Default: world
  -f, --file <string> ...
    Files to process
```

## API Reference

### Flag Definition

Define flags using the `flag_<type>` functions. Each function returns a pointer to the variable that will hold the flag's value after parsing.

- `bool* flag_bool(const char* name, bool default_value, const char* desc, char short_name)`
- `int32_t* flag_int32(const char* name, int32_t default_value, const char* desc, char short_name)`
- `int64_t* flag_int64(const char* name, int64_t default_value, const char* desc, char short_name)`
- `float* flag_float(const char* name, float default_value, const char* desc, char short_name)`
- `double* flag_double(const char* name, double default_value, const char* desc, char short_name)`
- `char** flag_string(const char* name, const char* default_value, const char* desc, char short_name)`
- `char*** flag_list(const char* name, const char* desc, char short_name)`

Use `FLAG_NO_SHORT_NAME` for the `short_name` parameter if you don't need a short option.

### Parsing

- `bool flag_parse(int argc, char** argv)`: Parses the command-line arguments from `main`. Returns `false` on error.

### Help and Errors

- `void flag_print_options(FILE* stream)`: Prints the auto-generated usage message to the specified stream.
- `void flag_print_error(FILE* stream)`: If `flag_parse` returned `false`, this function prints a descriptive error message to the specified stream.

### Accessing Values

- **Directly:** The pointers returned by the `flag_<type>` functions can be dereferenced to get the values after a successful `flag_parse`.
- **List Length:** `int flag_list_len(char** list)`: Returns the number of items in a list flag.
- **Remaining Arguments:**
  - `int flag_rest_argc()`: Returns the number of arguments remaining after flags have been parsed.
  - `char** flag_rest_argv()`: Returns an array of the remaining arguments.

## Inspiration

This project draws inspiration from the following YouTube videos:

- [Parsing Command Line Arguments in C (part 1)](https://www.youtube.com/watch?v=gtk3RZHwJUA)
- [Parsing Command Line Arguments in C (part 2)](https://www.youtube.com/watch?v=4JpgbdESzIU)

## License

This project is licensed under the MIT License.
