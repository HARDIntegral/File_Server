#include <stdlib.h>

#include "queue.h"

node_t* head_node = NULL;
node_t* tail_node = NULL;

void enqueue(int* client_socket) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    new_node->client_socket = client_socket;
    new_node->next = NULL;
    if (tail_node == NULL) 
        head_node = new_node;
    else 
        tail_node->next = new_node;
    tail_node = new_node;
}

int* dequeue() {
    if (head_node == NULL)
        return NULL;
    int* result = head_node->client_socket;
    node_t* tmp = head_node;
    head_node = head_node->next;
    if (head_node == NULL)
        tail_node = NULL;
    free(tmp);
    return result;
}