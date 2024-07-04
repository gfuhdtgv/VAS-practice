#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/poll.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<time.h>

#define MAX_SIZE_DATA 4192
volatile sig_atomic_t signal_alarm=0; //объявл. переменной которая может меняться вне программы (асинхронно)

void s_alarm (int sig){
	signal_alarm=1;
}

ssize_t read_return (int fd, void *buf,size_t number){ //если чтение прервалось то повторяем его
	struct pollfd pfd = { .fd = fd, .events = POLLIN}; //иниц. poll для чтения
	ssize_t data_read;
	while ((data_read = read(fd,buf,number))==-1) { //чтение
		if (errno == EINTR || errno ==EWOULDBLOCK){ //если временно недоступно чтение 
			int ret = poll(&pfd, 1 , -1); //ожидание готовности к чтению (-1 бесконечное ожидание)
			if (ret == -1){
				perror("Ошибка poll");
				return -1;
			}
			continue;
		}else if (errno == EINTR){
			if (signal_alarm){
				return -1;
				continue;
			}
		}else {
			perror("Ошибка чтения");
			return -1;
		}
	}
	return data_read;
}

ssize_t write_return (int fd, void *buf,ssize_t number){ //если запись прервалась то повторяем ее
        struct pollfd pfd = { .fd = fd, .events = POLLOUT}; //иниц. poll для записи
	ssize_t data_write;

        while ((data_write = write(fd,buf,number))==-1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			int ret = poll(&pfd, 1 , -1);
			if (ret == -1){
				perror("Ошибка poll");
				return -1;
			}
			continue;
		} else if (errno == EINTR){
			if (signal_alarm){
				return -1;
				continue;
			}
		}else {
			perror("Ошибка записи");
			return -1;
		}
	}
        return data_write;
}

int init_file (const char *filename){
	int file = open(filename, O_RDONLY | O_NONBLOCK); //только для чтения, не блокирующий
	if (file == -1){
		perror("Ошибка чтения файла");
		exit(EXIT_FAILURE);
	}
	return file;
}

void create_socket_pair(int socket_fd[2]){ // создание сокет-пары
	if (socketpair(AF_UNIX,SOCK_STREAM|SOCK_NONBLOCK,0,socket_fd) == -1){
		perror("Ошибка создания сокет-пары");
		exit(EXIT_FAILURE);
	}
}

void parent_process(int file, int socket_fd[2], int feedback_fd[2]){ //Родительский процесс, отправляет данные рандомно, проверяет правильность доставки
	srand(time(NULL)); //Рандом при каждом запуске
	close (socket_fd[1]); //Закрываем не использ.поток.
	close (feedback_fd[0]); //Закр. 1ый дискрипт. обратной связи.
	
	char data_file[MAX_SIZE_DATA];
	ssize_t byte_read;
	ssize_t total_byte = 0;
	ssize_t total_get_child = 0;

	while ((byte_read = read_return(file, data_file, rand() % MAX_SIZE_DATA + 1)) > 0 ){
		printf("Родительский процесс отправил: %zd байт\n", byte_read);
		total_byte += byte_read;

		if (write_return(socket_fd[0], &byte_read, sizeof(byte_read)) == -1 || write_return(socket_fd[0],data_file,byte_read) == -1){
			perror("Ошибка записи");
			break;
		}

		ssize_t get_child;
		if (read_return(feedback_fd[1], &get_child,sizeof(get_child)) == -1 ) {
			perror("Ошибка чтения обратной связи");
			break;
		}

		total_get_child += get_child;
		printf("Родительский процесс получил подтверждение: %zd байт  \n", get_child);

		if (total_byte != total_get_child) {
			fprintf(stderr,"Ошибка: не совпадение отправленных и полученных данных");
			break;
		}
	}

	close(socket_fd[0]);
	close(feedback_fd[1]);

	printf("Передача файла завершена.\n Всего отправлено: %zd байт \n Всего получено: %zd байт\n",total_byte,total_get_child);
}

void child_process(int socket_fd[2], int feedback_fd[2]) { //получает данные, отправляет подтверждение
	close(socket_fd[0]);
	close(feedback_fd[1]);

	char data_file[MAX_SIZE_DATA];
	ssize_t get_byte;

	while(1){
		ssize_t byte_read=read_return(socket_fd[1],&get_byte,sizeof(get_byte));
		if (byte_read <= 0){	
			if (byte_read == 0){
				printf("конец передачи данных в дочернем процессе\n");
			} else{
				perror("Ошибка чтения в дочернем процессе");
			}
			break;
		}

		printf("Дочерний процесс получил: %zd байт\n",get_byte);

		byte_read=read_return(socket_fd[1],data_file,get_byte);

		if (byte_read <= 0){  
                        if (byte_read == 0){
                                printf("конец передачи данных в дочернем процессе\n");
                        } else{
                                perror("Ошибка чтения в дочернем процессе");
                        }
                        break;
                }

		if (write_return(feedback_fd[0],&get_byte,sizeof(get_byte)) == -1){
			perror("Ошибка записи в дочернем процессе");
			break;
		}
	}
	close(socket_fd[1]);
	close(feedback_fd[0]);

	printf("Дочерний процесс получил данные \n");
}

void create_child_process (int file, int socket_fd[2], int feedback_fd[2]){//создание доч.процесса
	pid_t pid = fork();
	
	if(pid == -1){
		perror("Ошибка создания процесса");
		exit(EXIT_FAILURE);
	} else if (pid > 0){
		parent_process(file, socket_fd,feedback_fd);
		wait(NULL);
		close(file);
	} else{
		child_process(socket_fd,feedback_fd);
		exit(0);
	}
}

int main(int argc, char *argv[]){

	if (argc != 2){
		fprintf(stderr, "Ошибка: %s некоректный путь файла \n", argv[0]);
		exit(EXIT_FAILURE);
	}

	signal(SIGALRM, s_alarm); //Вызов обработки сигнала

	const char *file_path = argv[1]; 
	int file = init_file(file_path);
	
	int socket_fd[2];
	create_socket_pair(socket_fd);

	int feedback_fd[2];
	create_socket_pair(feedback_fd);

	create_child_process(file,socket_fd,feedback_fd);

	return 0;
}

