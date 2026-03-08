#ifndef __FLAG_H__
#define __FLAG_H__

#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLAG_NO_SHORT_NAME '\0'
#define FLAGS_CAPACITY     128
#define FLAG_LIST_CAPCITY  8

typedef struct
{
        size_t count;
        size_t capacity;
} FlagListHeader;

void
flag_list_append(char*** list, char* item)
{
        if (*list == NULL) {
                FlagListHeader* header =
                  (FlagListHeader*)malloc(sizeof(FlagListHeader) + sizeof(char*) * FLAG_LIST_CAPCITY);
                header->count    = 0;
                header->capacity = FLAG_LIST_CAPCITY;
                *list            = (char**)(header + 1);
        }

        FlagListHeader* header   = (FlagListHeader*)(*list) - 1;
        (*list)[header->count++] = item;
}

int
flag_list_len(char** list)
{
        if (!list)
                return 0;
        return ((FlagListHeader*)(list)-1)->count;
}

#define FLAG_TYPE_LIST                                                                                                 \
        FLAG_X(BOOL, bool, bool)                                                                                       \
        FLAG_X(INT32, int32_t, int32)                                                                                  \
        FLAG_X(INT64, int64_t, int64)                                                                                  \
        FLAG_X(FLOAT, float, float)                                                                                    \
        FLAG_X(DOUBLE, double, double)                                                                                 \
        FLAG_X(STRING, char*, string)                                                                                  \
        FLAG_X(LIST, char**, list)

typedef enum
{
#define FLAG_X(name, type, field) FLAG_##name,
        FLAG_TYPE_LIST
#undef FLAG_X
          COUNT_FLAG_TYPES,
} FlagType;

_Static_assert(COUNT_FLAG_TYPES == 7, "Exhaustive FlagValue definition");

typedef union
{
#define FLAG_X(name, type, field) type as_##field;
        FLAG_TYPE_LIST
#undef FLAG_X
} FlagValue;

typedef struct
{
        FlagType  type;
        char      short_name;
        char*     name;
        char*     desc;
        FlagValue value;
        FlagValue default_value;
        void*     reference;
} Flag;

typedef enum
{
        FLAG_NO_ERROR = 0,
        FLAG_ERROR_UNKNOWN,
        FLAG_ERROR_NO_VALUE,
        FLAG_ERROR_INVALID_NUMBER,
        FLAG_ERROR_INTEGER_OVERFLOW,
        FLAG_ERROR_FLOAT_OVERFLOW,
        FLAG_ERROR_DOUBLE_OVERFLOW,
        FLAG_ERROR_INVALID_SIZE_SUFFIX,
        COUNT_FLAG_ERRORS,
} FlagError;

typedef struct
{
        char*       program_name;
        Flag        flags[FLAGS_CAPACITY];
        size_t      flags_count;
        FlagError   error;
        const char* error_name;
        int         rest_argc;
        char**      rest_argv;
        bool        stop_parse;
} FlagContext;

static FlagContext flag_global_context;

static char*
flag_shift_args(int* argc, char*** argv)
{
        if (*argc == 0 || *argv == NULL || **argv == NULL)
                return NULL;
        char* result = **argv;
        (*argv)++;
        (*argc)--;
        return result;
}

static Flag*
__flag_new(FlagContext* flag_context, FlagType type, const char* name, const char* desc)
{
        assert(flag_context->flags_count < FLAGS_CAPACITY);
        Flag* flag = &flag_context->flags[flag_context->flags_count++];
        memset(flag, 0, sizeof(*flag));
        flag->type = type;
        flag->name = (char*)name;
        flag->desc = (char*)desc;
        return flag;
}

inline static void*
__flag_get_ref(Flag* flag)
{
        if (flag->reference) {
                return flag->reference;
        }
        return &flag->value;
}

