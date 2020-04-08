#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

extern int errno;
#define max_worker 5
pthread_mutex_t end;
pthread_cond_t finish;

char name[50] = "clinet.txt";
int port = 8888;
char ip[100] = "127.0.0.1";
void *client_thread(void *arg)
{
    int socket_desc, new_socket, c;
    struct sockaddr_in client;
    char *message;
    // Create socket (create active socket descriptor)
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        puts("Error creating socket!");
        exit(1);
    }
    // prepare sockaddr_instructure
    client.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &client.sin_addr); // defaults to 127.0.0.1
    client.sin_port = htons(port);
    // Bind (corrent the server's socket address to the socket descriptor)
    int i = 0;

    c = sizeof(struct sockaddr_in);
    new_socket = connect(socket_desc, (struct sockaddr *)&client, sizeof(client));

    FILE *fd;
    fd = fopen(name, "r");
    char buffer[20];
    char recver[20];
    while (fscanf(fd, "%s\n", buffer) != EOF)
    {
        int i = send(socket_desc, buffer, strlen(buffer), 0);
    }
    fclose(fd);
}

int main(int argc, char *argv[])
{
    pthread_t w_thread[max_worker];
    for (size_t i = 0; i < max_worker; i++)
    {
        pthread_create(&(w_thread[i]), NULL, &client_thread, NULL);
    }
    for (int i = 0; i < 2; i++)
        pthread_join(w_thread[i], NULL);
}