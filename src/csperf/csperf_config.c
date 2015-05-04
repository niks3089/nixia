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
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "csperf_config.h"
#include "csperf_defaults.h"
#include "csperf_network.h"
#include "log.h"
#include "common.h"

static csperf_config_t *g_csperf_config = NULL;

csperf_config_t*
csperf_config_get()
{
    return g_csperf_config;
}

void
csperf_config_dump()
{
    zlog_info(log_get_cat(), "-----------------------------");
    zlog_info(log_get_cat(), "CSperf Configuration");
    zlog_info(log_get_cat(), "-----------------------------");
    zlog_info(log_get_cat(), "Role: %d", g_csperf_config->role);
    zlog_info(log_get_cat(), "Server ip: %s", g_csperf_config->server_ip);
    zlog_info(log_get_cat(), "Server port: %u", g_csperf_config->server_port);
    zlog_info(log_get_cat(), "Server echo: %d", g_csperf_config->server_echo);
    zlog_info(log_get_cat(), "Data block size: %u", g_csperf_config->data_block_size);
    zlog_info(log_get_cat(), "Total data blocks: %u", g_csperf_config->total_data_blocks);
    zlog_info(log_get_cat(), "Repeat count: %d", g_csperf_config->repeat_count);
    zlog_info(log_get_cat(), "--------------------");
}

void
csperf_config_cleanup(csperf_config_t *config)
{
    if (!config) {
        return;
    }
    if (config->server_ip) {
        free(config->server_ip);
    }

    if (config->output_directory) {
        free(config->output_directory);
    }

    if (config->client_output_file) {
        free(config->client_output_file);
    }
    if (config->server_output_file) {
        free(config->server_output_file);
    }
    free(config);
    config = NULL;
}

int
csperf_config_init(cJSON *root)
{
    double new_val = 0;
    char *cs_type;

    if (!root) {
        zlog_error(log_get_cat(), "No json to init config");
        return -1;
    }

    if (!g_csperf_config) {
        g_csperf_config = (csperf_config_t *) calloc (1, sizeof(csperf_config_t));

        if (!g_csperf_config) {
            return -1;
        }
    } else {
        memset(g_csperf_config, 0, sizeof(csperf_config_t));
    }

    /* Get configuration from json config */
    json_parser_get_element_str(root, "csperf", "cs_type",
            &cs_type, NULL);

    if (strcmp(cs_type, "client") == 0) {
        g_csperf_config->role = CS_CLIENT;
    } else {
        g_csperf_config->role = CS_SERVER;
    }
    free(cs_type);
    json_parser_get_element_str(root, "csperf", "server_ip",
            &g_csperf_config->server_ip, NULL);

    json_parser_get_element_double(root, "csperf", "server_port",
            &new_val, CSPERF_DEFAULT_SERVER_PORT); 
    g_csperf_config->server_port = new_val;

    json_parser_get_element_double(root, "csperf", "data_block_size",
            &new_val, CSPERF_DEFAULT_DATA_BLOCKLEN); 
    g_csperf_config->data_block_size = new_val;

    json_parser_get_element_double(root, "csperf", "num_blocks",
            &new_val, CSPERF_DEFAULT_DATA_BLOCKS); 
    g_csperf_config->total_data_blocks = new_val;

    json_parser_get_element_double(root, "csperf", "repeat_count",
            &new_val, 1); 
    g_csperf_config->repeat_count = new_val;

    json_parser_get_element_double(root, "csperf", "server_echo",
            &new_val, 0); 
    g_csperf_config->server_echo = new_val;
    csperf_config_dump();
    zlog_info(log_get_cat(), "Initialized CSperf configuration\n");
    return 0;
}
