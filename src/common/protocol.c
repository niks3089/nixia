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

/*
 * Description: Conatins APIs for main and applications to communicate with
 * each other. Allows new applications to register here
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ncurses/ncurses.h>
#include <event2/event.h>

#include "common.h"
#include "protocol.h"
#include "main_process.h"
#include "log.h"
#include "json_parser.h"
#include "hiredis.h"

#include "http_process.h"
#include "http_config.h"

#include "csperf_process.h"
#include "csperf_config.h"

/* 
 * Protocol registeration table.
 * New protocols must register here.
 * protocol_name:             Name of the proto used for logging.
 * protocol_set:              Set when user clicks on the checkbox.
 * protocol_test_completed_event: Event which allows to inform main gui when the
 * test is completed.
 *                            well   
 * protocol_config_init:      Callback to initialize config
 * protocol_start_test:       Callback to start the test for that protocol
 * protocol_display_output:   Callback to display stats to GUI 
 * protocol_stop_test:        Callback to stop the tests and traffic
 * protocol_shutdown:         Callback to clean up
 */
static struct protocol_table_t {
     const char            *protocol_name;
     uint8_t               protocol_set;
     struct event          *protocol_test_completed_event;
    
     /* Callback functions */
     protocol_config_init_t    protocol_config_init;
     protocol_start_test_t     protocol_start_test;
     protocol_display_output_t protocol_display_output;
     protocol_stop_test_t      protocol_stop_test;
     protocol_shutdown_t       protocol_shutdown;
} protocol_table[] = {
    [PROTO_HTTP] = { "http", 0, NULL, 
                    http_config_init,
                    http_process_start, 
                    http_process_display_output,
                    http_process_stop, 
                    http_process_shutdown}, 

    [PROTO_CSPERF] = { "csperf", 0, NULL, 
                    csperf_config_init,
                    csperf_process_start, 
                    csperf_process_display_output,
                    csperf_process_stop, 
                    csperf_process_shutdown}, 
};

int
protocol_table_init(char *protocol_list, char *json_config)
{
    int i = 0, error = 0;
    cJSON *root = cJSON_Parse(json_config);

    /* For all the protocols in the protocol list, set protocol_set
     * in protocol table when will allow us to know what  
     * protocol to load and when */
    for (i = 0; i < PROTO_MAX; i++) {
        if(strstr(protocol_list, protocol_table[i].protocol_name) != NULL) {
            zlog_debug(log_get_cat(), "Setting protocol: %s", 
                    protocol_table[i].protocol_name);
            protocol_table[i].protocol_set = 1;

            if (protocol_table[i].protocol_config_init) { 
                if ((error = protocol_table[i].protocol_config_init(root))) {
                    return -1;
                }
            } else {
                zlog_error(log_get_cat(), "No callback to setup config");
                return -1;
            }
        }
    }
    return 0;
}

/* Called by the main activity thread every second. It calls
 * callbacks of other protocols to display stats */ 
void
protocol_display_output()
{
    int i;

    for (i = 0; i < PROTO_MAX; i++) {
        if (protocol_table[i].protocol_set) {
            if (protocol_table[i].protocol_display_output) {
                protocol_table[i].protocol_display_output();
            }
        }
    }
}

/* Inform main gui that this test is completed.
 * This will result in call to protocol_handle_test_completion() */
void
protocol_announce_test_is_completed(int protocol)
{
    if (protocol_table[protocol].protocol_set) {
        event_active(protocol_table[protocol].protocol_test_completed_event,
                EV_READ|EV_WRITE, 1);
    }
}

