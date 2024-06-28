#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define MAX_MESSAGE_LENGTH 128

typedef struct {
    int id;
    float capital;
    int pipe_fd[2];
    int state; // 0 - банкрот, 1 - покупает, 2 - продает
    pid_t pid;
} User;

void UserProcess(int id, float capital, int pipe_fd[]) { //поведение игроков
    srand(time(NULL) ^ getpid()); //Более случайный рандом, чтобы у каждого процесса был свой

    while (capital > 0) {

        char message[MAX_MESSAGE_LENGTH];
        float transaction_capital = ((float) rand() / RAND_MAX) * (0.5 * capital - 0.25 * capital) + 0.25 * capital; // Сумма транзакции

        if (rand() % 2 == 0) { //выбор состояния - покупка -продажа
            snprintf(message, sizeof(message), "Игрок %d: покупает %.2f", id, transaction_capital);
        } else {
            snprintf(message, sizeof(message), "Игрок %d: продает %.2f", id, transaction_capital);
        }

        write(pipe_fd[1], message, sizeof(message)); // Отправка состояния в род.процесс

        // Ожидание результата транзакции от рынка
        char result_message[MAX_MESSAGE_LENGTH];
        read(pipe_fd[0], result_message, sizeof(result_message));
        float result;
        sscanf(result_message, "Результат транзакции: %f", &result);
        capital += result;

        if (capital <= 0) { // Проверка на банкротство
            snprintf(message, sizeof(message), "Игрок %d: Банкрот", id);
            write(pipe_fd[1], message, sizeof(message)); // Передача сообщения родительскому процессу о банкротстве
            break;
        }
    }
    close(pipe_fd[1]);
    exit(0); // Завершение процесса пользователя
}

