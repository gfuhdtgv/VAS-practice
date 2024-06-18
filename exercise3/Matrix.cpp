#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define ROWS 3
#define COLS 3

void print_matrix(int matrix[ROWS][COLS]) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main() {
    int static_matrix[ROWS][COLS] = { //Статик.
        {1, 2, 3}, 
        {4, 5, 6}, 
        {7, 8, 9} 
    };

    int local_matrix[ROWS][COLS] = { //Локал.(Стек)
        {9, 6, 5},
        {8, 7, 4},
        {3, 2, 1}
    };

    int **dynamic_matrix = (int **)malloc(ROWS * sizeof(int *));//Динам. память
    for (int i = 0; i < ROWS; ++i) {
        dynamic_matrix[i] = (int *)malloc(COLS * sizeof(int));
    }

    for (int i = 0; i < ROWS; ++i) { //Динам.
        for (int j = 0; j < COLS; ++j) {
            dynamic_matrix[i][j] = i * COLS + j;
        }
    }

    printf("Статическая память:\n");
    print_matrix(static_matrix);

    printf("Локальная память:\n");
    print_matrix(local_matrix);

    printf("Динамическая матрица:\n");
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            printf("%d ", dynamic_matrix[i][j]);
        }
        printf("\n");
    }

    // очистка
    for (int i = 0; i < ROWS; ++i) {
        free(dynamic_matrix[i]);
    }
    free(dynamic_matrix);

    return 0;
}

