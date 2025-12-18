#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <netinet/in.h>

typedef enum { ROLE_UNKNOWN = 0, ROLE_OWNER, ROLE_TENANT } client_role_t;

typedef struct client_node {
    int socket_fd;
    struct sockaddr_in address;
    client_role_t role;
    char pseudo[64]; // pour propriétaire ou champ temporaire (ex: compteur tentatives pour tenant)
    struct client_node *next;
} client_node_t;

client_node_t* add_client(client_node_t *head, int socket_fd, struct sockaddr_in address);
client_node_t* remove_client(client_node_t *head, int socket_fd);
void free_clients(client_node_t *head);
client_node_t* find_owner(client_node_t *head);
client_node_t* find_client_by_fd(client_node_t *head, int fd);

#endif // LINKED_LIST_H
