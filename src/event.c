#include "event.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "json_module.h"
#include "message_manager.h"

struct Event *create_event(char *value_name, enum ValueType value_type, union Value value, 
                                enum CompType comp_type, char *email, char *email_password, 
                                char *message, char **recipients, int recipients_count) 
{
        if (value_type == STRING && comp_type != EQUAL && comp_type != NOT_EQUAL) {
                syslog(LOG_ERR, "'string' values can be compared using only 'equal' or 'not_equal'");
                return NULL;
        }

        struct Event *event = (struct Event *)malloc(sizeof(struct Event));
        if (event == NULL)
                return NULL;

        event->value_type = value_type;
        event->comp_type = comp_type;

        event->recipients = (char (*)[])recipients;
        event->recipients_count = recipients_count;

        strcpy(event->value_name, value_name);
        strcpy(event->email, email);
        strcpy(event->email_password, email_password);
        strcpy(event->message, message);

        if (value_type == NUMBER) {
                event->comp_value.number = value.number;
        } else if (value_type == STRING) {
                event->comp_value.string = (char *)malloc(strlen(value.string) + 1);
                if (event->comp_value.string == NULL) {
                        syslog(LOG_ERR, "Failed creating event");
                        free(event);
                        return NULL;
                }
                strcpy(event->comp_value.string, value.string);
        }

        return event;
}

void delete_event(struct Event **event)
{
        if (*event == NULL)
                return;

        if ((*event)->recipients != NULL)
                free((*event)->recipients);
        
        if ((*event)->value_type == STRING && (*event)->comp_value.string != NULL)
                free((*event)->comp_value.string);

        free(*event);
        *event = NULL;
}

unsigned condition_met(union Value actual_value, union Value comp_value, enum ValueType value_type, enum CompType comp_type) 
{
        if (value_type == NUMBER) {
                switch (comp_type) {
                case LESS:
                        return actual_value.number < comp_value.number;
                case GREATER:
                        return actual_value.number > comp_value.number;
                case LESS_EQUAL:
                        return actual_value.number <= comp_value.number;
                case GREATER_EQUAL:
                        return actual_value.number >= comp_value.number;
                case EQUAL:
                        return actual_value.number == comp_value.number;
                case NOT_EQUAL:
                        return actual_value.number != comp_value.number;
                }
        } else if (value_type == STRING) {
                switch (comp_type) {
                case EQUAL:
                        return strcmp(actual_value.string, comp_value.string) == 0;
                case NOT_EQUAL:
                        return strcmp(actual_value.string, comp_value.string) != 0;
                }
        }
        return 0;
}

void process_event(char *msg_payload, struct Event *event)
{
        union Value value;
        value.string = NULL;

        if (event->value_type == NUMBER) {
                if (extract_data_number(&value.number, msg_payload, event->value_name) != 0) {
                        syslog(LOG_ERR, "Failed extracting %s number value from event message payload", event->value_name);
                        return;
                }
        } else if (event->value_type == STRING) {
                value.string = (char *)malloc(LENGTH);
                if (value.string == NULL) {
                        syslog(LOG_ERR, "Failed allocating memory for value string");
                        return;
                }
                if (extract_data_string(value.string, LENGTH, msg_payload, event->value_name) != 0) {
                        syslog(LOG_ERR, "Failed extracting %s string value from event message payload", event->value_name);
                        free(value.string);
                        return;
                }
        }

        if (condition_met(value, event->comp_value, event->value_type, event->comp_type)) {
                printf("\n\n%s\n\n", event->value_name);
                message_manager_send_email(event);
        }

        if (event->value_type == STRING && value.string != NULL)
                free(value.string);
}

//delete later
struct Event *create_event_from_file(char *filename)
{
        char name[256];
        enum ValueType value_type;
        union Value value;
        enum CompType comp_type;
        char email[256];
        char password[256];

        FILE *file = fopen(filename, "r");
        if (file == NULL)
                return NULL;
        
        char line[256];
        fgets(line, sizeof(line), file);
        line[strcspn(line, "\n")] = '\0';
        strcpy(name, line);
        
        fgets(line, sizeof(line), file);
        line[strcspn(line, "\n")] = '\0';
        value_type = (enum ValueType)atoi(line);

        fgets(line, sizeof(line), file);
        line[strcspn(line, "\n")] = '\0';
        value.number = atoi(line);

        fgets(line, sizeof(line), file);
        line[strcspn(line, "\n")] = '\0';
        comp_type = (enum CompType)atoi(line);

        fgets(line, sizeof(line), file);
        line[strcspn(line, "\n")] = '\0';
        strcpy(email, line);

        fgets(line, sizeof(line), file);
        line[strcspn(line, "\n")] = '\0';
        strcpy(password, line);

        fclose(file);

        return create_event(name, value_type, value, comp_type, email, password, "msg", NULL, 0);
}