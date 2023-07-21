#ifndef EVENT_H
#define EVENT_H

#define LENGTH 256

enum CompType {
        LESS,
        GREATER,
        LESS_EQUAL,
        GREATER_EQUAL,
        EQUAL,
        NOT_EQUAL
};

enum ValueType {
        NUMBER,
        STRING
};

union Value {
        int number;
        char *string;
};

struct Event {
        char value_name[LENGTH];
        enum ValueType value_type; 
        union Value comp_value;
        enum CompType comp_type;
        char email[LENGTH];
        char email_password[LENGTH];
        char message[LENGTH];
        char (*recipients)[LENGTH];
        int recipients_count;
};

struct Event *create_event(char *value_name, enum ValueType value_type, union Value comp_value, 
                                enum CompType comp_type, char *email, char *email_password, char *message,
                                char **recipients, int recipients_count);
void delete_event(struct Event **event);
void process_event(char *msg_payload, struct Event *event);
struct Event *create_event_from_file(char *filename);

#endif