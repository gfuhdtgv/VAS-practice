#include <iostream>
#include <cstdlib>

void printDynamic(int rows, int cols, int **array) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << array[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void printStatic(int array[][10], int rows) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < 10; ++j) {
            std::cout << array[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    int rows, cols;

    std::cout << "Введите количество строк: ";
    if (scanf("%d", &rows) != 1 || rows <= 0) {
        std::cerr << "Некорректное количество строк\n";
        return EXIT_FAILURE;
    }

    std::cout << "Введите количество столбцов: ";
    if (scanf("%d", &cols) != 1 || cols <= 0) {
        std::cerr << "Некорректное количество столбцов\n";
        return EXIT_FAILURE;
    }

    int **array = new int*[rows]; //Память для i**
    for (int i = 0; i < rows; ++i) {
        array[i] = new int[cols];
    }

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            array[i][j] = i * cols + j;
        }
    }

    printDynamic(rows, cols, array);

    for (int i = 0; i < rows; ++i) { //Освобождение памяти
        delete[] array[i];
    }
    delete[] array;

    const int rows1 = 5; // Можно задать динамически i*[]
    int array2[rows1][10]; // i[][10] 

    for (int i = 0; i < rows1; ++i) {
        for (int j = 0; j < 10; ++j) {
            array2[i][j] = i * 10 + j;
        }
    }

    printStatic(array2, rows1);

    return 0;
}
