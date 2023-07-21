#include "json_module.h"

#include <string.h>

#include "cJSON.h"

int extract_data_number(int *buf, char *json_string, char *property_name)
{
        cJSON *root = cJSON_Parse(json_string);
        if (root == NULL)
                return -1;

        cJSON *data = cJSON_GetObjectItem(root, "data");
        if (data == NULL) {
                cJSON_Delete(root);
                return -2;
        }

        cJSON *property = cJSON_GetObjectItem(data, property_name);
        if (property == NULL) {
                cJSON_Delete(root);
                return -3;
        }

        if (property->type != cJSON_Number) {
                cJSON_Delete(root);
                return -4;
        }

        *buf = property->valueint;
        cJSON_Delete(root);
        return 0;
}

int extract_data_string(char *buf, int buf_len, char *json_string, char *property_name)
{
        cJSON *root = cJSON_Parse(json_string);
        if (root == NULL) 
                return -1;

        cJSON *data = cJSON_GetObjectItem(root, "data");
        if (data == NULL) {
                cJSON_Delete(root);
                return -2;
        }

        cJSON *property = cJSON_GetObjectItem(data, property_name);
        if (property == NULL) {
                cJSON_Delete(root);
                return -3;
        }

        if (property->type != cJSON_String || property->valuestring == NULL) {
                cJSON_Delete(root);
                return -4;
        }

        strncpy(buf, property->valuestring, buf_len);
        cJSON_Delete(root);
        return 0;
}