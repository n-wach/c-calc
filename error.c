#ifndef ERROR_C
#define ERROR_C

typedef enum error_type {
    NONE,
    INVALID_CHAR,
    EXTRA_R_P,
    EXTRA_L_P,
    MISSING_VALUE,
    EXTRA_VALUE,
    DIVIDE_BY_ZERO,
    EMPTY_EXPRESSION,
} Error;

typedef struct result {
    Error error;
    int value;
} Result;

void print_error(Result res) {
    switch(res.error) {
        case NONE:
            break;
        case INVALID_CHAR:
            printf("Invalid character '%c'\n", res.value);
            break;
        case EXTRA_R_P:
            printf("Extra ')'\n");
            break;
        case EXTRA_L_P:
            printf("Unclosed '('\n");
            break;
        case MISSING_VALUE:
            printf("Operator '%c' does not have two arguments\n", res.value);
            break;
        case EXTRA_VALUE:
            printf("Extra value '%d' is missing operator\n", res.value);
            break;
        case DIVIDE_BY_ZERO:
            printf("Divide by zero\n");
            break;
        case EMPTY_EXPRESSION:
            printf("Empty expression\n");
            break;
    }
}

#endif