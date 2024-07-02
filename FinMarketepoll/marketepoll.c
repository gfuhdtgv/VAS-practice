#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_EVENTS 100
#define MAX_MESSAGE_LENGTH 128

typedef struct {
    int id;
    float capital;
    int pipe_fd[2];
    int state; // 0 - банкрот, 1 - покупает, 2 - продает
    pid_t pid;
} User;

void UserProcess(int id, float capital, int pipe_fd[]) {// Поведение игроков
    srand(time(NULL) ^ getpid()); // более случайный рандом, чтобы у каждого процесса был свой
    while (capital > 0) {
        char message[MAX_MESSAGE_LENGTH];
        float transaction_capital = ((float) rand() / RAND_MAX) * (0.5 * capital - 0.25 * capital) + 0.25 * capital;//сумма транзакции

        if (rand() % 2 == 0) { //выбор состояния -покупка -продажа
            snprintf(message, sizeof(message), "Игрок %d: покупает %.2f", id, transaction_capital);
        } else {
            snprintf(message, sizeof(message), "Игрок %d: продает %.2f", id, transaction_capital);
        }
        write(pipe_fd[1], message, sizeof(message));//отправка в род.процесс

        char result_message[MAX_MESSAGE_LENGTH];
        read(pipe_fd[0], result_message, sizeof(result_message));
        float result;
        sscanf(result_message, "Результат транзакции: %f", &result);
        capital += result;
        if (capital <= 0) {
            snprintf(message, sizeof(message), "Игрок %d: Банкрот", id);
            write(pipe_fd[1], message, sizeof(message));
            break;
        }
    }
    close(pipe_fd[1]);
    exit(0);
}

