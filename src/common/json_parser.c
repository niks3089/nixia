/*
*    Copyright (C) 2015 Nikhil AP 
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "json_parser.h"

char*
json_parser_parse_file(char *file)
{
	FILE *f; long len; char *data;
	
	f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
	data = (char*) malloc (len + 1);
    if (!data) {
        return NULL;
    }
    fread(data, 1, len, f);
    fclose(f);
    return data;
}

void
json_parser_get_element_str(cJSON *root, char *table_name, char *element, char **val, 
        char *val_default) 
{
    cJSON *child = NULL, *json = NULL;

    child = cJSON_GetObjectItem(root, table_name); 
    json = cJSON_GetObjectItem(child, element);

    if (json && strlen(json->valuestring)) {
        *val = strdup(json->valuestring);
    } else if (val_default) {
        *val = strdup(val_default);
    }
}

void
json_parser_get_element_double(cJSON *root, char *table_name, char *element, double *val, 
        double val_default) 
{
    cJSON *child = NULL, *json = NULL;

    child = cJSON_GetObjectItem(root, table_name); 
    json = cJSON_GetObjectItem(child, element);

    if (json) {
        *val = json->valuedouble;
    } else {
        *val = val_default; 
    }
}

void
json_parser_get_array_str(cJSON *root, char *table_name,
        char *array_name, char **val)
{
    cJSON *child = NULL;
    int i = 0;
    char tmp[MAX_MSG_LENGTH] = {0};
    char *array_item;

    child = cJSON_GetObjectItem(root, table_name); 
    cJSON *array = cJSON_GetObjectItem(child, array_name);

    for (i = 0; i < cJSON_GetArraySize(array); i++) {
        array_item = cJSON_GetArrayItem(array, i)->valuestring;
        strcat(tmp, array_item);
    }
    *val = strdup(tmp);
}
