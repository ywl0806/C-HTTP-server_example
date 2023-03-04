#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define PORT 3031
#define BUF_SIZE 4096
#define MAX_CLIENT 256

void error_handling(char *msg);
void *handle_http(void *arg);

int client_cnt = 0;
int client_socks[MAX_CLIENT];
pthread_mutex_t mutex;

int main(int argc, char *argv[])
{
	int serv_sock, client_sock;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_addr_size;
	pthread_t thread_id;

	pthread_mutex_init(&mutex, NULL);

	if ((serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		error_handling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(PORT);

	if (bind(serv_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		error_handling("bind() error");

	if (listen(serv_sock, 5))
		error_handling("listen() error");

	printf("listen on post : %d \n", PORT);
	while (1)
	{
		client_addr_size = sizeof(client_addr);
		client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &client_addr_size);

		pthread_mutex_lock(&mutex);
		client_socks[client_cnt++] = client_sock;
		pthread_mutex_unlock(&mutex);

		pthread_create(&thread_id, NULL, handle_http, (void *)&client_sock);
	}
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void *handle_http(void *args)
{
	int client_sock = *((int *)args);
	int str_len = 0;
	char msg[BUF_SIZE];
	printf("handle http req client socket: %d \n\n", client_sock);
	// http요청을 처리
	while ((str_len = read(client_sock, msg, sizeof(msg)) != 0))
	{
		printf("%s \n", msg);
	}
	pthread_mutex_lock(&mutex);
	for (int i = 0; i < client_cnt; i++)
	{
		if (client_sock == client_socks[i])
		{
			while (i++ < client_cnt - 1)
				client_socks[i] = client_socks[i + 1];
			break;
		}
	}
	client_cnt--;
	pthread_mutex_unlock(&mutex);
	close(client_sock);

	return NULL;
}