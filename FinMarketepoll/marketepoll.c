#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_MESSAGE_LENGTH 256
#define MAX_EVENTS 10000

typedef struct {
    int id;
    float capital;
    int pipe_fd[2];
    int state; // 0 - банкрот, 1 - покупает, 2 - продает
    pid_t pid;
} User;

void UserProcess(int id, float capital, int pipe_fd[]) {
    srand(time(NULL) ^ getpid());
    close(pipe_fd[0]); // Закрываем сторону чтения пайпа в дочернем процессе

    while (capital > 0) {
        char message[MAX_MESSAGE_LENGTH];
        float transaction_capital = ((float) rand() / RAND_MAX) * (0.5 * capital - 0.25 * capital) + 0.25 * capital;
        if (rand() % 2 == 0) {
            snprintf(message, sizeof(message), "Игрок %d: покупает %.2f", id, transaction_capital);
        } else {
            snprintf(message, sizeof(message), "Игрок %d: продает %.2f", id, transaction_capital);
        }
        write(pipe_fd[1], message, sizeof(message));

        char result_message[MAX_MESSAGE_LENGTH];
        read(pipe_fd[1], result_message, sizeof(result_message));
        float result;
        sscanf(result_message, "Результат транзакции: %f", &result);
        capital += result;

        // Проверка состояния капитала
        if (capital <= 0) {
            snprintf(message, sizeof(message), "Игрок %d: Банкрот", id);
            write(pipe_fd[1], message, sizeof(message));
            break;
        }
    }

    close(pipe_fd[1]);
    exit(0);
}

