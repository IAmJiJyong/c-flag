#include "flag.h"
#include <stdbool.h>
#include <stdio.h>

int
max(int a, int b)
{
        return (a > b) ? a : b;
}

void
print_list(const char** items, size_t count)
{
        printf("[");
        for (size_t i = 0; i < count; ++i) {
                if (i > 0)
                        printf(", ");
                printf("%s", items[i]);
        }
        printf("]\n");
}

int
main(int argc, char** argv)
{
        bool*     help   = flag_bool("help", false, "Show the usage", 'h');
        bool*     Bool   = flag_bool("bool", false, "Boolean flag", 'b');
        int*      int32  = flag_int32("int32", 123, "Int32 flag", 'i');
        long int* int64  = flag_int64("int64", 10000000, "Int64 flag", 'I');
        float*    Float  = flag_float("float", 3.14, "Float flag", 'f');
        double*   Double = flag_double("double", 1.141, "Doubel flag", 'd');
        char**    string = flag_string("string", "Hello World!", "String flag", 's');
        char***   list   = flag_list("list", "List flag", 'l');

        if (!flag_parse(argc, argv)) {
                flag_print_error(stderr);
                exit(-1);
        }

        if (*help) {
                flag_print_options(stdout);
                exit(0);
        }

        argc = flag_rest_argc();
        argv = flag_rest_argv();

        int n, width = 0;

        width = max(width, strlen(flag_name(help)));
        width = max(width, strlen(flag_name(Bool)));
        width = max(width, strlen(flag_name(int32)));
        width = max(width, strlen(flag_name(int64)));
        width = max(width, strlen(flag_name(Float)));
        width = max(width, strlen(flag_name(Double)));
        width = max(width, strlen(flag_name(string)));
        width = max(width, strlen(flag_name(list)));

        printf("--%-*s => %s \n", width, flag_name(help), *help ? "true" : "false");
        printf("--%-*s => %s \n", width, flag_name(Bool), *Bool ? "true" : "false");
        printf("--%-*s => %d \n", width, flag_name(int32), *int32);
        printf("--%-*s => %ld\n", width, flag_name(int64), *int64);
        printf("--%-*s => %f \n", width, flag_name(Float), *Float);
        printf("--%-*s => %lf\n", width, flag_name(Double), *Double);
        printf("--%-*s => %s \n", width, flag_name(string), *string);
        printf("--%-*s => ", width, flag_name(list));
        print_list((const char**)(*list), flag_list_len(*list));

        printf("%-*s   => ", width, "args");
        print_list((const char**)argv, argc);

        return 0;
}
