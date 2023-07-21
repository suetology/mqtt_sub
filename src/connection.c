#include "connection.h"

#include <stdlib.h>
#include <string.h>
#include <syslog.h>

void add_topic(struct Connection *connection, struct Topic *topic)
{
        if (connection->topics_count == 0)
                connection->topics = (struct Topic **)malloc(sizeof(struct Topic *));
        else 
                connection->topics = (struct Topic **)realloc(connection->topics, (connection->topics_count + 1) * sizeof(struct Topic *));
        
        if (connection->topics == NULL) {
                syslog(LOG_ERR, "Failed allocating/reallocating memory for topic %s", topic->name);
                return;
        }
        connection->topics[connection->topics_count] = topic;
        connection->topics_count++;
}

void delete_connection(struct Connection *connection)
{
        if (connection->topics == NULL) 
                return;
                
        for (int i = 0; i < connection->topics_count; i++)
                delete_topic(&(connection->topics[i]));
        
        free(connection->topics);
        connection->topics = NULL;

        if (connection->username != NULL)
                free(connection->username);
        if (connection->password != NULL)
                free(connection->password);
        if (connection->ca_file != NULL)
                free(connection->ca_file);
        if (connection->cert_file != NULL)
                free(connection->cert_file);
        if (connection->key_file != NULL)
                free(connection->key_file);
}

struct Topic *get_topic(struct Connection *connection, const char *topic_name)
{
        if (connection == NULL || topic_name == NULL) {
                syslog(LOG_ERR, "Trying to get a topic with null connection or topic name");
                return NULL;
        }

        for (int i = 0; i < connection->topics_count; i++) {
                if (strcmp(connection->topics[i]->name, topic_name) == 0)
                        return connection->topics[i];
        }
        return NULL;
}