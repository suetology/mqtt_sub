#ifndef JSON_MODULE_H
#define JSON_MODULE_H

int extract_data_number(int *buf, char *json_string, char *property_name);
int extract_data_string(char *buf, int buf_len, char *json_string, char *property_name);

#endif