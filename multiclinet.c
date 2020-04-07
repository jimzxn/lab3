#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define max_worker 5
pthread_mutex_t lock;
pthread_cond_t fill_empty;

FILE* fd;
int port=8888;
char ip[100]="127.0.0.1";


void* client_thread(void* arg){

}

int main(int argc, char* argv[]){
    int socket_desc, new_socket, c;
    struct sockaddr_in  client;
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
    client.sin_addr.s_addr = INADDR_ANY; // defaults to 127.0.0.1
    client.sin_port = htons(port);
    // Bind (corrent the server's socket address to the socket descriptor)
    while (1)
    {
        c = sizeof(struct sockaddr_in);
        new_socket = connect(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (new_socket < 0)
        {
            puts("Error: Accept failed");
            continue;
        }
        puts("Connection accepted");
        // do something with new_socket
    }
}