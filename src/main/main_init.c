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
#include <signal.h>
#include <unistd.h>
#include <hiredis.h>
#include <ncurses/ncurses.h>

#include "log.h"
#include "common.h"
#include "json_parser.h"
#include "main_process.h"
#include "main_config.h"

/* Place where magic starts.
 * Initialize modules and fire up the main UI */
void
main_init(int argc, char** argv)
{
    int error = 0;
    char *json_data = NULL;
    main_config_user_input_t *user_input = NULL;

    /* Initalize our logging module. */
    if ((error = log_init())) {
        fprintf(stderr, "Failed to init logging module. error: %d\n", error);
        exit(1);
    }
    zlog_info(log_get_cat(), "Initialized logging module");

    signal (SIGINT, main_process_sigint_handler);

    /* Parse the arguments */
    user_input = main_process_parse_command_line(argc, argv);

    if (!user_input) {
        exit(1) ;
    }

    zlog_info(log_get_cat(), "Parsing file :%s", user_input->config_file);
    json_data = json_parser_parse_file(user_input->config_file); 

    if (!json_data) {
        zlog_error(log_get_cat(), "Failed to load configuration");
        if (!user_input->gui_mode) {
            fprintf(stdout, "Failed to load configuration.\n");
        }
        exit(1);
    } else if (!cJSON_Parse(json_data)) {
        zlog_error(log_get_cat(), "Failed to validate configuration. "
                "Error: %s", cJSON_GetErrorPtr());
        if (!user_input->gui_mode) {
            fprintf(stdout, "Failed to validate configuration. Error: %s\n",
                    cJSON_GetErrorPtr());
        }
        exit(1);
    }

    /* Initial main process and protocol configuration */
    if ((error = main_process_init(!user_input->gui_mode, json_data))) {
        zlog_error(log_get_cat(), "Failed to init main process");
        main_process_config_user_input_cleanup(user_input);
        main_process_shutdown();
        exit(1);
    }

    if ((error = main_process_start_output())) {
        zlog_error(log_get_cat(), "Failed to start the maint thread");
        main_process_config_user_input_cleanup(user_input);
        main_process_shutdown();
        exit(1);
    }

    if (!user_input->gui_mode) {
        initscr();
        mvprintw(0, 0, "Running test. Please wait...\n");
        refresh();
    } 

    /* Cleanup user input. We don't need it anymore */
    main_process_config_user_input_cleanup(user_input);

    /* Start the test */
    main_process_start_test();

    /* Tests are completed */
    main_process_wait_for_completion();
    zlog_error(log_get_cat(), "Tests completed");

    /* TODO: we might need to move where program
     * actually closes */
    //zlog_fini();
}

int main(int argc, char** argv)
{
    main_init(argc, argv);
    return 0;
}
