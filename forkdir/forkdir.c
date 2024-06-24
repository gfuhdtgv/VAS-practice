#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_DEPTH 20 //макс. глубина

int get_max_processes() { //ф-я для получения максимального количества процессов
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN); //получаем инф-ю о максимально доступном кол-ве процессов

    if (nprocs < 1) { //ошибка чтения процессов
        perror("sysconf");
        exit(EXIT_FAILURE);
    }
    return (int)nprocs;
}

void traverse_directory(const char *dir_path, int depth, int max_processes) { // ф-я обхода дир.
    static int current_processes = 0; // Переменная для отслеживания текущего количества процессов

    if (depth > MAX_DEPTH) { //Проверка на зацикливание
        fprintf(stderr, "Глубина рекурсии превышена, возможен бесконечный цикл!\n");
        return;
    }

    if (access(dir_path, R_OK) != 0) { //Проверка прав доступа на чтение
        perror("Ошибка прав доступа");
        return;
    }

    DIR *dir = opendir(dir_path); //Откр. директории
    if (dir == NULL) {
        perror("Ошибка открытия директории");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) { //считываются эл-ты директории
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { //текущая и род. дир. не обрабат.
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name); //Полный путь к файлу

        struct stat path_stat; //структура для хранения инф-ии о файле или дир.
        if (lstat(full_path, &path_stat) != 0) { //если возвр. ненулевое- ошибка
            perror("");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) { // Если директория
            printf("Директория: %s (PID: %d)\n", full_path, getpid());
            
            while (current_processes >= max_processes) { //Ограничение кол-ва процессов
                wait(NULL); //ждет завершения одного из доч.проц.
                current_processes--;
            }

            pid_t pid = fork(); //создание нового процесса

            if (pid < 0) {
                perror("Ошибка создания процесса");
                continue;
            } else if (pid == 0) { // Находится в дочернем процессе
                traverse_directory(full_path, depth + 1, max_processes); //рекурсивно вызывает обход
                exit(EXIT_SUCCESS);
            } else {
                current_processes++;
            }
        } else if (S_ISREG(path_stat.st_mode)) { //Если файл
            printf("Файл: %s\n", full_path);
        } else if (S_ISLNK(path_stat.st_mode)) { //Если символическая ссылка
            char link_target[PATH_MAX];
            ssize_t len = readlink(full_path, link_target, sizeof(link_target) - 1);

            if (len != -1) {
                link_target[len] = '\0';
                struct stat target_stat;
                if (stat(link_target, &target_stat) != 0) {
                    perror("Ошибка получения информации о файле");
                    continue;
                }
                if (S_ISREG(target_stat.st_mode)) {
                    printf("Символическая ссылка на файл: %s -> %s\n", full_path, link_target);
                }
            }
        }
    }
    closedir(dir); // Закрытие директории
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Неправильно указана начальная директория: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int max_processes = get_max_processes();
    traverse_directory(argv[1], 0, max_processes);

    return 0;
}
