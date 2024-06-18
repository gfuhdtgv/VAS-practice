#include <stdio.h>
#include <stdlib.h>

#define ROWS 3
#define COLS 3

void processing_fixed(int array[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%d ", array[i][j]);
        }
        printf("\n");
    }
}

void processing_arbitrary_local(int rows, int cols, int array[rows][cols]) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", array[i][j]);
        }
        printf("\n");
    }
}

void processing_arbitrary_dynamic(int rows, int cols, int **array) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", array[i][j]);
        }
        printf("\n");
    }
}

int main() {
    int array[ROWS][COLS] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    processing_fixed(array);

    int rows;
    int cols;

    printf("Введите количество строк: ");
    if (scanf("%d", &rows) != 1 || rows <= 0) {
        fprintf(stderr, "Неверный ввод для количества строк\n");
        return EXIT_FAILURE;
    }

    printf("Введите количество столбцов: ");
    if (scanf("%d", &cols) != 1 || cols <= 0) {
        fprintf(stderr, "Неверный ввод для количества столбцов\n");
        return EXIT_FAILURE;
    }

    int array1[rows][cols];
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            array1[i][j] = i * cols + j + 1;
        }
    }
    processing_arbitrary_local(rows, cols, array1);

    int **array2 = malloc(rows * sizeof(int *)); // Память для строк
    if (array2 == NULL) {
        perror("Ошибка выделения памяти");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < rows; i++) { // Память для столбцов
        array2[i] = malloc(cols * sizeof(int));
        if (array2[i] == NULL) {
            perror("Ошибка выделения памяти");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            array2[i][j] = i * cols + j + 1;
        }
    }
    processing_arbitrary_dynamic(rows, cols, array2); // освобождение памяти

    for (int i = 0; i < rows; i++) {
        free(array2[i]);
    }
    free(array2);

    return 0;
}

