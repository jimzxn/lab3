#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define max_worker 10
//global varible
int port = 8888;
char dictionary[100] = "dictionary.txt";
int dic_size = 0;
char dic_arr[100000][50];
//queue varible
int log_fornt = 0, log_rear = -1, log_count = 0;
char *log_queue[100]; //need init at main

int sock_fornt = 0, sock_rear = -1, sock_count = 0;
int sock[100];
//pthread varible
pthread_cond_t s_empty, s_fill;
pthread_cond_t log_empty, log_fill;
pthread_mutex_t s_mutex, log_mutex;

void *worker_theard(void *arg)
{
    int socket_desc = 0;
    while (1)
    {
        pthread_mutex_lock(&s_mutex);
        //dequeue

        while (sock_count == 0)
        {
            pthread_cond_wait(&s_fill, &s_mutex);
        }
        socket_desc = sock[0];
        for (size_t i = 0; i < sock_count; i++)
        {
            sock[i] = sock[i + 1];
        }
        sock_rear--;
        sock_count--;
        //dequeue end
        pthread_cond_signal(&s_empty);
        pthread_mutex_unlock(&s_mutex);
        char word[100] = "\0";
        int len;
        while ((len = read(socket_desc, word, 100)) > 0)
        {
            if (word[len - 1] == '\n')
            {
                word[len - 1] = '\0';
            }
            char result[20] = "\0";
            int c = 0;
            while (c < dic_size)
            {
                if (strcmp(word, dic_arr[c]) == 0)
                {
                    strcpy(result, " OK");
                    break;
                }
                else
                {
                    strcpy(result, "MISSPLELLED");
                }
                c++;
            }
            strcat(word, result);
            strcat(word, "\n");
            printf("output:%s\n", word);
            write(socket_desc, word, strlen(word));
            //log
            pthread_mutex_lock(&log_mutex);
            //enqueue log
            while (log_count == 100)
            {
                pthread_cond_wait(&log_empty, &log_mutex);
            }
            log_rear++;
            strcpy(log_queue[log_rear], word);
            log_count++;
            printf("work log count:%d\n", log_count);
            //done
            pthread_cond_signal(&log_fill);
            pthread_mutex_unlock(&log_mutex);
            word[0] = '\0';
        }
        //clear fd for new loop
        close(socket_desc);
    }
}

void *log_thread(void *arg)
{

    while (1)
    {
        pthread_mutex_lock(&log_mutex);
        while (log_count == 0)
        {
            pthread_cond_wait(&log_fill, &log_mutex);
        }
        //dequeue
        FILE *fd = fopen("LOG.txt", "a+");
        fprintf(fd, "%s", log_queue[0]);
        fclose(fd);
        for (size_t i = 0; i < log_count; i++)
        {
            log_queue[i] = log_queue[i + 1];
        }
        log_rear--;
        log_count--;
        //dequeue end
        pthread_cond_signal(&log_empty);
        pthread_mutex_unlock(&log_mutex);
    }
}
int main(int argc, char *argv[])
{
    //inital varible
    pthread_cond_init(&log_empty, 0);
    pthread_cond_init(&s_empty, 0);
    pthread_cond_init(&log_fill, 0);
    pthread_cond_init(&s_fill, 0);
    pthread_mutex_init(&s_mutex, 0);
    pthread_mutex_init(&log_mutex, 0);
    //done
    //arg check
    if (argc == 3)
    {
        strcpy(dictionary, argv[2]);
        port = atoi(argv[1]);
    }
    if (argc == 2)
    {
        if (strstr(argv[1], "txt"))
        {

            strcpy(dictionary, argv[1]);
        }
        else
        {
            port = atoi(argv[1]);
        }
    }
    //dix init
    FILE *fd;
    if ((fd = fopen(dictionary, "r")) == NULL)
    {
        exit(-1);
    }
    int index = 0;
    while ((fscanf(fd, "%s\n", dic_arr[index]) != EOF))
    {
        index++;
    }
    dic_size = index;
    //done
    //queue init
    for (size_t i = 0; i < 100; i++)
    {
        log_queue[i] = malloc(100);
    }

    pthread_t w_thread[max_worker];
    for (size_t i = 0; i < max_worker; i++)
    {
        pthread_create(&(w_thread[i]), NULL, &worker_theard, NULL);
    }

    pthread_t log;
    pthread_create(&log, NULL, log_thread, NULL);

    //socket
    int portNumber = port;
    int socket_desc, new_socket, c;
    struct sockaddr_in server, client;
    char *message;
    // Create socket (create active socket descriptor)
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        puts("Error creating socket!");
        exit(1);
    }
    // prepare sockaddr_instructure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // defaults to 127.0.0.1
    server.sin_port = htons(portNumber);
    // Bind (corrent the server's socket address to the socket descriptor)
    int bind_result = bind(socket_desc, (struct sockaddr *)&server, sizeof(server));
    if (bind_result < 0)
    {
        puts("Error: failed to Bind.");
        exit(1);
    }
    puts("Bind done.");
    // Listen (converts active socket to a LISTENING socket which can accept connections)
    listen(socket_desc, 3);
    puts("Waiting for incoming connections...");
    while (1)
    {
        c = sizeof(struct sockaddr_in);
        new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (new_socket < 0)
        {
            puts("Error: Accept failed");
            continue;
        }
        puts("Connection accepted");
        // do something with new_socket
        pthread_mutex_lock(&s_mutex);
        while (sock_count == 100)
        {
            pthread_cond_wait(&s_empty, &s_mutex);
        }
        sock_rear++;
        sock[sock_rear] = new_socket;
        printf("main:%d:::\n", new_socket);
        sock_count++;
        puts("Connection enqueued");
        pthread_cond_signal(&s_fill);
        pthread_mutex_unlock(&s_mutex);
    }
}