#include "arg_parser.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <getopt.h>

#include "event.h"

static char args_description[] = "Usage:\n"
                                 "-?                    displays this message\n"
                                 "-D                    creates a daemon process\n"
                                 "-h, --host            mqtt host to connect to. Defaults to localhost\n"
                                 "-p, --port            network port to connect to. Defaults to 1883 for plain MQTT and 8883 for MQTT over TLS\n"
                                 "-u, --username        provide a username\n"
                                 "-P, --password        provide a password\n"
                                 "--cafile              path to a directory containing trusted CA certificates to enable encrypted communication\n"
                                 "--cert                client certificate for authentication, if required by server\n"
                                 "--key                 client private key for authentication, if required by server\n";

struct Arguments {
        unsigned is_daemon;
        int port;
        char *host;
        char *username;
        char *password;
        char *ca_file;
        char *cert_file;
        char *key_file;
};

static struct option long_options[] = {
        { "port",     required_argument, 0,  'p' },
        { "host",     required_argument, 0,  'h' },
        { "username", required_argument, 0,  'u' },
        { "password", required_argument, 0,  'P' },
        { "cafile",   required_argument, 0,   0  },
        { "cert",     required_argument, 0,   0  },
        { "key",      required_argument, 0,   0  },
        { "daemon",   no_argument,       0,  'D' },
        { 0,          0,                 0,   0  }
};

void parse_options(int argc, char* argv[], struct Arguments *args)
{
        int c;
        int i;
        while ((c = getopt_long(argc, argv, "h:p:Du:P:?", long_options, &i)) != -1) {
                switch (c) {
                case 0:
                        if (i == 0) 
                                args->port = atoi(optarg);
                        else if (i == 1)
                                args->host = optarg;
                        else if (i == 2)
                                args->username = optarg;
                        else if (i == 3)
                                args->password = optarg;
                        else if (i == 4)
                                args->ca_file = optarg;
                        else if (i == 5)
                                args->cert_file = optarg;
                        else if (i == 6)
                                args->key_file = optarg;
                        else if (i == 7)
                                args->is_daemon = 1;
                        break;
                case 'h':
                        args->host = optarg;
                        break;
                case 'p':
                        args->port = atoi(optarg);
                        break;
                case 'D':
                        args->is_daemon = 1;
                        break;
                case 'u':
                        args->username = optarg;
                        break;
                case 'P':
                        args->password = optarg;
                        break;
                case '?':
                        printf("%s", args_description);
                        exit(0);
                }
        }
}

int parse_arguments(int argc, char* argv[], unsigned *is_daemon, struct Connection *connection)
{
        struct Arguments args = { 0 };
        args.is_daemon = 0;
        args.port = -1;
        args.host = NULL;
        args.username = NULL;
        args.password = NULL;
        args.ca_file = NULL;
        args.cert_file = NULL;
        args.key_file = NULL;

        parse_options(argc, argv, &args);
        
        if (args.host != NULL)
                strncpy(connection->host, args.host, HOST_LEN);
        else
                strncpy(connection->host, "localhost", HOST_LEN);

        if (args.ca_file != NULL) {
                int len = strlen(args.ca_file);
                connection->ca_file = (char *)malloc(len + 1);
                if (connection->ca_file != NULL) {
                        strncpy(connection->ca_file, args.ca_file, len);
                        connection->ca_file[len] = '\0';
                }
        }
        if (args.cert_file != NULL) {
                int len = strlen(args.cert_file);
                connection->cert_file = (char *)malloc(len + 1);
                if (connection->cert_file != NULL) {
                        strncpy(connection->cert_file, args.cert_file, len);
                        connection->cert_file[len] = '\0';
                }
        }
        if (args.key_file != NULL) {
                int len = strlen(args.key_file);
                connection->key_file = (char *)malloc(len + 1);
                if (connection->key_file != NULL) {
                        strncpy(connection->key_file, args.key_file, len);
                        connection->key_file[len] = '\0';
                }
        }

        if (args.port != -1) {
              connection->port = args.port;
        } else {
                if (args.ca_file == NULL)
                        connection->port = DEFAULT_PORT;
                else
                        connection->port = TLS_CONN_PORT;
        }   

        if (args.username != NULL) {
                int len = strlen(args.username);
                connection->username = (char *)malloc(len + 1);
                if (connection->username != NULL) {
                        strncpy(connection->username, args.username, len);
                        connection->username[len] = '\0';
                }
        }
        if (args.password != NULL) {
                int len = strlen(args.password);
                connection->password = (char *)malloc(len + 1);
                if (connection->password != NULL) {
                        strncpy(connection->password, args.password, len);
                        connection->password[len] = '\0';
                }
        }

        if (args.is_daemon)
                *is_daemon = 1;
        else
                *is_daemon = 0;

        return 0;
}