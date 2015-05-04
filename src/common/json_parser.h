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

#ifndef __JSON_PARSER_H
#define __JSON_PARSER_H

#include "cJSON.h"

char* json_parser_parse_file(char *file);
void json_parser_get_element_double(cJSON *root, char *table_name, char *element, double *val, 
        double val_default); 
void json_parser_get_element_str(cJSON *root, char *table_name, char *element, char **val, 
        char *val_default);
void json_parser_get_array_str(cJSON *root, char *table_name,
        char *array_name, char **val);
#endif /* __JSON_PARSER_H */