int
flag_rest_argc();
char**
flag_rest_argv();
char*
flag_context_name(FlagContext* flag_context, void* value);
char*
flag_name(void* value);
bool
flag_context_parse(void* context, int argc, char** argv);
bool
flag_parse(int argc, char** argv);
bool*
flag_context_bool(FlagContext* flag_context, const char* name, bool default_value, const char* desc, char short_name);
bool*
flag_bool(const char* name, bool default_value, const char* desc, char short_name);
int32_t*
flag_context_int32(FlagContext* flag_context, const char* name, int32_t def, const char* desc, char short_name);
int32_t*
flag_int32(const char* name, int32_t default_value, const char* desc, char short_name);
int64_t*
flag_context_int64(FlagContext* flag_context, const char* name, int64_t def, const char* desc, char short_name);
int64_t*
flag_int64(const char* name, int64_t default_value, const char* desc, char short_name);
float*
flag_context_float(FlagContext* flag_context, const char* name, float default_value, const char* desc, char short_name);
float*
flag_float(const char* name, float default_value, const char* desc, char short_name);
double*
flag_context_double(FlagContext* flag_context, const char* name, double def, const char* desc, char short_name);
double*
flag_double(const char* name, double default_value, const char* desc, char short_name);
char**
flag_context_string(FlagContext* flag_context, const char* name, const char* def, const char* desc, char short_name);
char**
flag_string(const char* name, const char* default_value, const char* desc, char short_name);
char***
flag_context_list(FlagContext* flag_context, const char* name, const char* desc, char short_name);
char***
flag_list(const char* name, const char* desc, char short_name);
void
flag_context_print_error(FlagContext* flag_context, FILE* stream);
void
flag_print_error(FILE* stream);
void
flag_context_print_options(FlagContext* flag_context, FILE* stream);
void
flag_print_options(FILE* stream);

bool
parse_short_flags(FlagContext* flag_context, char* arg, int* argc, char*** argv)
{
        arg++;
        while (*arg) {
                Flag* flag = NULL;
                for (int i = 0; i < flag_context->flags_count; i++) {
                        if (*arg == flag_context->flags[i].short_name) {
                                flag = &flag_context->flags[i];
                                break;
                        }
                }

                if (!flag) {
                        flag_context->error      = FLAG_ERROR_UNKNOWN;
                        flag_context->error_name = arg;
                        return false;
                }

                *(bool*)__flag_get_ref(flag) = true;
        }

        return true;
}

