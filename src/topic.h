#ifndef TOPIC_H
#define TOPIC_H

#include "event.h"

struct Topic {
        char name[LENGTH];
        struct Event **events;
        int events_count;
};

struct Topic *create_topic(char *name);
void delete_topic(struct Topic **topic);
unsigned has_event(struct Topic *topic);
void add_event(struct Topic *topic, struct Event *event);
struct Topic *create_topic_from_file(char *name);

#endif