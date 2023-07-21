#include "uci_module.h"

#include <uci.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#define CONFIG_FILE "mqtt_sub"
#define TOPIC_SECTION_TYPE "topic"
#define EVENT_SECTION_TYPE "event"

static struct uci_package *config;
static struct uci_context *ctx;

char *get_option_value(struct uci_section *section, const char *option_name) 
{
        if (section == NULL || option_name == NULL)
                return NULL;
        
        struct uci_option *option = uci_lookup_option(ctx, section, option_name);
        if (option == NULL) {
                syslog(LOG_ERR, "Failed to get option %s in section %s", option_name, section->e.name);
                return NULL;
        }

        if (option->type != UCI_TYPE_STRING)
                return NULL;

        return option->v.string;
}

int get_list_size(struct uci_list *list)
{
        int i = 0;
        struct uci_element *e;
        uci_foreach_element(list, e)
                i++;
        return i;
}

struct uci_option *get_list(struct uci_section *section, const char *list_name)
{
        if (section == NULL || list_name == NULL)
                return NULL;

        struct uci_option *option = uci_lookup_option(ctx, section, list_name);
        if (option == NULL) {
                syslog(LOG_ERR, "Failed to get list %s in section %s", list_name, section->e.name);
                return NULL;
        }

        if (option->type != UCI_TYPE_LIST || uci_list_empty(&option->v.list))
                return NULL;

        return option;

        int size = get_list_size(&option->v.list);

        struct uci_element *e;
        uci_foreach_element(&option->v.list, e) {
                syslog(LOG_INFO, "%s", e->name);
        }
        return NULL;
}

struct Event *load_event(struct uci_section *section)
{
        char *value_name = get_option_value(section, "value_name");
        char *value_type_str = get_option_value(section, "value_type");
        char *comp_value_str = get_option_value(section, "comp_value");
        char *comp_type_str = get_option_value(section, "comp_type"); 
        char *email = get_option_value(section, "email");
        char *password = get_option_value(section, "password");
        char *message = get_option_value(section, "message");

        union Value comp_value;
        enum ValueType value_type;
        enum CompType comp_type;

        if (strcmp(value_type_str, "number") == 0) {
                value_type = NUMBER; 
                comp_value.number = atoi(comp_value_str);
        } else if (strcmp(value_type_str, "string") == 0) {
                value_type = STRING;
                comp_value.string = comp_value_str;
        } else {        
                syslog(LOG_ERR, "Trying to load an event with an invalid value_type (legit are: number, string)");
                return NULL;
        }

        if (strcmp(comp_type_str, "less") == 0) {
                comp_type = LESS; 
        } else if (strcmp(comp_type_str, "greater") == 0) {
                comp_type = GREATER;
        } else if (strcmp(comp_type_str, "less_equal") == 0) {
                comp_type = LESS_EQUAL;
        } else if (strcmp(comp_type_str, "greater_equal") == 0) {
                comp_type = GREATER_EQUAL;
        } else if (strcmp(comp_type_str, "equal") == 0) {
                comp_type = EQUAL;
        } else if (strcmp(comp_type_str, "not_equal") == 0) {
                comp_type = NOT_EQUAL;
        } else {        
                syslog(LOG_ERR, "Trying to load an event with an invalid comp_type (legit are: less, greater, less_equal, greater_equal, equal, not_equal)");
                return NULL;
        }

        struct uci_option *option = get_list(section, "recipients");
        int size = get_list_size(&option->v.list);

        char (*recipients)[LENGTH];
        recipients = (char (*)[])malloc(size * sizeof(*(recipients)));
        if (recipients == NULL) {
                syslog(LOG_ERR, "Failed allocating memory for recipients while loading config");
                return NULL;
        }

        struct uci_element *e;
        int i = 0;
        uci_foreach_element(&option->v.list, e) {
                int len = strlen(e->name);
                strncpy(recipients[i], e->name, len);
                recipients[i][len] = '\0';
                i++;
        }

        struct Event *event = create_event(value_name, value_type, comp_value, comp_type, email, password, message, (char **)recipients, size);
        if (event == NULL) {
                syslog(LOG_ERR, "Failed creating event");
                free(recipients);
                return NULL;
        }

        return event;
}

void load_events(struct Connection *connection)
{
        struct uci_element *section_e;
        struct uci_section *section;
        uci_foreach_element(&config->sections, section_e) {
                section = uci_to_section(section_e);
                if (strcmp(section->type, EVENT_SECTION_TYPE) == 0) {
                        char *topic_name = get_option_value(section, "topic_name");
                        struct Topic *topic = get_topic(connection, topic_name); 
                        if (topic == NULL) {
                                syslog(LOG_ERR, "Trying to add an event to topic %s that does not exist", topic_name);
                                return;
                        }
                        struct Event *event = load_event(section);
                        if (event == NULL) {
                                syslog(LOG_ERR, "Failed to load an event for topic %s", topic_name);
                                return;
                        }
                        add_event(topic, event);
                }
        }
}

void load_topics(struct Connection *connection)
{
        struct uci_element *section_e;
        struct uci_section *section;
        uci_foreach_element(&config->sections, section_e) {
                section = uci_to_section(section_e);
                if (strcmp(section->type, TOPIC_SECTION_TYPE) == 0) {
                        struct Topic *topic = create_topic(section->e.name);
                        add_topic(connection, topic);
                }
        }
}

int uci_module_load_config(struct Connection *connection)
{
        ctx = uci_alloc_context();
        if (ctx == NULL) {
                syslog(LOG_ERR, "Failed to allocate UCI context");
                return 1;
        }
        if (uci_load(ctx, CONFIG_FILE, &config) != UCI_OK) {
               syslog(LOG_ERR, "Failed to load configuration file");
               return 2;
        }

        load_topics(connection);
        load_events(connection);

        uci_free_context(ctx);
        return 0;
}