#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>

#include "token.c"
#include "error.c"


Result parse_to_stack(TokenStack *stack, char *exp) {
    //return 1 if valid input, 0 if not
    Result result = {NONE};
    bool should_neg_mean_num = true;
    int paren_depth = 0;
    for(int i = 0; ; i++) {
        char c = exp[i];
        if(c == '\0' || c == '\n') {
            break;
        } else if(isdigit(c) || (c == '-' && isdigit(exp[i+1]) && should_neg_mean_num)) {
            int value = atoi(exp + i);
            push_value(stack, value);
            while(isdigit(exp[i + 1])) {
                i++;
            }
            should_neg_mean_num = false;
        } else if(c == ADD || c == SUB || c == MUL || c == DIV) {
            push_operator(stack, c);
            should_neg_mean_num = true;
        } else if(c == L_P) {
            paren_depth++;
            push_operator(stack, c);
            should_neg_mean_num = true;
        } else if(c == R_P) {
            paren_depth--;
            if(paren_depth < 0) {
                result.error = EXTRA_R_P;
                return result;
            }
            push_operator(stack, c);
            should_neg_mean_num = false;
        } else if(c == ' ') {
            //skip
        } else {
            // characters can only be operators, numbers or spaces
            result.error = INVALID_CHAR;
            result.value = c;
            return result;
        }
    }
    if(paren_depth > 0) {
        result.error = EXTRA_L_P;
        return result;
    }
    reverse(stack);
    return result;
}

Result eval_postfix(TokenStack *stack) {
    Result result = {NONE};

    TokenStack temp = {NULL};
    TokenNode *v1 = NULL;
    TokenNode *v2 = NULL;
    while(stack->top != NULL) {
        TokenNode *n = pop(stack);
        if(n->type == VALUE) {
            push(&temp, n);
        } else {
            v1 = pop(&temp);
            if(v1 == NULL || v1->type != VALUE) {
                result.error = MISSING_VALUE;
                result.value = n->value;
                return result;
            }
            v2 = pop(&temp);
            if(v2 == NULL || v2->type != VALUE) {
                result.error = MISSING_VALUE;
                result.value = n->value;
                return result;
            }
            switch(n->value) {
                case ADD:
                    push_value(&temp, v2->value + v1->value);
                    break;
                case SUB:
                    push_value(&temp, v2->value - v1->value);
                    break;
                case MUL:
                    push_value(&temp, v2->value * v1->value);
                    break;
                case DIV:
                    if(v1->value == 0) {
                        result.error = DIVIDE_BY_ZERO;
                        return result;
                    }
                    push_value(&temp, v2->value / v1->value);
                    break;
            }
            free(v1);
            free(v2);
            free(n);
        }
    }
    if(temp.top == NULL) {
        result.error = EMPTY_EXPRESSION;
        return result;
    } else {
        TokenNode *r = pop(&temp);
        result.value = r->value;
        free(r);
    }
    if(temp.top != NULL) {
        TokenNode *n;
        result.error = EXTRA_VALUE;
        while(temp.top != NULL) {
            n = pop(&temp);
            result.value = n->value;
            free(n);
        }
        return result;
    }
    return result;
}

void print_stack(TokenStack *stack) {
    TokenNode *head = stack->top;
    while(head != NULL) {
        if(head->type == VALUE) {
            printf("%i", head->value);
        } else {
            printf("%c", head->value);
        }
        head = head->next;
        printf(" ");
    }
    printf("\n");
}

int precedence(int op) {
    switch(op) {
        case ADD: return 2;
        case SUB: return 2;
        case MUL: return 3;
        case DIV: return 3;
        default: return 0;
    }
}

Result shunting_yard(TokenStack *postfixOut, TokenStack *algebraicInput) {
    Result result = {NONE};
    TokenStack operatorStack = {NULL};
    while(algebraicInput->top != NULL) {
        TokenNode *token = pop(algebraicInput);
        if(token->type == VALUE) {
            push(postfixOut, token);
        } else if (token->value == L_P){
            // Left paren
            push(&operatorStack, token);
        } else if (token->value == R_P) {
            // Right paren
            while(true) {
                if(operatorStack.top == NULL) {
                    result.error = EXTRA_R_P;
                    return result;
                }
                TokenNode *op = pop(&operatorStack);
                if(op->value == L_P) {
                    free(op);
                    break;
                };
                push(postfixOut, op);
            }
            free(token);
        } else {
            // Operator - ADD, SUB, MUL, DIV
            while(operatorStack.top != NULL) {
                TokenNode *op = operatorStack.top;
                if(op->value == L_P) break;
                int stack_p = precedence(op->value);
                int new_p = precedence(token->value);
                if(new_p > stack_p) break;
                move_top(&operatorStack, postfixOut);
            }
            push(&operatorStack, token);
        }
    }
    while(operatorStack.top != NULL) {
        move_top(&operatorStack, postfixOut);
    }
    return result;
}


Result calc(char *exp) {
    // Turns algebraic string into token stack in same order
    TokenStack input_stack = {NULL};
    Result result = {NONE};

    result = parse_to_stack(&input_stack, exp);
    if(result.error != NONE){
        return result;
    }
    //printf("Input: ");
    //print_stack(&input_stack);

    // Use shunting yard to change order to postfix
    TokenStack postfix = {NULL};
    result = shunting_yard(&postfix, &input_stack);
    if(result.error != NONE){
        return result;
    }
    reverse(&postfix);

    //printf("Postfix: ");
    //print_stack(&postfix);

    // Evaluate postfix
    result = eval_postfix(&postfix);
    if(result.error != NONE){
        return result;
    }
    return result;
}

void test() {
    assert(calc("-1").value == -1);
    assert(calc("1-1").value == 0);
    assert(calc("1--1").value == 2);
    assert(calc("-5*-1").value == 5);
    assert(calc("1-(-1)").value == 2);
    assert(calc("(1)--1").value == 2);
    assert(calc("1+1*5").value == 6);
    assert(calc("(1+1)*5").value == 10);
    assert(calc("(1+1   )   *5").value == 10);
    assert(calc("((((5))))*(((1))+((2)))").value == 15);
}

void intr() {
    char *line = NULL;
    size_t len = 0;
    printf("calc: ");
    getline(&line, &len, stdin);
    Result r = calc(line);
    if(r.error == NONE) {
        printf("=%i\n", r.value);
    } else {
        print_error(r);
    }
    free(line);
}

int main() {
    //test();
    while(true) {
        intr();
    }
    return 0;
}

