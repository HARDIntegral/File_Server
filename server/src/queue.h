#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct node {
    struct node *next;
    int* client_socket;
} node_t;

void enqueue(int* client_socket);
int* dequeue();

#endif /* __QUEUE_H__ */