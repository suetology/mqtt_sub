#ifndef CONNECTION_H
#define CONNECTION_H

#include "topic.h"

#define DEFAULT_PORT 1883
#define TLS_CONN_PORT 8883 
#define HOST_LEN 15

struct Connection {
        int port;
        char host[HOST_LEN];
        struct Topic **topics;
        int topics_count;
        char *username;
        char *password;
        char *ca_file;
        char *cert_file;
        char *key_file;
};

void delete_connection(struct Connection *connection);
void add_topic(struct Connection *connection, struct Topic *topic);
struct Topic *get_topic(struct Connection *connection, const char *topic_name);

#endif