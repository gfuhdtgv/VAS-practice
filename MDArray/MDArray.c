#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef void* (*initialization)(int*, int); // Определение типа функции инициализации

void** createArray(int dim, int* sizes, initialization init, int* index) { // Многомерный массив (рекурсивно)
    if (dim == 0) { // если размерность равна нулю
        return NULL;
    }
    int size = sizes[0]; // получаем значение текущего измерения массива
    void** array = (void**)malloc(size * sizeof(void*)); // Память для массива указателей

    if (dim > 1) { // Создание подмассивов
        for (int i = 0; i < size; i++) {
            index[0] = i;
            array[i] = createArray(dim - 1, sizes + 1, init, index + 1); // Рекурсия
        }
    } else { // инициализир. каждый элемент
        for (int i = 0; i < size; i++) {
            index[0] = i;
            array[i] = init(index, dim);
        }
    }
    return array;
}

void createMDArray(void **res, int n, initialization init, int* dimensions) { // считывает размерность и измерения, вызывает создание
    int* index = (int*)malloc(n * sizeof(int)); // Память для последующей инициализации

    *res = createArray(n, dimensions, init, index); // создаем массив

    free(index);
}

void freeArray(void* arr, int dim, int* sizes) { // Освобождение памяти многомерного массива (рекурсия)
    if (dim == 0) {
        return;
    }
    void** array = (void**)arr;
    int size = sizes[0]; // значение текущего измерения

    if (dim > 1) {
        for (int i = 0; i < size; i++) {
            freeArray(array[i], dim - 1, sizes + 1);
        }
    } else {
        for (int i = 0; i < size; i++) {
            free(array[i]);
        }
    }
    free(array);
}

void freeMDArray(void* array, int n, int* dimensions) {
    freeArray(array, n, dimensions);
}

void* initFunction(int* index, int dim) { // инициализация
    int* value = (int*)malloc(sizeof(int));
    *value = 0;

    for (int i = 0; i < dim; ++i) {
        *value = *value * 10 + index[i];
    }
    return value;
}

void printArray(void** array, int dim, int* sizes, int level) { // вывод массива
    if (dim == 1) {
        for (int i = 0; i < sizes[0]; i++) {
            printf("%d ", *((int*)array[i]));
        }
        printf("\n");
    } else {
        for (int i = 0; i < sizes[0]; i++) {
            printArray((void**)array[i], dim - 1, sizes + 1, level + 1); // Рекурсия
        }
        if (level == 0) {
            printf("\n");
        }
    }
}

int main() {
    void* array = NULL;

    int n;
    printf("Введите размерность массива: ");
    scanf("%d", &n);

    int* dimensions = (int*)malloc(n * sizeof(int));
    printf("Введите размеры для каждого измерения:\n");
    for (int i = 0; i < n; ++i) {
        printf("Размер для измерения %d: ", i + 1);
        scanf("%d", &dimensions[i]);
    }

   
    createMDArray(&array, n, initFunction, dimensions);//Вызов ф-ии создания массива

    printArray((void**)array, n, dimensions, 0); //Вывод

    freeMDArray(array, n, dimensions);
    free(dimensions);

    return 0;
}