bool
parse_short_flag(FlagContext* flag_context, char* arg, int* argc, char*** argv)
{
        arg++;

        Flag* flag = NULL;

        for (int i = 0; i < flag_context->flags_count; i++) {
                if (*arg == flag_context->flags[i].short_name) {
                        flag = &flag_context->flags[i];
                        break;
                }
        }

        if (!flag) {
                flag_context->error      = FLAG_ERROR_UNKNOWN;
                flag_context->error_name = arg - 1;
                return false;
        }

        switch (flag->type) {
                case FLAG_BOOL:
                        *(bool*)__flag_get_ref(flag) = true;
                        break;
                case FLAG_INT32: {
                        if (*argc <= 0) {
                                flag_context->error      = FLAG_ERROR_NO_VALUE;
                                flag_context->error_name = arg - 1;
                                return false;
                        }
                        char*         end_ptr;
                        char*         value  = flag_shift_args(argc, argv);
                        long long int result = strtoll(value, &end_ptr, 10);
                        if (*end_ptr != '\0') {
                                flag_context->error      = FLAG_ERROR_INVALID_NUMBER;
                                flag_context->error_name = value;
                                return false;
                        }

                        if (errno == ERANGE || result < INT32_MIN || result > INT32_MAX) {
                                flag_context->error      = FLAG_ERROR_INTEGER_OVERFLOW;
                                flag_context->error_name = value;
                                return false;
                        }
                        *(int32_t*)__flag_get_ref(flag) = result;
                } break;
                case FLAG_INT64: {
                        if (*argc <= 0) {
                                flag_context->error      = FLAG_ERROR_NO_VALUE;
                                flag_context->error_name = arg - 1;
                                return false;
                        }
                        char*         end_ptr;
                        char*         value  = flag_shift_args(argc, argv);
                        long long int result = strtoll(value, &end_ptr, 10);
                        if (*end_ptr != '\0') {
                                flag_context->error      = FLAG_ERROR_INVALID_NUMBER;
                                flag_context->error_name = value;
                                return false;
                        }

                        if (errno == ERANGE || result < INT64_MIN || result > INT64_MAX) {
                                flag_context->error      = FLAG_ERROR_INTEGER_OVERFLOW;
                                flag_context->error_name = value;
                                return false;
                        }
                        *(int32_t*)__flag_get_ref(flag) = result;
                } break;
                case FLAG_FLOAT: {
                        if (*argc <= 0) {
                                flag_context->error      = FLAG_ERROR_NO_VALUE;
                                flag_context->error_name = arg - 1;
                                return false;
                        }
                        char* end_ptr;
                        char* value  = flag_shift_args(argc, argv);
                        float result = strtof(value, &end_ptr);
                        if (*end_ptr != '\0') {
                                flag_context->error      = FLAG_ERROR_INVALID_NUMBER;
                                flag_context->error_name = value;
                                return false;
                        }

                        if (errno == ERANGE || result == FLT_MAX) {
                                flag_context->error      = FLAG_ERROR_FLOAT_OVERFLOW;
                                flag_context->error_name = value;
                                return false;
                        }
                        *(float*)__flag_get_ref(flag) = result;
                } break;
                case FLAG_DOUBLE: {
                        if (*argc <= 0) {
                                flag_context->error      = FLAG_ERROR_NO_VALUE;
                                flag_context->error_name = arg - 1;
                                return false;
                        }
                        char*  end_ptr;
                        char*  value  = flag_shift_args(argc, argv);
                        double result = strtod(value, &end_ptr);

                        if (*end_ptr != '\0') {
                                flag_context->error      = FLAG_ERROR_INVALID_NUMBER;
                                flag_context->error_name = value;
                                return false;
                        }

                        if (errno == ERANGE || result == DBL_MAX) {
                                flag_context->error      = FLAG_ERROR_DOUBLE_OVERFLOW;
                                flag_context->error_name = value;
                                return false;
                        }
                        *(double*)__flag_get_ref(flag) = result;
                } break;
                case FLAG_STRING: {
                        if (*argc <= 0) {
                                flag_context->error      = FLAG_ERROR_NO_VALUE;
                                flag_context->error_name = arg - 1;
                                return false;
                        }
                        char* value                   = flag_shift_args(argc, argv);
                        *(char**)__flag_get_ref(flag) = value;
                } break;
                case FLAG_LIST: {
                        if (*argc <= 0) {
                                flag_context->error      = FLAG_ERROR_NO_VALUE;
                                flag_context->error_name = arg - 1;
                                return false;
                        }
                        char* value = flag_shift_args(argc, argv);
                        flag_list_append((char***)__flag_get_ref(flag), value);
                } break;
                default:
                        assert(0 && "Unreachable");
                        exit(-1);
                        break;
        }

        return true;
}

