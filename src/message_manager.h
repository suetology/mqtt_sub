#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include "event.h"

int message_manager_init(const char *storage_path);
int message_manager_store_msg(const char *topic, const char *payload);
int message_manager_send_email(struct Event *event);
void message_manager_terminate();

#endif