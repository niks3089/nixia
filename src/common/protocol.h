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

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>
#include <stdio.h>

#include "json_parser.h"

/* All the protocols needs to have functions of type shown below. */ 
typedef int  (*protocol_config_init_t) (cJSON *root);
typedef void (*protocol_start_test_t) (char *output_directory);
typedef void (*protocol_display_output_t) ();
typedef void (*protocol_stop_test_t) ();
typedef void (*protocol_shutdown_t) ();

typedef enum {
    PROTO_HTTP = 0,
    PROTO_CSPERF,

    /* Add new protocols above this */
    PROTO_MAX,
} protocols;

int protocol_table_init(char *protocol_list, char *json_data);
void protocol_load_next_protocol();

void protocol_start_test();
void protocol_stop_test();
void protocol_shutdown();
void protocol_display_output();
void protocol_announce_test_is_completed(int);
char* protocol_get_filename(char *directory, char *filename);
FILE* protocol_create_file(char *directory, char *filename);
void protocol_load_main_page();
void protocol_helper_display_result(char *output);

#endif /* __PROTOCOL_H */