bool
parse_long_flag(FlagContext* flag_context, char* arg, int* argc, char*** argv)
{
        char* name  = arg + 2;
        char* value = strchr(arg, '=');
        if (value) {
                *value = '\0';
                value++;
        }

        Flag* flag = NULL;
        for (int i = 0; i < flag_context->flags_count; i++) {
                if (strcmp(flag_context->flags[i].name, name) == 0) {
                        flag = &flag_context->flags[i];
                        break;
                }
        }

        if (!flag) {
                char error_name[64];
                sprintf(error_name, "--%s", name);
                flag_context->error      = FLAG_ERROR_UNKNOWN;
                flag_context->error_name = error_name;
                return false;
        }

        switch (flag->type) {
                case FLAG_BOOL:
                        *(bool*)__flag_get_ref(flag) = true;
                        break;
                case FLAG_INT32: {
                        if (!value) {
                                if (*argc <= 0) {
                                        char error_name[64];
                                        sprintf(error_name, "--%s", name);
                                        flag_context->error      = FLAG_ERROR_NO_VALUE;
                                        flag_context->error_name = error_name;
                                        return false;
                                }
                                value = flag_shift_args(argc, argv);
                        }

                        char*         end_ptr;
                        long long int result = strtoll(value, &end_ptr, 10);
                        if (*end_ptr != '\0') {
                                char error_name[64];
                                sprintf(error_name, "--%s", name);
                                flag_context->error      = FLAG_ERROR_INVALID_NUMBER;
                                flag_context->error_name = error_name;
                                return false;
                        }

                        if (errno == ERANGE || result < INT32_MIN || result > INT32_MAX) {
                                char error_name[64];
                                sprintf(error_name, "--%s", name);
                                flag_context->error      = FLAG_ERROR_INTEGER_OVERFLOW;
                                flag_context->error_name = error_name;
                                return false;
                        }

                        *(int32_t*)__flag_get_ref(flag) = (int32_t)result;
                } break;
                case FLAG_INT64: {
                        if (!value) {
                                if (*argc <= 0) {
                                        char error_name[64];
                                        sprintf(error_name, "--%s", name);
                                        flag_context->error      = FLAG_ERROR_NO_VALUE;
                                        flag_context->error_name = error_name;
                                        return false;
                                }
                                value = flag_shift_args(argc, argv);
                        }
                        char*         end_ptr;
                        long long int result = strtoll(value, &end_ptr, 10);
                        if (*end_ptr != '\0') {
                                char error_name[64];
                                sprintf(error_name, "--%s", name);
                                flag_context->error      = FLAG_ERROR_INVALID_NUMBER;
                                flag_context->error_name = error_name;
                                return false;
                        }

                        if (errno == ERANGE || result < INT64_MIN || result > INT64_MAX) {
                                char error_name[64];
                                sprintf(error_name, "--%s", name);
                                flag_context->error      = FLAG_ERROR_INTEGER_OVERFLOW;
                                flag_context->error_name = error_name;
                                return false;
                        }

                        *(int64_t*)__flag_get_ref(flag) = (int64_t)result;
                } break;
                case FLAG_FLOAT: {
                        if (!value) {
                                if (*argc <= 0) {
                                        char error_name[64];
                                        sprintf(error_name, "--%s", name);
                                        flag_context->error      = FLAG_ERROR_NO_VALUE;
                                        flag_context->error_name = error_name;
                                        return false;
                                }
                                value = flag_shift_args(argc, argv);
                        }
                        char* end_ptr;
                        float result = strtof(value, &end_ptr);
                        if (*end_ptr != '\0') {
                                char error_name[64];
                                sprintf(error_name, "--%s", name);
                                flag_context->error      = FLAG_ERROR_INVALID_NUMBER;
                                flag_context->error_name = error_name;
                                return false;
                        }
                        if (result == FLT_MAX && errno == ERANGE) {
                                char error_name[64];
                                sprintf(error_name, "--%s", name);
                                flag_context->error      = FLAG_ERROR_FLOAT_OVERFLOW;
                                flag_context->error_name = error_name;
                                return false;
                        }
                        *(float*)__flag_get_ref(flag) = result;
                } break;
                case FLAG_DOUBLE: {
                        if (!value) {
                                if (*argc <= 0) {
                                        char error_name[64];
                                        sprintf(error_name, "--%s", name);
                                        flag_context->error      = FLAG_ERROR_NO_VALUE;
                                        flag_context->error_name = error_name;
                                        return false;
                                }
                                value = flag_shift_args(argc, argv);
                        }
                        char*  end_ptr;
                        double result = strtod(value, &end_ptr);
                        if (*end_ptr != '\0') {
                                char error_name[64];
                                sprintf(error_name, "--%s", name);
                                flag_context->error      = FLAG_ERROR_INVALID_NUMBER;
                                flag_context->error_name = error_name;
                                return false;
                        }
                        if (errno == ERANGE) {
                                char error_name[64];
                                sprintf(error_name, "--%s", name);
                                flag_context->error      = FLAG_ERROR_DOUBLE_OVERFLOW;
                                flag_context->error_name = error_name;
                                return false;
                        }
                        *(double*)__flag_get_ref(flag) = result;
                } break;
                case FLAG_STRING: {
                        if (!value) {
                                if (*argc <= 0) {
                                        char error_name[64];
                                        sprintf(error_name, "--%s", name);
                                        flag_context->error      = FLAG_ERROR_NO_VALUE;
                                        flag_context->error_name = error_name;
                                        return false;
                                }
                                value = flag_shift_args(argc, argv);
                        }
                        *(char**)__flag_get_ref(flag) = value;
                } break;
                case FLAG_LIST: {
                        if (!value) {
                                if (*argc <= 0) {
                                        char error_name[64];
                                        sprintf(error_name, "--%s", name);
                                        flag_context->error      = FLAG_ERROR_NO_VALUE;
                                        flag_context->error_name = error_name;
                                        return false;
                                }
                                value = flag_shift_args(argc, argv);
                        }
                        flag_list_append((char***)__flag_get_ref(flag), value);
                } break;
                default:
                        assert(0 && "Unreachable");
                        exit(-1);
        }

        return true;
}

