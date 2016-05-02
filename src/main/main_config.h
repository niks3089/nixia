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

#ifndef __MAIN_CONFIG_H
#define __MAIN_CONFIG_H

#include "common.h"

typedef struct main_config_user_input_s {
    char       *config_file;
    int        gui_mode;
} main_config_user_input_t; 

typedef struct main_config_s {
    char           *test_name;
    char           *redis_channel;
    char           *redis_server;
    char           *interface;
    char           *protocols;
    char           *config_directory;
    char           *json_config;
    uint16_t       redis_port;
    uint16_t       total_run_time;
    uint16_t       repeat_count;
    uint8_t        cli_mode;
} main_config_t;

main_config_t* main_config_get();
int main_config_init(int mode, char *json_data);
void main_config_cleanup();
int main_config_is_cli_mode();

#endif /* __MAIN_CONFIG_H */
