#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<sys/wait.h>
#include<sys/resource.h>
#include<time.h>

#define MAX_EVENTS 1000

void log_error(const char *message){ //Для вывода сообщений об ошибках
	fprintf(stderr, "Ошибка: %s\n", message);
}

void check_args(int argc, char *argv[]){
	if (argc != 4){
		log_error("Введите IP, начальный порт, конечный порт");
		exit(EXIT_FAILURE);
	}
	int start_port = atoi(argv[2]);
	int end_port = atoi(argv[3]);

	if (start_port<=0 || end_port<=0 || start_port>end_port){
		log_error("Некоректный диапазон портов");
		exit(EXIT_FAILURE);
	}
}

int nonblock_sock (int sock_fd){ //nonblock для сокетов
	int flag = fcntl(sock_fd, F_GETFL,0);

	if (flag == -1){
		return -1;
	}
	return fcntl(sock_fd, F_SETFL, flag | O_NONBLOCK);
}

int max_open_files(){ //получения органичений на фд для динамич. отслеживания
	struct rlimit limit;
	
	if (getrlimit(RLIMIT_NOFILE,&limit)==-1){ //огранич. на кол-во фд для процесса
		perror("Ошибка получения ограничений файл.дискр.");
		exit(EXIT_FAILURE);
	}

	return limit.rlim_cur;
}

void scan_port (const char *ip, int start_port, int end_port){ //Сканирование портов
	int epoll_fd = epoll_create1(0);
	
	if (epoll_fd == -1){
		log_error("Ошибка создания epoll");
		exit(EXIT_FAILURE);
	}

	struct epoll_event event;
	struct epoll_event events[MAX_EVENTS];

	int max_socket = max_open_files();
	int open_socket = 0;

	for (int port = start_port; port <= end_port; port++){ //идем по портам

		if(open_socket >= max_socket - 10){ //если создалось максимум сокетов
			int s_wait = epoll_wait(epoll_fd,events,MAX_EVENTS, 1000);
			
			if (s_wait == -1){
				log_error("Ошибка ожидания события epoll");
				break;
			}

			for (int i = 0; i < s_wait;i++){ 

				int sock_fd = events[i].data.fd;
				int error_sock = 0;
				
				socklen_t lenght = sizeof(error_sock);

				if (getsockopt(sock_fd,SOL_SOCKET,SO_ERROR,&error_sock,&lenght)==-1){//получение дескрипт.сокета
					close(sock_fd);
					open_socket--;
					continue;
				}

				struct sockaddr_in addr;
				socklen_t addr_len = sizeof(addr);

				if (getpeername(sock_fd, (struct sockaddr*)&addr, &addr_len)== -1){ //получение инф-ии о подключении
					close(sock_fd);
					open_socket--;
					continue;
				}
				printf ("Порт %d открыт \n",ntohs(addr.sin_port));
				close(sock_fd);
				open_socket--;
			}
		}
		int sock_fd = socket(AF_INET,SOCK_STREAM,0); //создание сокета

		if(sock_fd == -1){
			fprintf(stderr,"Ошибка: не  создан сокет для порта %d: %s\n", port , strerror(errno));
			close(sock_fd);
			continue;
		}

		if (nonblock_sock(sock_fd)==-1){ 
			fprintf(stderr,"Ошибка:не удалось установить o_nonblock на порт %d  %s\n", port, strerror(errno));
			close(sock_fd);
			continue;
		}
		
		struct sockaddr_in server_addr;
		memset(&server_addr,0,sizeof(server_addr));

		server_addr.sin_family = AF_INET; //IPv4
		server_addr.sin_port = htons(port);//преобраз. из формата хоста в сетевой
		inet_pton(AF_INET,ip,&server_addr.sin_addr);//преобраз.из строки в бинар
		
		connect(sock_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)); //соединю с сервером

		event.data.fd = sock_fd;
		event.events = EPOLLOUT | EPOLLET;

		if (epoll_ctl(epoll_fd,EPOLL_CTL_ADD,sock_fd,&event)==-1){ //Добавление сокета в epoll
			fprintf(stderr,"Ошибка добавления сокета: %d %s\n",port,strerror(errno));
			close(sock_fd);
			continue;
		}
		open_socket++;
	}
	while (open_socket > 0){ //проверка портов
		int s_wait = epoll_wait(epoll_fd,events,MAX_EVENTS, 1000);

		if (s_wait == -1){
			log_error("Ошибка ожидания события epoll");
			break;
		}
			
		for (int i=0; i<s_wait; i++){
			int sock_fd = events[i].data.fd;
			int error_sock = 0;

			socklen_t lenght = sizeof(error_sock);
			if (getsockopt(sock_fd,SOL_SOCKET,SO_ERROR,&error_sock,&lenght)==-1){
				close(sock_fd);
				open_socket--;
				continue;
			}

			struct sockaddr_in addr;
			socklen_t addr_len = sizeof(addr);

			if (getpeername(sock_fd, (struct sockaddr*)&addr, &addr_len)== -1){
				close(sock_fd);
				open_socket--;
				continue;
			}
			printf ("Порт %d открыт \n",ntohs(addr.sin_port));
			close(sock_fd);
			open_socket--;
		}
	}
	close(epoll_fd);
}

void child_processes(const char *ip, int start_port, int end_port){
	int num_processes = 4;
	int port_range = (end_port - start_port + 1)/num_processes; //разбитие на минидиапазоны для доч.процессов

	for(int i = 0; i<num_processes;i++){
		pid_t pid = fork();

		if (pid == -1){
			log_error("создание процесса");
			exit(EXIT_FAILURE);
		} else if (pid == 0){
			int child_start = start_port + i * port_range; //начальный + смещение от номера
			int child_end = child_start + port_range - 1; //начальный + диапазон -1
			
			if (i == num_processes - 1){
				child_end = end_port;
			}

			printf ("Дочерний процесс сканирует от %d до  %d\n",child_start,child_end);
			scan_port(ip,child_start,child_end);
			exit(EXIT_SUCCESS);
		}
	}
	for (int i=0; i<num_processes; i++){
		wait(NULL);
	}
}

int main(int argc, char *argv[]){
	check_args(argc,argv);

	const char *ip = argv[1];
	int start_port = atoi(argv[2]);
	int end_port = atoi(argv[3]);

	struct timespec start_time, end_time;

	clock_gettime(CLOCK_MONOTONIC, &start_time);

	child_processes(ip,start_port,end_port);

	clock_gettime(CLOCK_MONOTONIC,&end_time);

	double sum_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

	printf("Время работы программы: %f\n",sum_time);

	return 0;
}