int
flag_rest_argc()
{
        return flag_global_context.rest_argc;
}

char**
flag_rest_argv()
{
        return flag_global_context.rest_argv;
}

char*
flag_context_name(FlagContext* flag_context, void* value)
{
        for (int i = 0; i < flag_context->flags_count; i++) {
                Flag* flag = &flag_context->flags[i];
                if (__flag_get_ref(flag) == value) {
                        return flag->name;
                }
        }
        return NULL;
}

char*
flag_name(void* value)
{
        return flag_context_name(&flag_global_context, value);
}

bool
flag_context_parse(void* context, int argc, char** argv)
{
        FlagContext* flag_context = (FlagContext*)context;

        if (flag_context->program_name == NULL) {
                flag_context->program_name = flag_shift_args(&argc, &argv);
        }

        while (argc > 0) {
                char* arg = flag_shift_args(&argc, &argv);

                if (*arg != '-') {
                        flag_global_context.rest_argc = argc;
                        flag_global_context.rest_argv = argv;
                        return true;
                }

                if (strcmp(arg, "--") == 0) {
                        flag_context->stop_parse = true;
                        flag_context->rest_argc  = argc;
                        flag_context->rest_argv  = argv;
                        return true;
                }

                if (arg[1] != '-') {
                        if (strlen(arg + 1) > 1) {
                                if (!parse_short_flags(flag_context, arg, &argc, &argv)) {
                                        return false;
                                }
                        } else if (!parse_short_flag(flag_context, arg, &argc, &argv)) {
                                return false;
                        }
                } else {
                        if (!parse_long_flag(flag_context, arg, &argc, &argv)) {
                                return false;
                        }
                }
        }

        return true;
}

bool
flag_parse(int argc, char** argv)
{
        return flag_context_parse(&flag_global_context, argc, argv);
}

bool*
flag_context_bool(FlagContext* flag_context, const char* name, bool default_value, const char* desc, char short_name)
{
        Flag* flag                  = __flag_new(flag_context, FLAG_BOOL, name, desc);
        flag->value.as_bool         = default_value;
        flag->default_value.as_bool = default_value;
        flag->short_name            = short_name;
        return &flag->value.as_bool;
}

bool*
flag_bool(const char* name, bool default_value, const char* desc, char short_name)
{
        return flag_context_bool(&flag_global_context, name, default_value, desc, short_name);
}

int32_t*
flag_context_int32(FlagContext* flag_context,
                   const char*  name,
                   int32_t      default_value,
                   const char*  desc,
                   char         short_name)
{
        Flag* flag                   = __flag_new(flag_context, FLAG_INT32, name, desc);
        flag->value.as_int32         = default_value;
        flag->default_value.as_int32 = default_value;
        flag->short_name             = short_name;
        return &flag->value.as_int32;
}