/* Protocol is done with its test. */
static void 
protocol_handle_test_completion(evutil_socket_t fd, short events, void *user_data)
{
    int i;
    /* Inform main about this and if all are done. 
     * Do whatever is necessary */
    struct protocol_table_t *protocol_info = 
        (struct protocol_table_t *)user_data;

    assert(protocol_info);

    if (protocol_info->protocol_set) {
        assert(protocol_info->protocol_test_completed_event);
        event_del(protocol_info->protocol_test_completed_event);
        protocol_info->protocol_test_completed_event = NULL;
    }

    /* Check if all protocols are done. If not, return */
    for (i = 0; i < PROTO_MAX; i++) {
        if (protocol_table[i].protocol_set) {
            if(protocol_table[i].protocol_test_completed_event) {
                return;
            }
        }
    }

    /* All tests are completed */
    protocol_display_output();
    main_process_tests_completed();
}

void
protocol_setup_test_completion_event(int protocol)
{
    if (protocol_table[protocol].protocol_set) {
        protocol_table[protocol].protocol_test_completed_event = 
            event_new(main_process_get_output_process()->evbase, -1, EV_READ|EV_WRITE,
                protocol_handle_test_completion, &protocol_table[protocol]);
            event_add(protocol_table[protocol].protocol_test_completed_event, NULL);
    } 
}

void
protocol_start_test()
{
    int i;
    char *directory = NULL;

    for (i = 0; i < PROTO_MAX; i++) {
        if (protocol_table[i].protocol_set) {
            if (protocol_table[i].protocol_start_test) {
                protocol_setup_test_completion_event(i);

                /* Before running test, create output directories */
                main_process_setup_protocol_directory(
                        protocol_table[i].protocol_name, &directory);

                /* Start test */
                protocol_table[i].protocol_start_test(directory);
            } else {
                zlog_error(log_get_cat(), "Oops! No start test handler for protocol: %s",
                        protocol_table[i].protocol_name);
            }
        }
    }
}

void
protocol_stop_test()
{
    int i;
	
    for (i = 0; i < PROTO_MAX; i++) {
        if (protocol_table[i].protocol_set) {
            if (protocol_table[i].protocol_stop_test) {
                protocol_table[i].protocol_stop_test();
            } else {
                //common_gui_exit_with_error("Failed to stop the test"); 
                zlog_error(log_get_cat(), "Oops! No stop_test handler for protocol: %s",
                        protocol_table[i].protocol_name);
            }
        }
    }
}

void
protocol_shutdown()
{
    int i;
	
    for (i = 0; i < PROTO_MAX; i++) {
        if (protocol_table[i].protocol_set) {
            if (protocol_table[i].protocol_shutdown) {
                protocol_table[i].protocol_shutdown();
            } else {
                zlog_error(log_get_cat(), "Oops! No start test handler for protocol: %s",
                        protocol_table[i].protocol_name);
            }
        }
    }
}

/* TODO: Move this to common.c */

/* Whoever uses this function has the responsibility 
 * to free filename */
char *
protocol_get_filename(char *directory, char *filename)
{
    char temp_name[MAX_NAME_LENGTH] = {0};

	if (!directory || !filename) {
		return NULL;
	}

	if ((strlen(directory) + strlen(filename)) >= MAX_NAME_LENGTH) {
		return NULL;
	}

    strncpy(temp_name, directory, strlen(directory));
    strcat(temp_name, "/");
    strncat(temp_name, filename, strlen(filename));

    return(strdup(temp_name));
}

FILE *
protocol_create_file(char *directory, char *filename)
{
    char *filepath;
    FILE *new_file = NULL;

	if (!directory || !filename) {
		return NULL;
	}

    if (!(filepath = protocol_get_filename(directory, filename))) {
        zlog_warn(log_get_cat_http(), "Failed to get the filepath for %s ",
                filename);
        return NULL;
    }
    new_file = fopen(filepath, "w");
    free(filepath);
    return new_file;
}

void
protocol_helper_display_result(char *output) 
{
    redisReply *reply;

    if (main_config_is_cli_mode()) {
        mvprintw(0, 0, "%s",output);
        refresh();
    } else {
        reply = redisCommand(main_process_get_output_process()->redis_ctx,
                "PUBLISH %s %s", 
                main_config_get()->redis_channel, output); 

        if (reply) {
            zlog_info(log_get_cat(), "reply command: %d:%s\n", reply->type, reply->str);
        }
    }
}
