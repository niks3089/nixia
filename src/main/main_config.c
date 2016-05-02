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

#include <string.h>
#include <stdlib.h>

#include "main_config.h"
#include "main_defaults.h"
#include "log.h"
#include "common.h"
#include "json_parser.h"

static main_config_t *g_main_config = NULL;

main_config_t*
main_config_get()
{
    return g_main_config;
}

void
main_config_cleanup()
{
    if (!g_main_config) {
        return;
    }

    if (g_main_config->json_config) {
        free(g_main_config->json_config);
    }

    if (g_main_config->interface) {
        free(g_main_config->interface);
    }

    if (g_main_config->test_name) {
        free(g_main_config->test_name);
    }
    
    if (g_main_config->redis_channel) {
        free(g_main_config->redis_channel);
    }

    if (g_main_config->redis_server) {
        free(g_main_config->redis_server);
    }

    if (g_main_config->protocols) {
        free(g_main_config->protocols);
    }

    if (g_main_config->config_directory) {
        free(g_main_config->config_directory);
    }

    free(g_main_config);
    g_main_config = NULL;
}

void
main_config_dump()
{
    zlog_info(log_get_cat(), "Main configuration:");
    zlog_info(log_get_cat(), "total_run_time: %u", g_main_config->total_run_time);
    zlog_info(log_get_cat(), "repeat_count: %u", g_main_config->repeat_count);
    zlog_info(log_get_cat(), "test name: %s", g_main_config->test_name);
    zlog_info(log_get_cat(), "protocols: %s", g_main_config->protocols);
    zlog_info(log_get_cat(), "interface: %s", g_main_config->interface);
    zlog_info(log_get_cat(), "redis channel: %s", g_main_config->redis_channel);
    zlog_info(log_get_cat(), "redis server: %s", g_main_config->redis_server);
    zlog_info(log_get_cat(), "redis port: %u", g_main_config->redis_port);
    zlog_info(log_get_cat(), "---------------------");
}

int
main_config_is_cli_mode()
{
    return g_main_config? g_main_config->cli_mode : 1;
}

/* Initialize the main configuration.
 * This will be used as FYI by other protocols */
int
main_config_init(int mode, char *json_data)
{
    double new_val = 0;
    cJSON *root;

    if (!(root = cJSON_Parse(json_data))) {
        zlog_error(log_get_cat(), "Failed to validate config: %s",
                cJSON_GetErrorPtr());
        return -1;
    }

    g_main_config = (main_config_t *) calloc (1, sizeof(main_config_t));

    if (!g_main_config) {
        zlog_error(log_get_cat(), "Failed to allocate memory for main config");
        return -1;
    }
    g_main_config->cli_mode = mode;
    g_main_config->json_config = json_data;

    /* Get the configuration from json */

    /* Get string types */
    json_parser_get_element_str(root, "main", "name",
            &g_main_config->test_name, DEFAULT_TEST_NAME);
    json_parser_get_element_str(root, "main", "redis_channel", 
            &g_main_config->redis_channel, NULL);
    json_parser_get_element_str(root, "main", "redis_server", 
            &g_main_config->redis_server, NULL);
    json_parser_get_element_str(root, "main", "interface", 
            &g_main_config->interface, NULL);

    json_parser_get_array_str(root, "main", "protocols", 
            &g_main_config->protocols);

    /* Get integer types */
    json_parser_get_element_double(root, "main", "redis_port", 
            &new_val, 0);
    g_main_config->redis_port = new_val;
    json_parser_get_element_double(root, "main", "total_run_time", 
            &new_val, MAIN_DEFAULT_TOTAL_RUN_TIME);
    g_main_config->total_run_time = new_val;
    json_parser_get_element_double(root, "main", "repeat_count", 
            &new_val, MAIN_DEFAULT_REPEAT_COUNT);
    g_main_config->repeat_count = new_val;

    main_config_dump();
    zlog_info(log_get_cat(), "Initialized main configuration");
    return 0;
}
