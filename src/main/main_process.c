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
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <ncurses/ncurses.h>

#include "log.h"
#include "common.h"

#include "main_process.h"
#include "main_config.h"
#include "protocol.h"

main_process_output_t *g_main_output_process = NULL;
static int g_main_running = 0;

main_process_output_t*
main_process_get_output_process()
{
    return g_main_output_process;
}

void
main_process_setup_protocol_directory(const char* protocol, char **directory)
{
    main_config_t *config = main_config_get();
    char dir_name[MAX_NAME_LENGTH] = {0};
    struct stat st = {0};
    int error = 0;

	if (!protocol || !config || !directory) {
		return;
	}

	if ((strlen(config->test_name) + strlen(protocol)) >= 
		MAX_NAME_LENGTH) {
		return;
	}

    /* TODO: Fix these kind of bugs that can lead to buffer overflow */
    strncpy(dir_name, config->test_name, strlen(config->test_name));
    strcat(dir_name, "/");
    strncat(dir_name, protocol, strlen(protocol));

    if (stat(dir_name, &st) == -1) { 
        zlog_info(log_get_cat(), "Creating directory: %s", dir_name);
        error = mkdir(dir_name, 0777);
    }

    if (error) {
        zlog_error(log_get_cat(), "Failed to create "
                "sub directories: %s err: %d",
                dir_name, error);
        //common_gui_exit_with_error("Failed to create sub directories");
    }

    *directory = strdup(dir_name);
}

void
main_process_set_up_test_directory()
{
    int error = 0;
    main_config_t *config = main_config_get();
    struct stat st = {0};
    char config_dir_name[MAX_NAME_LENGTH] = {0};

    if (!config) {
	zlog_error(log_get_cat(), "Failed to set up directory");
	return;
    }

    if (stat(config->test_name, &st) == -1) { 
        error = mkdir(config->test_name, 0777);
    }

    if (error) {
        //common_gui_exit_with_error("Failed to create directories");
    }

    strncpy(config_dir_name, config->test_name, strlen(config->test_name));
    strcat(config_dir_name, "/config");

    if (stat(config_dir_name, &st) == -1) { 
        zlog_info(log_get_cat(), "Creating directory: %s", config_dir_name);
        error = mkdir(config_dir_name, 0777);
    }

    if (error) {
        //common_gui_exit_with_error("Failed to config directory");
    }

    config->config_directory = strdup(config_dir_name);

    zlog_info(log_get_cat(), "Created directory: %s", config->config_directory);
}

void
main_process_output_stuff()
{
    main_config_t *config = main_config_get();
    char filename[MAX_NAME_LENGTH] = {0};

    if (!config || !config->config_directory) {
        return;
    }

    strncpy(filename, config->config_directory, 
            strlen(config->config_directory));
    strcat(filename, "/config.xml");

    //xml_writer_write_to_file(filename, config->config_xml);
}

int
main_process_connect_with_redis()
{
    main_config_t          *config = main_config_get();
    main_process_output_t  *output = main_process_get_output_process();

    if (!config->redis_channel || !config->redis_server ||
            !config->redis_port) {
        zlog_error(log_get_cat(), "Do not have the config to run GUI mode");
        return -1;
    }

    /* This is a blocking connect on purpose... */
    zlog_error(log_get_cat(), "Connecting to redis on %s:%u", 
            config->redis_server, config->redis_port);
    output->redis_ctx = redisConnect(config->redis_server, config->redis_port);

    if (!output->redis_ctx || output->redis_ctx->err) {
        if (output->redis_ctx) {
            zlog_warn(log_get_cat(), "Connection error: %s\n", 
                    output->redis_ctx->errstr);
        } else {
           zlog_error(log_get_cat(), "Connection error: "
                   "can't allocate redis context\n");
        }
        return -1;
    }

    /* Connected successfuly */
    zlog_info(log_get_cat(), "Connected successfully to redis "
            "server, %s:%u", config->redis_server, config->redis_port);
    return 0;
}

void
main_process_start_test()
{
    zlog_info(log_get_cat(), "Starting test");

    main_process_set_up_test_directory();
    protocol_start_test();
}

void
main_process_stop_test()
{
    zlog_info(log_get_cat(), "Stopping test");

    /* TODO: Save files and stuff here */
    main_process_output_stuff();

    protocol_stop_test();
}

void
main_process_output_cleanup()
{
    if (g_main_output_process) {
        zlog_info(log_get_cat(), "Stopping maint thread");
        event_base_loopbreak(g_main_output_process->evbase);
        event_base_free(g_main_output_process->evbase);
        if (g_main_output_process->redis_ctx) {
            redisFree(g_main_output_process->redis_ctx);
        }
        free(g_main_output_process);
    }
}

void
main_process_shutdown()
{
    int cli_mode = 0;
    zlog_info(log_get_cat(), "Shutting down");

    if (main_config_is_cli_mode()) {
        cli_mode = 1;
        printw("\nTests completed. Results in '%s' directory\n", 
                main_config_get()->test_name);
    } 
    zlog_info(log_get_cat(), "Tests completed. Results in '%s' directory\n", 
            main_config_get()->test_name);

    main_process_stop_test();

    protocol_shutdown();

    /* Other cleanups */
    main_config_cleanup();

    /* Do proper cleanup */
    main_process_output_cleanup();

    zlog_info(log_get_cat(), "Bye bye");
    zlog_fini();
    if (cli_mode) {
        printw("Press any character to exit\n");
        getch();
        endwin();
    }
}

