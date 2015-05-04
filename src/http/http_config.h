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

#ifndef __HTTP_CONFIG_H
#define __HTTP_CONFIG_H

#include "common.h"
#include "json_parser.h"

/* TODO: We should make all of them const */
typedef struct http_config_s {
    char*          output_directory;
    char*          url;
    char*          url_path;
    char*          http_version;
    uint32_t       total_connections;
    uint32_t       concurrency;
    uint32_t       connections_per_second;
    uint32_t       total_requests;
    uint32_t       requests_per_second;
    uint32_t       requests_per_connections;
    uint8_t        is_https;
    uint8_t        pipeline_requests;
    uint8_t        divide_requests_equally;
} http_config_t;

int http_config_init(cJSON *root);
void http_config_cleanup();
http_config_t* http_config_get();
void http_config_dump();
int http_config_strict_validate(char *message);

#endif /* __HTTP_CONFIG_H */
