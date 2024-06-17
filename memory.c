#include <stdio.h>
#include <stdlib.h>

int static_var = 3;

void stack_memory() {
    int stack_var = 19;
    printf("Локальная переменная: %d\n", stack_var);
}

void dynamic_memory() {
    int *dynamic_var = (int *)malloc(sizeof(int));
    if (dynamic_var == NULL) {
        printf("Ошибка выделения памяти \n");
        return;
    }
    *dynamic_var = 77;
    printf("Динамическая переменная: %d\n", *dynamic_var);

    free(dynamic_var);
}

int main() {
    printf("Статическая переменная: %d\n", static_var);

    stack_memory();
    dynamic_memory();

    return 0;
}