void market(int num_user, User users[]) { //фун-ия поведения рынка
    int epfd = epoll_create1(0);//создает экземпляр epoll 
    if (epfd == -1) {
        perror("Ошибка создания epoll");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event; //структура для добавления дескрипт. в epoll
    struct epoll_event *events = calloc(MAX_EVENTS, sizeof(event)); //Для хранения событий (epoll_wait)

    for (int i = 0; i < num_user; ++i) { //добавл. файловых дескрипт.
        event.events = EPOLLIN;//события чтения
        event.data.fd = users[i].pipe_fd[0];
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, users[i].pipe_fd[0], &event) == -1) {
            perror("Ошибка добавления файлового дескриптора в epoll");
            exit(EXIT_FAILURE);
        }
	int flags = fcntl(users[i].pipe_fd[0],F_GETFL,0); //файловый дескрипт. в неблокирующий режим
    	fcntl(users[i].pipe_fd[0],F_SETFL,flags | O_NONBLOCK);
    }

    int transaction_count = 0;

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("Ошибка ожидания событий ввода/вывода epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == -1)
                continue;

            char message[MAX_MESSAGE_LENGTH];
            read(events[n].data.fd, message, sizeof(message));

            int user_id;
            for (int i = 0; i < num_user; i++) {
                if (users[i].pipe_fd[0] == events[n].data.fd) {
                    user_id = i;
                    break;
                }
            }

            if (strstr(message, "покупает") != NULL) {
                users[user_id].state = 1;
            } else if (strstr(message, "продает") != NULL) {
                users[user_id].state = 2;
            }

            for (int j = 0; j < num_user; j++) {
                if (j != user_id && users[j].capital > 0 && ((users[user_id].state == 1 && users[j].state == 2) || (users[user_id].state == 2 && users[j].state == 1))) {
		    if (users[user_id].capital <= 0 || users[j].capital <= 0) {
                        continue; // Пропускаем игроков с нулевым капиталом
                    }
                    float transaction_capital_i, transaction_capital_j;//извлечение капитала сделки
                    sscanf(message, "Игрок %*d: %*s %f", &transaction_capital_i);
                    char message_j[MAX_MESSAGE_LENGTH];
                    read(users[j].pipe_fd[0], message_j, sizeof(message_j));
                    sscanf(message_j, "Игрок %*d: %*s %f", &transaction_capital_j);

                    printf("\n Игрок %d и Игрок %d\n", users[user_id].id, users[j].id);
                    printf("Бюджет %d до: %.2f\n", users[user_id].id, users[user_id].capital);
                    printf("Бюджет %d до: %.2f\n", users[j].id, users[j].capital);
                    printf("Сумма транзакции %d: %.2f\n", users[user_id].id, transaction_capital_i);
		    printf("Сумма транзакции %d: %.2f\n", users[j].id, transaction_capital_j);

                    float result = transaction_capital_i - transaction_capital_j;
                    char result_message[MAX_MESSAGE_LENGTH];
                    snprintf(result_message, sizeof(result_message), "Результат транзакции: %.2f", result);

                    write(users[user_id].pipe_fd[1], result_message, sizeof(result_message));//передача рез-та транзакции
                    write(users[j].pipe_fd[1], result_message, sizeof(result_message));

                    users[user_id].capital += result;
                    users[j].capital -= result;

                    printf("Бюджет %d после: %.2f\n", users[user_id].id, users[user_id].capital);
                    printf("Бюджет %d после: %.2f\n", users[j].id, users[j].capital);

                    transaction_count++;

                    // Сброс состояния после сделки
                    users[user_id].state = 0;
                    users[j].state = 0;

                    // Проверка на банкротство
                    if (users[user_id].capital <= 0) {
                        snprintf(result_message, sizeof(result_message), "Игрок %d: Банкрот", users[user_id].id);
                        write(users[user_id].pipe_fd[1], result_message, sizeof(result_message));
                        close(users[user_id].pipe_fd[0]);
			epoll_ctl(epfd, EPOLL_CTL_DEL, users[user_id].pipe_fd[0], NULL);
                        events[n].data.fd = -1; // Удаляем из epoll
                    }
                    if (users[j].capital <= 0) {
                        snprintf(result_message, sizeof(result_message), "Игрок %d: Банкрот", users[j].id);
                        write(users[j].pipe_fd[1], result_message, sizeof(result_message));
                        close(users[j].pipe_fd[0]);
			epoll_ctl(epfd, EPOLL_CTL_DEL, users[j].pipe_fd[0], NULL); // Удаляем из epoll
                        for (int k = 0; k < nfds; k++) {
                            if (events[k].data.fd == users[j].pipe_fd[0]) {
                                events[k].data.fd = -1; // Удаляем из epoll
                                break;
                            }
                        }
			users[j].state=0;
			break;
                    }
                }
            }

            if (strstr(message, "Банкрот") != NULL) {
                printf("%s\n", message);
            }
        }

        int active_users_count = 0;
        for (int i = 0; i < num_user; i++) {
            if (users[i].capital > 0) {
                active_users_count++;
            }
        }
        if (active_users_count <= 1) {
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
        if (all_buying || all_selling) {
            break;
        }
    }

    printf("\nВсе игроки либо покупают, либо продают, либо обанкротились. Завершение работы.\n");

    for (int i = 0; i < num_user; i++) {
    const char *state_str;
    if (users[i].capital <= 0) {
        state_str = "Банкрот";
    } else {
        switch (users[i].state) {
            case 1:
                state_str = "покупает";
                break;
            case 2:
                state_str = "продает";
                break;
        }
    }
        printf("Игрок %d: финальный капитал %.2f, состояние: %s\n", users[i].id, users[i].capital, state_str);
    }

    printf("\nОбщее число транзакций: %d\n", transaction_count);

    close(epfd);
    free(events);
}

void createUserProcesses(int num_user, User users[]) {
    for (int i = 0; i < num_user; i++) { //состояние id капитал
        users[i].id = i + 1;
        users[i].capital = ((float) rand() / RAND_MAX) * 1000.0 + 100.0;
        users[i].state = 0;
        if (pipe(users[i].pipe_fd) == -1) {
            perror("Ошибка создания pipe");
            exit(EXIT_FAILURE);
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("Ошибка создания процесса")
;
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(users[i].pipe_fd[0]);
            UserProcess(users[i].id, users[i].capital, users[i].pipe_fd);
            exit(0);
        } else {
            close(users[i].pipe_fd[1]);
            users[i].pid = pid;
            printf("Игрок %d: начальный капитал %.2f\n", users[i].id, users[i].capital);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Ошибка: %s введите количество игроков\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int num_user = atoi(argv[1]);
    if (num_user <= 0) {
        fprintf(stderr, "Некорректный ввод количества игроков\n");
        exit(EXIT_FAILURE);
    }
    User *users = malloc(num_user * sizeof(User));
    if (users == NULL) {
        perror("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));
    createUserProcesses(num_user, users);
    market(num_user, users);
    free(users);
    return 0;
}