void market(int num_user, User users[]) { //ф-я поведения рынка
    int epoll_fd = epoll_create1(0);//создает экземпляр epoll и созд.дескрипт.
    if (epoll_fd == -1) {
        perror("Ошибка создания epoll");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event; //структура для хранения дескриптю в epoll
    struct epoll_event events[MAX_EVENTS]; //для хранения событий (epoll_wait)
    int transaction_count = 0;

    for (int i = 0; i < num_user; i++) { //Добавление файл.дескрипт
        event.events = EPOLLIN;// события чтения
        event.data.fd = users[i].pipe_fd[0];
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, users[i].pipe_fd[0], &event) == -1) {
            perror("Ошибка добавления файлового дескриптора в epoll");
            exit(EXIT_FAILURE);
        }
	int flags = fcntl(users[i].pipe_fd[0], F_GETFL,0);
    	fcntl(users[i].pipe_fd[0],F_SETFL,flags | O_NONBLOCK);
    }

    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n == -1) {
            perror("Ошибка ожидания событий ввода/вывода epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < num_user; j++) {
                if (events[i].data.fd == users[j].pipe_fd[0]) {

		    if (users[j].capital <= 0) {
                        continue; // Пропускаем обанкротившихся игроков
                    }
                    char message[MAX_MESSAGE_LENGTH];
                    read(users[j].pipe_fd[0], message, sizeof(message));
                   
                    if (strstr(message, "покупает") != NULL) {
                        users[j].state = 1;
                    } else if (strstr(message, "продает") != NULL) {
                        users[j].state = 2;
                    }


                    for (int k = 0; k < num_user; k++) {
                        if (k != j && users[k].capital > 0 && ((users[j].state == 1 && users[k].state == 2) || (users[j].state == 2 && users[k].state == 1))) {
                            float transaction_capital_j, transaction_capital_k;
                            sscanf(message, "Игрок %*d: %*s %f", &transaction_capital_j); //Извлечение транзакции
                            char message_k[MAX_MESSAGE_LENGTH];
                            read(users[k].pipe_fd[0], message_k, sizeof(message_k));
                            sscanf(message_k, "Игрок %*d: %*s %f", &transaction_capital_k);

                            float result = transaction_capital_j - transaction_capital_k; //транзакция
                            char result_message[MAX_MESSAGE_LENGTH];
                            snprintf(result_message, sizeof(result_message), "Результат транзакции: %.2f", result);

                            write(users[j].pipe_fd[1], result_message, sizeof(result_message)); //Передача результата игрокам
                            write(users[k].pipe_fd[1], result_message, sizeof(result_message));

                            printf("\n Игрок %d и Игрок %d \n", users[j].id, users[k].id);
                            printf("Бюджет%d: %.2f \n", users[j].id, users[j].capital);
                            printf("Бюджет%d: %.2f \n", users[k].id, users[k].capital);
                            printf("Сумма транзакции %d: %.2f \n", users[j].id, transaction_capital_j);
                            printf("Сумма транзакции %d: %.2f \n", users[k].id, transaction_capital_k);
                            printf("Результат транзакции: %.2f\n", result);

                            users[j].capital += result; //обновление капитала
                            users[k].capital -= result;
                            printf("Бюджет%d: %.2f \n", users[j].id, users[j].capital);
                            printf("Бюджет%d: %.2f \n", users[k].id, users[k].capital);

                            users[j].state = 0; //сброс состояния игрока
                            users[k].state = 0;

			    transaction_count++;

			    if (users[k].capital <= 0) {
                                printf("Игрок %d обанкротился.\\n", users[k].id);
                                kill(users[k].pid, SIGTERM); // Завершаем дочерний процесс игрока
                                close(users[k].pipe_fd[0]);
                                close(users[k].pipe_fd[1]);
                                users[k].state = -1; // Обновляем состояние игрока на банкротство
                            }

                        }
                    }

                    if (strstr(message, "Банкрот") != NULL) {
                        printf("%s \n", message);
                    }
                }
            }
        }

        int active_users_count = 0; //подсчет активных пользователей
        for (int i = 0; i < num_user; i++) {
            if (users[i].capital > 0) {
                active_users_count++;
            }
        }

        if (active_users_count <= 1) { //если остался 1 и менее активных пользователей
            break;
        }

        int all_buying = 1;
        int all_selling = 1;

        for (int i = 0; i < num_user; i++) { //проверка итогового состояния для вывода
            if (users[i].capital > 0) {
                if (users[i].state != 1) {
                    all_buying = 0;
                }
                if (users[i].state != 2) {
                    all_selling = 0;
                }
            } else{
		   users[i].state = -1;
	    }
        }

        if (all_buying || all_selling) {
            break;
        }
    }

    printf("\nВсе игроки либо покупают, либо продают, либо обанкротились. Завершение работы.\n");
    for (int i = 0; i < num_user; i++) {
        const char *state_str;
        switch (users[i].state) {
            case 1:
                state_str = "покупает";
                break;
            case 2:
                state_str = "продает";
                break;
            case -1:
                state_str = "Банкрот";
                break;
        }
        printf("Игрок %d: финальный капитал %.2f, состояние: %s \n", users[i].id, users[i].capital, state_str);
    }

    close(epoll_fd);

    // Закрываем все файловые дескрипторы пайпов в родительском процессе
    for (int i = 0; i < num_user; i++) {
        close(users[i].pipe_fd[0]);
    }
     printf("\nВсего выполнено транзакций: %d\n", transaction_count);
}

void createUserProcesses(int num_user, User users[]) { //создание игрока
    for (int i = 0; i < num_user; i++) {
        users[i].id = i + 1;
        users[i].capital = ((float) rand() / RAND_MAX) * 1000.0 + 100.0;
        users[i].state = 0;
        if (pipe(users[i].pipe_fd) == -1) {
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
        fprintf(stderr, "Ошибка: %s введите количество игроков.\n", argv[0]);
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

    for (int i = 0; i < num_user; i++) {
        waitpid(users[i].pid, NULL, 0); // Ожидаем завершения всех дочерних процессов
    }

    free(users);
    printf("Все игроки завершили игру.\n");
    return 0;
}
