#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_DEPTH 20 // Максимальная глубина рекурсии

//получение максимального количества процессов
int get_max_processes() {
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1) {
	printf("Error code: %d\n", errno);
        perror("Ошибка чтения процессов");
        exit(EXIT_FAILURE);
    }
    return (int)nprocs;
}

static int total_processes = 0; // для отслеживания общего количества дочерних процессов

//обход директории
void traverse_directory(const char *dir_path, int depth, int max_processes) {
    static int current_processes = 0; // отслеживание текущего количества процессов

    if (depth > MAX_DEPTH) { //Глубина рекурсии
        fprintf(stderr, "Глубина рекурсии превышена, возможен бесконечный цикл!\n");
        return;
    }

    if (access(dir_path, R_OK) != 0) { //проверка на права доступа
	printf("Error code: %d\n", errno);
        perror("Ошибка прав доступа");
        return;
    }

    DIR *dir = opendir(dir_path); //Открытие директории
    if (dir == NULL) {
	printf("Error code: %d\n", errno);
        perror("Ошибка открытия директории");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat path_stat;
        if (lstat(full_path, &path_stat) != 0) {
	    printf("Error code: %d\n", errno);
            perror("Ошибка получения информации о пути");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) { // Если директория
            printf("Директория: %s (PID: %d)\n", full_path, getpid());

            while (current_processes >= max_processes) {
                current_processes--;
            }

            pid_t pid = fork();
            if (pid < 0) {
		printf("Error code: %d\n", errno);
                perror("Ошибка создания процесса");
                continue;
            } else if (pid == 0) { // Дочерний процесс
                traverse_directory(full_path, depth + 1, max_processes); //Рекурсия
                exit(EXIT_SUCCESS);
            } else {
                current_processes++;
                total_processes++;
            }
        } else if (S_ISREG(path_stat.st_mode)) { // Если файл
            printf("Файл: %s\n", full_path);
        } else if (S_ISLNK(path_stat.st_mode)) { // Если символическая ссылка
            char link_target[PATH_MAX];
            ssize_t len = readlink(full_path, link_target, sizeof(link_target) - 1);
            if (len != -1) {
                link_target[len] = '\0';
                struct stat target_stat;
                if (stat(link_target, &target_stat) != 0) {
		    printf("Error code: %d\n", errno);
                    perror("Ошибка получения информации о файле");
                    continue;
                }
                if (S_ISREG(target_stat.st_mode)) {
                    printf("Символическая ссылка на файл: %s -> %s\n", full_path, link_target);
                }
            }
        }
    }

    closedir(dir); // Закрываем директорию после обхода
    while (wait(NULL) > 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Ошибка ввода директории: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int max_processes = get_max_processes(); //Получение макс. количества потоков для создания
    traverse_directory(argv[1], 0, max_processes); //Обход директории

    // Ожидаем завершения всех дочерних процессов
    while (wait(NULL) > 0);

    printf("Общее количество использованных дочерних процессов: %d\n", total_processes);

    return 0;
}

