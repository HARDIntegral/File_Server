#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>

#include "queue.h"

#define SERVER_PORT 6969
#define BUFF_SIZE   4096
#define SOCKET_ERR  (-1)
#define BACKLOG     1

#define THREAD_POOL_SIZE 16
pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* connection_handler(void* p_client_socket);
int check(int exp, const char* msg);
void* thread_function(void* arg);

int main() {
    
    int server_socket, client_socket, addr_size;
    struct sockaddr_in server_addr, client_addr;

    for (int i=0; i<THREAD_POOL_SIZE; i++)
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);

    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), "[SERVER] Failed to create socket");
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(SERVER_PORT);
    
    check(bind(
        server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)
        ), "[SERVER] Bind failed");
    check(listen(server_socket, BACKLOG), "[SERVER] Listen failed");
    
    while (true) {
        printf("[SERVER] Waiting for connections ...\n");

        addr_size = sizeof(struct sockaddr_in);
        check(client_socket = 
            accept(server_socket, (struct sockaddr*)&client_addr, (socklen_t*)&addr_size),
            "[SERVER] Accept failed"
        );
        printf("[SERVER] Connected!\n");

        int* p_client = (int*)malloc(sizeof(int));
        *p_client = client_socket;
        pthread_mutex_lock(&mutex);
        enqueue(p_client);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}

void* connection_handler(void* p_client_socket) {
    int client_socket = *(int*)p_client_socket;
    free(p_client_socket);

    char buffer[BUFF_SIZE];
    size_t bytes_read;
    int msg_size = 0;
    char actual_path[PATH_MAX+1];

    // read client message
    while ((bytes_read = read(client_socket, buffer+msg_size, sizeof(buffer)-msg_size-1)) > 0) {
        msg_size += bytes_read;
        if (msg_size > BUFF_SIZE-1 || buffer[msg_size-1] == '\n') 
            break;
    }
    check(bytes_read, "[SERVER] Receive error\n");
    buffer[msg_size-1] = 0;     // null terminate message and remove \n
    
    printf("[SERVER] Client Request: %s\n", buffer);
    fflush(stdout);

    // verify requested path
    if (realpath(buffer, actual_path) == NULL) {
        printf("[SERVER] ERROR (bad path): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    FILE* fp = fopen(actual_path, "r");
    if (fp == NULL) {
        printf("[SERVER] ERROR (open) %s\n", actual_path);
        close(client_socket);
        return NULL;
    }

    while ((bytes_read = fread(buffer, 1, BUFF_SIZE, fp)) > 0) {
        printf("[SERVER] sending %zu bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }
    close(client_socket);
    fclose(fp);
    printf("[SERVER] closing connection\n\n"); 
}

int check(int exp, const char* msg) {
    if (exp == SOCKET_ERR) {
        perror(msg);
        exit(1);
    }
    return exp;
}

void* thread_function(void* arg) {
    while (true) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        int* p_client = dequeue();
        pthread_mutex_unlock(&mutex);
        
        if (p_client)
            connection_handler(p_client);
    }
}