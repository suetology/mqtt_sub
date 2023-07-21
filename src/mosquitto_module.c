#include "mosquitto_module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <mosquitto.h>

#include "topic.h"
#include "event.h"
#include "message_manager.h"

static int id = 0;
static struct mosquitto *mosq = NULL;
static struct Connection *connection = NULL;

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
        if (rc != 0)
                syslog(LOG_ERR, "Error while connecting to mosquitto with result code: %d", rc);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
        message_manager_store_msg(msg->topic, msg->payload);

        syslog(LOG_INFO, "New message with topic { %s }: %s", msg->topic, (char *)msg->payload);

        for (int i = 0; i < connection->topics_count; i++) {
                struct Topic *topic = connection->topics[i];
                if (strcmp(topic->name, msg->topic) == 0 && has_event(topic))
                        for (int j = 0; j < topic->events_count; j++)
                                process_event((char *)msg->payload, topic->events[j]);
        }
}   

int set_tls()
{
        if (connection->ca_file == NULL && connection->cert_file == NULL && connection->key_file == NULL)
                return MOSQ_ERR_SUCCESS;

        return mosquitto_tls_set(mosq, connection->ca_file, NULL, connection->cert_file, connection->key_file, NULL);
}

int set_user()
{
        if (connection->username == NULL && connection->password == NULL)
                return MOSQ_ERR_SUCCESS;

        return mosquitto_username_pw_set(mosq, connection->username, connection->password);
}

int mosquitto_module_init(struct Connection *conn)
{
        connection = conn;

        mosquitto_lib_init();
        mosq = mosquitto_new(NULL, true, &id);

        set_user();
        set_tls();

        mosquitto_connect_callback_set(mosq, on_connect);
        mosquitto_message_callback_set(mosq, on_message);
        if (mosquitto_connect(mosq, conn->host, conn->port, 10) != MOSQ_ERR_SUCCESS) {
                syslog(LOG_ERR, "Failed to connect to MQTT broker");
                return 1;
        }

        for (int i = 0; i < conn->topics_count; i++) 
                mosquitto_subscribe(mosq, NULL, conn->topics[i]->name, 0);

        return 0;
}

int mosquitto_module_loop()
{
        return mosquitto_loop(mosq, -1, 1);
}

void mosquitto_module_terminate() 
{       
        if (mosq != NULL) {
                mosquitto_disconnect(mosq);
                mosquitto_destroy(mosq);
        }
        mosquitto_lib_cleanup();
}