int32_t*
flag_int32(const char* name, int32_t default_value, const char* desc, char short_name)
{
        return flag_context_int32(&flag_global_context, name, default_value, desc, short_name);
}

int64_t*
flag_context_int64(FlagContext* flag_context,
                   const char*  name,
                   int64_t      default_value,
                   const char*  desc,
                   char         short_name)
{
        Flag* flag                   = __flag_new(flag_context, FLAG_INT64, name, desc);
        flag->value.as_int64         = default_value;
        flag->default_value.as_int64 = default_value;
        flag->short_name             = short_name;
        return &flag->value.as_int64;
}

int64_t*
flag_int64(const char* name, int64_t default_value, const char* desc, char short_name)
{
        return flag_context_int64(&flag_global_context, name, default_value, desc, short_name);
}

float*
flag_context_float(FlagContext* flag_context, const char* name, float default_value, const char* desc, char short_name)
{
        Flag* flag                   = __flag_new(flag_context, FLAG_FLOAT, name, desc);
        flag->value.as_float         = default_value;
        flag->default_value.as_float = default_value;
        flag->short_name             = short_name;
        return &flag->value.as_float;
}

float*
flag_float(const char* name, float default_value, const char* desc, char short_name)
{
        return flag_context_float(&flag_global_context, name, default_value, desc, short_name);
}

double*
flag_context_double(FlagContext* flag_context,
                    const char*  name,
                    double       default_value,
                    const char*  desc,
                    char         short_name)
{
        Flag* flag                    = __flag_new(flag_context, FLAG_DOUBLE, name, desc);
        flag->value.as_double         = default_value;
        flag->default_value.as_double = default_value;
        flag->short_name              = short_name;
        return &flag->value.as_double;
}
double*
flag_double(const char* name, double default_value, const char* desc, char short_name)
{
        return flag_context_double(&flag_global_context, name, default_value, desc, short_name);
}

char**
flag_context_string(FlagContext* flag_context,
                    const char*  name,
                    const char*  default_value,
                    const char*  desc,
                    char         short_name)
{
        Flag* flag                    = __flag_new(flag_context, FLAG_STRING, name, desc);
        flag->value.as_string         = (char*)default_value;
        flag->default_value.as_string = (char*)default_value;
        flag->short_name              = short_name;
        return &flag->value.as_string;
}

char**
flag_string(const char* name, const char* default_value, const char* desc, char short_name)
{
        return flag_context_string(&flag_global_context, name, default_value, desc, short_name);
}

char***
flag_context_list(FlagContext* flag_context, const char* name, const char* desc, char short_name)
{
        Flag* flag          = __flag_new(flag_context, FLAG_LIST, name, desc);
        flag->short_name    = short_name;
        flag->value.as_list = NULL;
        return &flag->value.as_list;
}

char***
flag_list(const char* name, const char* desc, char short_name)
{
        return flag_context_list(&flag_global_context, name, desc, short_name);
}

void
flag_context_print_error(FlagContext* flag_context, FILE* stream)
{
        flag_print_options(stderr);
        switch (flag_context->error) {
                case FLAG_NO_ERROR:
                        return;
                case FLAG_ERROR_UNKNOWN:
                        fprintf(stream, "ERROR: %s: unknown flag\n", flag_context->error_name);
                        break;
                case FLAG_ERROR_NO_VALUE:
                        fprintf(stream, "ERROR: %s: flag requires a value\n", flag_context->error_name);
                        break;
                case FLAG_ERROR_INVALID_NUMBER:
                        fprintf(stream, "ERROR: %s: invalid number for flag\n", flag_context->error_name);
                        break;
                case FLAG_ERROR_INTEGER_OVERFLOW:
                        fprintf(stream, "ERROR: %s: integer overflow for flag\n", flag_context->error_name);
                        break;
                case FLAG_ERROR_FLOAT_OVERFLOW:
                        fprintf(stream, "ERROR: %s: float overflow for flag\n", flag_context->error_name);
                        break;
                case FLAG_ERROR_DOUBLE_OVERFLOW:
                        fprintf(stream, "ERROR: %s: double overflow for flag\n", flag_context->error_name);
                        break;
                case FLAG_ERROR_INVALID_SIZE_SUFFIX:
                        fprintf(stream, "ERROR: %s: invalid size suffix for flag\n", flag_context->error_name);
                        break;
                default:
                        assert(0 && "Unreachable");
                        exit(-1);
        }
}