void
main_process_sigint_handler(int signum)
{
    zlog_info(log_get_cat(), "Got sigint");
    if (main_config_is_cli_mode()) {
        endwin();
        fprintf(stdout, "Stopping test..Please wait\n");
        fflush(stdout);
    }
    g_main_running = 0;
}

void
main_process_wait_for_completion()
{
    pthread_join(g_main_output_process->maint_thread, NULL);
    zlog_info(log_get_cat_http(), "Mainteance thread stopped");
}

static int
main_process_output_timer_update()
{
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    evtimer_add(g_main_output_process->output_timer_event, &timeout);

    return 0;
}

static void
main_process_run_output_thread(int fd, short kind, void *userp)
{
    if (g_main_running) {
        zlog_info(log_get_cat(), "Running main thread");

        /* Send Alive command */
        if (!main_config_is_cli_mode() && main_process_get_output_process()
                && main_process_get_output_process()->redis_ctx) {
            zlog_info(log_get_cat(), "Sending Alive command");
            redisCommand(main_process_get_output_process()->redis_ctx,
                "PUBLISH %s %s", 
                main_config_get()->redis_channel, "KEEPALIVE"); 
        }

        protocol_display_output();
        main_process_output_timer_update();
    } else {
        if (!main_config_is_cli_mode() && main_process_get_output_process()
                && main_process_get_output_process()->redis_ctx) {
            zlog_info(log_get_cat(), "Sending Completed command");
            redisCommand(main_process_get_output_process()->redis_ctx,
                "PUBLISH %s %s", 
                main_config_get()->redis_channel, "DONE"); 
        }
        zlog_info(log_get_cat(), "Shutting down");
        main_process_shutdown();
    }
}

void *
main_process_output_thread(void *nothing)
{
    main_process_run_output_thread(0, 0, NULL);
    event_base_dispatch(g_main_output_process->evbase);
    return NULL;
}

/* Called when all the tests are done */
void
main_process_tests_completed()
{
    g_main_running = 0;
}

int
main_process_start_output()
{
    int error = 0;

    g_main_output_process = (main_process_output_t *) calloc (1, sizeof(
                main_process_output_t)); 

    if (!g_main_output_process) {
        return -1;
    }

    if (!main_config_is_cli_mode()) {
        error = main_process_connect_with_redis(); 
        if (error) {
            return -1;
        }
    }

    g_main_output_process->evbase = event_base_new();

    evthread_use_pthreads();
    if (evthread_make_base_notifiable(g_main_output_process->evbase) < 0) {
        //common_gui_exit_with_error("Failed to start output thread");
    }

    g_main_running = 1;
    g_main_output_process->output_timer_event = evtimer_new(
            g_main_output_process->evbase, 
            main_process_run_output_thread, NULL);

    error = pthread_create(&g_main_output_process->maint_thread, NULL,
               main_process_output_thread, NULL); 
    return error;
}

void
main_process_display_help()
{

    fprintf(stderr, "Usage: nixia -f <configuration file name> \n");
    fprintf (stderr, "Note, to run your test, "
            "create your configuration.json file.\n");
    fprintf (stderr, "\n");
    exit(1);
}

void
main_process_config_user_input_cleanup(main_config_user_input_t *user_input)
{
    if (!user_input) {
        return;
    }

    if (user_input->config_file) {
        free(user_input->config_file);
    }
    free(user_input);
}

main_config_user_input_t *
main_process_parse_command_line(int argc, char** argv)
{
    int rget_opt = 0;
    main_config_user_input_t *user_input = NULL;

    if (argc < 2) {
        main_process_display_help();
        /* Not needed */
        return NULL;
    }

    if (!(user_input = (main_config_user_input_t *) calloc (1 ,
                    sizeof(main_config_user_input_t)))) {
        return NULL;
    }

    while((rget_opt = getopt(argc, argv, "f:g")) != -1) {
        switch (rget_opt) {
        case 'f':
            if (optarg) {
                user_input->config_file = strdup(optarg);
            } else {
                main_process_config_user_input_cleanup(user_input);
                main_process_display_help();
            }
            break;
        case 'g':
            user_input->gui_mode = 1;
            break;
        default:
            main_process_config_user_input_cleanup(user_input);
            main_process_display_help();
        }
    }

    if (!user_input->config_file) {
        main_process_config_user_input_cleanup(user_input);
        main_process_display_help();
    }

    return user_input;
}

int
main_process_init(int mode, char *json_data)
{
    int error = 0;

    /* Initialize main configuration */
    if ((error = main_config_init(mode, json_data))) {
        return error;
    }

    /* Initialize protocol table and protocol configuration */
    if ((error = protocol_table_init(main_config_get()->protocols, 
                    json_data))) {
        /* Unrecoverable error. Cleanup everything */
        /* TODO: Cleanup since  this will exit by leaking memory */
        g_main_running = 0;
        return -1;
    }

    return 0;
}
