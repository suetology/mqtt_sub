#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>

#include "event.h"
#include "daemon.h"
#include "arg_parser.h"
#include "uci_module.h"
#include "connection.h"
#include "message_manager.h"
#include "mosquitto_module.h"

static unsigned program_running = 0;

void signal_handler(int sig) 
{
        program_running = 0;
}

int main(int argc, char *argv[])
{
        openlog("MQTT Subscriber", LOG_PID, 0);
        syslog(LOG_INFO, "Starting program");

        program_running = 1;

        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGQUIT, signal_handler);

        unsigned is_daemon = 0;
        struct Connection connection = { 0 };
        if (parse_arguments(argc, argv, &is_daemon, &connection) != 0) {
                syslog(LOG_ERR, "Failed to parse arguments");
                delete_connection(&connection);
                closelog();
                return 1;
        }

        if (is_daemon) {
                if (make_daemon(NO_CLOSE_FILES) != 0) {
                        syslog(LOG_ERR, "Failed to create a daemon process");
                        delete_connection(&connection);
                        closelog();
                        return 2;
                } else {
                        syslog(LOG_INFO, "Created daemon process");
                }
        } 

        if (uci_module_load_config(&connection) != 0) {
                delete_connection(&connection);
                closelog();
                return 3;
        }

        if (connection.topics_count == 0) {
                syslog(LOG_ERR, "No topics specified in config file");
                delete_connection(&connection);
                closelog();
                return 4;
        }

        message_manager_init(NULL);

        if (mosquitto_module_init(&connection) != 0) {
                delete_connection(&connection);
                closelog();
                return 5;
        }

        while (program_running) {
                if (mosquitto_module_loop() != 0) {
                        syslog(LOG_ERR, "Error in mosquitto loop");
                }
        }
        
        mosquitto_module_terminate();
        message_manager_terminate();

        delete_connection(&connection);
        syslog(LOG_INFO, "Program has ended");
        closelog();

        return 0;
}