void market(int num_user, User users[]) { //функция поведения рынка
    fd_set readfds; // Набор для слежения за появлением данных
    struct timeval tv; // Структура для установки таймаута
    int max_fd = 0;

    while (1) {
        FD_ZERO(&readfds); // Очистка набора дескрипторов
        for (int i = 0; i < num_user; i++) {
            if (users[i].capital > 0) { // Добавляем только активных пользователей
                FD_SET(users[i].pipe_fd[0], &readfds); // Добавление дескрипторов игроков для чтения
                if (users[i].pipe_fd[0] > max_fd) {
                    max_fd = users[i].pipe_fd[0]; // Обновление максимального дескриптора
                }
            }
        }
        tv.tv_sec = 1; // Таймаут 1 секунда
        tv.tv_usec = 0;

        int wait_dec = select(max_fd + 1, &readfds, NULL, NULL, &tv);
        if (wait_dec == -1) {
            printf("Ошибка чтения данных игроков: %d\n", errno);
            exit(EXIT_FAILURE);
        } else if (wait_dec) {
            for (int i = 0; i < num_user; i++) {
                if (users[i].capital > 0 && FD_ISSET(users[i].pipe_fd[0], &readfds)) {
                    char message[MAX_MESSAGE_LENGTH];
                    read(users[i].pipe_fd[0], message, sizeof(message));

                    if (strstr(message, "покупает") != NULL) {
                        users[i].state = 1; // Покупает
                    } else if (strstr(message, "продает") != NULL) {
                        users[i].state = 2; // Продает
                    }

                    for (int j = 0; j < num_user; j++) { // Поиск партнера для сделки
                        if (j != i && users[j].capital > 0 && ((users[i].state == 1 && users[j].state == 2) || (users[i].state == 2 && users[j].state == 1))) {
                            float transaction_capital_i, transaction_capital_j;
                            sscanf(message, "Игрок %*d: %*s %f", &transaction_capital_i); // Извлечение капитала сделки

                            char message_j[MAX_MESSAGE_LENGTH];
                            read(users[j].pipe_fd[0], message_j, sizeof(message_j));
                            sscanf(message_j, "Игрок %*d: %*s %f", &transaction_capital_j);

                            float result = transaction_capital_i - transaction_capital_j; //Транзакция
                            char result_message[MAX_MESSAGE_LENGTH];
                            snprintf(result_message, sizeof(result_message), "Результат транзакции: %.2f", result);
                            write(users[i].pipe_fd[1], result_message, sizeof(result_message));//Передача игрокам результата транзакции
                            write(users[j].pipe_fd[1], result_message, sizeof(result_message));

                            printf("\n Игрок %d и Игрок %d \n", users[i].id, users[j].id);
                            printf("Бюджет%d: %.2f \n", users[i].id, users[i].capital);
                            printf("Бюджет%d: %.2f \n", users[j].id, users[j].capital);
                            printf("Сумма транзакции %d: %.2f \n", users[i].id, transaction_capital_i);
                            printf("Сумма транзакции %d: %.2f \n", users[j].id, transaction_capital_j);
                            printf("Результат транзакции: %.2f \n", result);

                            users[i].capital += result; // Обновление капитала игрока i
                            users[j].capital -= result; // Обновление капитала игрока j

                            printf("Бюджет%d: %.2f \n", users[i].id, users[i].capital);
                            printf("Бюджет%d: %.2f \n", users[j].id, users[j].capital);

                            users[i].state = 0; // Сброс состояния игрока
                            users[j].state = 0; // Сброс состояния игрока

                        }
                    }

                    if (strstr(message, "Банкрот") != NULL) {
                        printf("%s \n", message);
                    }
                }
            }
        }

        int active_users_count = 0;
        for (int i = 0; i < num_user; i++) { //Подсчет активных пользователей
            if (users[i].capital > 0) {
                active_users_count++;
            }
        }

        if (active_users_count <= 1) { // Если остался один или меньше активных пользователей
            break;
        }

        int all_buying = 1;
        int all_selling = 1;

        for (int i = 0; i < num_user; i++) { 
            if (users[i].capital > 0) {
                if (users[i].state != 1) {
                    all_buying = 0;
                }
                if (users[i].state != 2) {
                    all_selling = 0;
                }
            }
        }

        if (all_buying || all_selling) { // Если все активные пользователи либо покупают, либо продают
            break;
        }
    }

    printf("\n Все игроки либо покупают, либо продают, либо обанкротились. Завершение работы. \n");

    for (int i = 0; i < num_user; i++) {//Проверка итогового состояния пользователей для вывода
        const char *state_str;
        switch (users[i].state) {
            case 1:
                state_str = "покупает";
                break;
            case 2:
                state_str = "продает";
                break;
            default:
                state_str = "Банкрот";
                break;
        }
        printf("Игрок %d: финальный капитал %.2f, состояние: %s \n", users[i].id, users[i].capital, state_str);
    }
}

void createUserProcesses(int num_user, User users[]) { //Создание игроков
    for (int i = 0; i < num_user; i++) { // id, начальный капитал, первичное состояние
        users[i].id = i + 1;
        users[i].capital = ((float) rand() / RAND_MAX) * 1000.0 + 100.0;
        users[i].state = 0;

        if (pipe(users[i].pipe_fd) == -1) { //ошибка создания канала
            perror("Ошибка создания pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Ошибка создания процесса");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(users[i].pipe_fd[0]);
            UserProcess(users[i].id, users[i].capital, users[i].pipe_fd);
            exit(0);
        } else {
            close(users[i].pipe_fd[1]);
            users[i].pid = pid;
            printf("Игрок %d: начальный капитал %.2f \n", users[i].id, users[i].capital);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
fprintf(stderr, "Ошибка: %s введите количество игроков \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_user = atoi(argv[1]); //преобразование 2го аргумента в целое число

    if (num_user <= 0) {
        fprintf(stderr, "Некоректный ввод количества игроков \n");
        exit(EXIT_FAILURE);
    }

    User *users = malloc(num_user * sizeof(User));
    if (users == NULL) {
        perror("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL)); //генер.случ.чисел
    createUserProcesses(num_user, users); //запуск ф-ии создания игрока
    market(num_user, users); //запуск фин.рынка

    free(users);
    return 0;
}
