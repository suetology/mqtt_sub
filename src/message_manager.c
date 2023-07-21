#include "message_manager.h"

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <curl/curl.h>

#define SMTP_SERVER "smtps://smtp.gmail.com:465"

static FILE *storage = NULL;
static char email_buffer[2048];
static const char *DEFAULT_STORAGE = "/usr/mqtt_sub_messages";

struct upload_status {
    size_t bytes_read;
};

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
        return 0;
    }

    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;
    size_t room = size * nmemb;

    data = &email_buffer[upload_ctx->bytes_read];

    if (data) {
        size_t len = strlen(data);
        if (room < len)
            len = room;
        memcpy(ptr, data, len);
        upload_ctx->bytes_read += len;

        return len;
    }

    return 0;
}

void consruct_email(struct Event *event)
{
        sprintf(email_buffer, "From: %s\r\n"
                        "Subject: MQTT Subscriber event\r\n"
                        "\r\n"
                        "%s\r\n", 
                        event->email, event->message);
}

int message_manager_init(const char *storage_path)
{
        if (storage_path == NULL)
                storage_path = DEFAULT_STORAGE;
        
        storage = fopen(storage_path, "a");
        if (storage == NULL) {
                syslog(LOG_ERR, "Failed to open file %s", storage_path);
                return 1;
        }

        return 0;
}

int message_manager_store_msg(const char *topic, const char *payload)
{
        if (storage == NULL) {
                syslog(LOG_ERR, "Trying to store a message in unitialized file");
                return 1;
        }

        fprintf(storage, "> Topic: \"%s\" Message: \"%s\"\n", topic, payload);
        fflush(storage);
        return 0;
}

void message_manager_terminate()
{
        if (storage != NULL)
                fclose(storage);        
}

int message_manager_send_email(struct Event *event)
{
        struct curl_slist *recipients = NULL;
        struct upload_status upload_ctx = { 0 };        

        CURL *curl = curl_easy_init();
        if (curl == NULL) {
                syslog(LOG_ERR, "Failed initializing curl");
                return CURLE_FAILED_INIT;
        }

        consruct_email(event);
        for (int i = 0; i < event->recipients_count; i++)
                recipients = curl_slist_append(recipients, event->recipients[i]);
        
        curl_easy_setopt(curl, CURLOPT_USERNAME, event->email);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, event->email_password);
        curl_easy_setopt(curl, CURLOPT_URL, SMTP_SERVER);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, event->email);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
                syslog(LOG_ERR, "Failed sending email: %s\n", curl_easy_strerror(res));

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);

        return (int)res;
}