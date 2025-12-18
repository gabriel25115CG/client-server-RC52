#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

client_node_t* add_client(client_node_t *head, int socket_fd, struct sockaddr_in address) {
    client_node_t *new_client = malloc(sizeof(client_node_t));
    if (!new_client) return head;
    new_client->socket_fd = socket_fd;
    new_client->address = address;
    new_client->role = ROLE_UNKNOWN;
    new_client->pseudo[0] = '\0';
    new_client->next = head;

    printf("Client %d added: %s:%d\n", socket_fd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    return new_client;
}

client_node_t* remove_client(client_node_t *head, int socket_fd) {
    client_node_t *current = head;
    client_node_t *prev = NULL;

    while (current != NULL) {
        if (current->socket_fd == socket_fd) {
            if (prev == NULL)
                head = current->next;
            else
                prev->next = current->next;
            printf("Client %d removed: %s:%d\n", socket_fd, inet_ntoa(current->address.sin_addr), ntohs(current->address.sin_port));
            close(socket_fd);
            free(current);
            return head;
        }
        prev = current;
        current = current->next;
    }
    return head;
}

void free_clients(client_node_t *head) {
    while (head != NULL) {
        client_node_t *temp = head;
        head = head->next;
        close(temp->socket_fd);
        free(temp);
    }
}

client_node_t* find_owner(client_node_t *head) {
    client_node_t *cur = head;
    while (cur) {
        if (cur->role == ROLE_OWNER) return cur;
        cur = cur->next;
    }
    return NULL;
}

client_node_t* find_client_by_fd(client_node_t *head, int fd) {
    client_node_t *cur = head;
    while (cur) {
        if (cur->socket_fd == fd) return cur;
        cur = cur->next;
    }
    return NULL;
}
