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

#ifndef __MAIN_PROCESS_H
#define __MAIN_PROCESS_H

#include "main_config.h"
#include "hiredis.h"

typedef struct main_process_output_s {
    pthread_t             maint_thread;
    struct event_base     *evbase;
    struct event          *output_timer_event;
    redisContext          *redis_ctx;
} main_process_output_t;

void main_process_user_input();
void main_process_start_test();
int main_process_start_output();
void main_process_shutdown();
void main_process_sigint_handler(int signum);
main_process_output_t *main_process_get_output_process();
void main_process_tests_completed();
void main_process_setup_protocol_directory(const char* protocol, 
        char **directory);
void main_process_stop_test();
int main_process_init(int mode, char *json_data);
void main_process_wait_for_completion();
main_config_user_input_t 
    *main_process_parse_command_line(int argc, char** argv);
void main_process_config_user_input_cleanup(
        main_config_user_input_t *user_input);

#endif /* __MAIN_PROCESS_H */
