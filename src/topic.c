#include "topic.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

struct Topic *create_topic(char *name)
{
        if (name == NULL)
                return NULL;

        struct Topic *topic = (struct Topic *)malloc(sizeof(struct Topic));
        if (topic == NULL)
                return NULL;

        strcpy(topic->name, name);
        topic->events = NULL;
        topic->events_count = 0;

        return topic;
}

void delete_topic(struct Topic **topic)
{
        if (*topic == NULL)
                return;

        if ((*topic)->events != NULL) {
                for (int i = 0; i < (*topic)->events_count; i++)
                        delete_event(&((*topic)->events[i]));
                free((*topic)->events);
        }
        free(*topic);
        *topic = NULL;
}

unsigned has_event(struct Topic *topic)
{
        return topic->events_count > 0;
}

void add_event(struct Topic *topic, struct Event *event)
{
        if (topic->events_count == 0)
                topic->events = (struct Event **)malloc(sizeof(struct Event *));
        else
                topic->events = (struct Event **)realloc(topic->events, (topic->events_count + 1) * sizeof(struct Event *));

        if (topic->events == NULL) {
                syslog(LOG_ERR, "Failed allocating/reallocating memory for event");
                return;
        }
        topic->events[topic->events_count] = event;
        topic->events_count++;
}