void
flag_print_error(FILE* stream)
{
        flag_context_print_error(&flag_global_context, stream);
}

void
flag_context_print_options(FlagContext* flag_context, FILE* stream)
{
        fprintf(stream, "Usage: %s [OPTIONS] [--] [ARGS]\n", flag_context->program_name);
        fprintf(stream, "Options:\n");
        for (int i = 0; i < flag_context->flags_count; i++) {
                Flag* flag = &flag_context->flags[i];
                switch (flag->type) {
                        case FLAG_BOOL:
                                if (flag->short_name) {
                                        fprintf(stream, "  -%c, --%s\n", flag->short_name, flag->name);
                                } else {
                                        fprintf(stream, "  %s\n", flag->name);
                                }
                                fprintf(stream, "    %s\n", flag->desc);
                                fprintf(stream, "    Default: %s\n", flag->default_value.as_bool ? "true" : "false");
                                break;
                        case FLAG_INT32:
                                if (flag->short_name) {
                                        fprintf(stream, "  -%c, --%s <int32>\n", flag->short_name, flag->name);
                                } else {
                                        fprintf(stream, "  %s <int32>\n", flag->name);
                                }
                                fprintf(stream, "    %s\n", flag->desc);
                                fprintf(stream, "    Default: %d\n", flag->default_value.as_int32);
                                break;
                        case FLAG_INT64:
                                if (flag->short_name) {
                                        fprintf(stream, "  -%c, --%s <int64>\n", flag->short_name, flag->name);
                                } else {
                                        fprintf(stream, "  %s <int64>\n", flag->name);
                                }
                                fprintf(stream, "    %s\n", flag->desc);
                                fprintf(stream, "    Default: %ld\n", flag->default_value.as_int64);
                                break;
                        case FLAG_FLOAT:
                                if (flag->short_name) {
                                        fprintf(stream, "  -%c, --%s <float>\n", flag->short_name, flag->name);
                                } else {
                                        fprintf(stream, "  %s <float>\n", flag->name);
                                }
                                fprintf(stream, "    %s\n", flag->desc);
                                fprintf(stream, "    Default: %f\n", flag->default_value.as_float);
                                break;
                        case FLAG_DOUBLE:
                                if (flag->short_name) {
                                        fprintf(stream, "  -%c, --%s <double>\n", flag->short_name, flag->name);
                                } else {
                                        fprintf(stream, "  %s <double>\n", flag->name);
                                }
                                fprintf(stream, "    %s\n", flag->desc);
                                fprintf(stream, "    Default: %lf\n", flag->default_value.as_double);
                                break;
                        case FLAG_STRING:
                                if (flag->short_name) {
                                        fprintf(stream, "  -%c, --%s <string>\n", flag->short_name, flag->name);
                                } else {
                                        fprintf(stream, "  %s <string>\n", flag->name);
                                }
                                fprintf(stream, "    %s\n", flag->desc);
                                fprintf(stream, "    Default: %s\n", flag->default_value.as_string);
                                break;
                        case FLAG_LIST:
                                if (flag->short_name) {
                                        fprintf(stream, "  -%c, --%s <string> ...\n", flag->short_name, flag->name);
                                } else {
                                        fprintf(stream, "  %s <string>\n", flag->name);
                                }
                                fprintf(stream, "    %s\n", flag->desc);
                                break;
                        default:
                                assert(0 && "Unreachable");
                                exit(-1);
                }
        }
}

void
flag_print_options(FILE* stream)
{
        flag_context_print_options(&flag_global_context, stream);
}

#endif
