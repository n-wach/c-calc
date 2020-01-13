#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>

#include "token.c"

int parse_to_stack(TokenStack *stack, char *exp) {
    //return 1 if valid input, 0 if not
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
            if(paren_depth < 0) return 0;
            push_operator(stack, c);
            should_neg_mean_num = false;
        } else if(c == ' ') {
            //skip
        } else {
            // characters can only be operators, numbers or spaces
            return 0;
        }
    }
    if(paren_depth > 0) {
        return 0;
    }
    reverse(stack);
    return 1;
}

int eval_postfix(TokenStack *stack) {
    //stack should be of postfix form:
    //Top = 1 -> 5 -> 8 -> + -> + = 1 5 8 + + = 1 + 5 + 8
    TokenStack temp = {NULL};
    TokenNode *v1 = NULL;
    TokenNode *v2 = NULL;
    while(stack->top != NULL) {
        TokenNode *n = pop(stack);
        if(n->type == VALUE) {
            push_node(&temp, n);
        } else {
            v1 = pop(&temp);
            if(v1 == NULL || v1->type != VALUE) return 0;
            v2 = pop(&temp);
            if(v2 == NULL || v2->type != VALUE) return 0;
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
                    if(v1->value == 0) return 0;
                    push_value(&temp, v2->value / v1->value);
                    break;
            }
            free(v1);
            free(v2);
            free(n);
        }
    }
    return temp.top->value;
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

int shunting_yard(TokenStack *postfixOut, TokenStack *algebraicInput) {
    TokenStack operatorStack = {NULL};
    while(algebraicInput->top != NULL) {
        TokenNode *token = pop(algebraicInput);
        if(token == NULL) return 0;
        if(token->type == VALUE) {
            push_node(postfixOut, token);
        } else if (token->value == L_P){
            // Left paren
            push_node(&operatorStack, token);
        } else if (token->value == R_P) {
            // Right paren
            while(true) {
                if(operatorStack.top == NULL) return 0;
                TokenNode *op = pop(&operatorStack);
                if(op->value == L_P) {
                    free(op);
                    break;
                };
                push_node(postfixOut, op);
            }
            free(token);
        } else {
            // Operator - ADD, SUB, MUL, DIV
            while(operatorStack.top != NULL) {
                TokenNode *op = operatorStack.top;
                if(op->value == L_P) break;
                int stack_p = precedence(op->value);
                int new_p = precedence(token->value);
                if(new_p >= stack_p) break;
                move_top(&operatorStack, postfixOut);
            }
            push_node(&operatorStack, token);
        }
    }
    while(operatorStack.top != NULL) {
        move_top(&operatorStack, postfixOut);
    }
    return 1;
}


int calc(char *exp) {
    // Turns algebraic string into token stack in same order
    TokenStack input_stack = {NULL};
    if(parse_to_stack(&input_stack, exp) == 0) {
        printf("Invalid input string.\n");
        return 0;
    }
    //printf("Input: ");
    //print_stack(&input_stack);

    // Use shunting yard to change order to postfix
    TokenStack postfix = {NULL};
    if(shunting_yard(&postfix, &input_stack) == 0) {
        printf("Invalid input string.\n");
        return 0;
    }
    reverse(&postfix);

    //printf("Postfix: ");
    //print_stack(&postfix);

    // Evaluate postfix
    return eval_postfix(&postfix);
}

void test() {
    assert(calc("-1") == -1);
    assert(calc("1-1") == 0);
    assert(calc("1--1") == 2);
    assert(calc("-5*-1") == 5);
    assert(calc("1-(-1)") == 2);
    assert(calc("(1)--1") == 2);
    assert(calc("1+1*5") == 6);
    assert(calc("(1+1)*5") == 10);
    assert(calc("(1+1   )   *5") == 10);
    assert(calc("((((5))))*(((1))+((2)))") == 15);
}

void intr() {
    char *line = NULL;
    size_t len = 0;
    printf("calc: ");
    getline(&line, &len, stdin);
    printf("%i\n", calc(line));
    free(line);
}

int main() {
    test();
    while(true) {
        intr();
    }
    return 0;
}

