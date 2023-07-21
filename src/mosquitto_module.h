#ifndef MOSQUITTO_MODULE_H
#define MOSQUITTO_MODULE_H

#include "connection.h" 

int mosquitto_module_init(struct Connection *connection);
int mosquitto_module_loop();
int mosquitto_module_reconnect();
void mosquitto_module_terminate();

#endif