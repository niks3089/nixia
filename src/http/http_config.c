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

#include "http_config.h"
#include "http_defaults.h"
#include "log.h"
#include "common.h"

static http_config_t *g_http_config = NULL;

http_config_t*
http_config_get()
{
    return g_http_config;
}

void
http_config_dump()
{
    zlog_info(log_get_cat(), "----------------------------");
    zlog_info(log_get_cat(), "HTTP configuration:");
    zlog_info(log_get_cat(), "----------------------------");
    zlog_info(log_get_cat(), "URL: %s", g_http_config->url);
    zlog_info(log_get_cat(), "Path: %s", g_http_config->url_path);
    zlog_info(log_get_cat(), "HTTP Version: %s", g_http_config->http_version);
    zlog_info(log_get_cat(), "total_connections: %u", g_http_config->total_connections);
    zlog_info(log_get_cat(), "concurrency: %u", g_http_config->concurrency);
    zlog_info(log_get_cat(), "connections_per_second: %u", g_http_config->connections_per_second);
    zlog_info(log_get_cat(), "total_requests: %u", g_http_config->total_requests);
    zlog_info(log_get_cat(), "requests_per_second: %u", g_http_config->requests_per_second);
    zlog_info(log_get_cat(), "Pipeling enabled: %d", g_http_config->pipeline_requests);
    zlog_info(log_get_cat(), "--------------------");
}

void
http_config_complete_init()
{
    if (g_http_config->total_connections == HTTP_DEFAULT_TOTAL_CONNECTIONS) {
        if (g_http_config->concurrency != HTTP_DEFAULT_CONCURRENCY) {
            g_http_config->total_connections = g_http_config->concurrency;
        }

        if (g_http_config->connections_per_second != 
                HTTP_DEFAULT_CONNECTIONS_PER_SECOND) {
            g_http_config->total_connections = 
                g_http_config->connections_per_second;
        }
    }

    if (g_http_config->total_requests == HTTP_DEFAULT_TOTAL_REQUESTS) {
        g_http_config->divide_requests_equally = 1;
        g_http_config->total_requests = g_http_config->total_connections;
    }

    if (g_http_config->divide_requests_equally) {
        g_http_config->requests_per_connections =
            g_http_config->total_requests / g_http_config->total_connections;
    }
}

void
http_config_cleanup()
{
    zlog_error(log_get_cat(), "Cleaning up config");

    if (!g_http_config) {
        return;
    }

    if (g_http_config->output_directory) { 
        free(g_http_config->output_directory);
    }

    if (g_http_config->url) {
        free(g_http_config->url);
    }

    if (g_http_config->url_path) {
        free(g_http_config->url_path);
    }

    if (g_http_config->http_version) {
        free(g_http_config->http_version);
    }

    free(g_http_config);
    g_http_config = NULL;
}

/* Initialize the http configuration. */
int
http_config_init(cJSON *root)
{
    double new_val = 0;

    if (!root) {
        zlog_error(log_get_cat(), "No json to init config");
        return -1;
    }

    /* IF the config is already setup, don't allocate a new one */ 
    if (!g_http_config) { 
        g_http_config = (http_config_t *) calloc (1, sizeof(http_config_t));

        if (!g_http_config) {
            zlog_error(log_get_cat(), "Failed to allocate memory for http memory");
            return -1;
        }
    } else {
        memset(g_http_config, 0, sizeof(http_config_t));
    }
    /* Get the configuration from json config */

    /* Get string types */
    json_parser_get_element_str(root, "http", "url",
            &g_http_config->url, NULL);
    json_parser_get_element_str(root, "http", "url_path",
            &g_http_config->url_path, "/");
    json_parser_get_element_str(root, "http", "http_version",
            &g_http_config->http_version, HTTP_DEFAULT_HTTP_VERSION);

    /* Get integer types */
    json_parser_get_element_double(root, "http", "total_connections", 
            &new_val, HTTP_DEFAULT_TOTAL_CONNECTIONS);
    g_http_config->total_connections = new_val;

    json_parser_get_element_double(root, "http", "concurrency", 
            &new_val, HTTP_DEFAULT_CONCURRENCY);
    g_http_config->concurrency = new_val;

    json_parser_get_element_double(root, "http", "connections_per_second", 
            &new_val, HTTP_DEFAULT_CONNECTIONS_PER_SECOND);
    g_http_config->connections_per_second = new_val;

    json_parser_get_element_double(root, "http", "total_requests", 
            &new_val, HTTP_DEFAULT_TOTAL_REQUESTS);
    g_http_config->total_requests = new_val;

    json_parser_get_element_double(root, "http", "requests_per_second", 
            &new_val, HTTP_DEFAULT_REQUESTS_PER_SECOND);
    g_http_config->requests_per_second = new_val;

    json_parser_get_element_double(root, "http", "divide_requests_equally", 
            &new_val, 0);
    g_http_config->divide_requests_equally = new_val;

    http_config_complete_init();
    http_config_dump();
    zlog_info(log_get_cat(), "Initialized http configuration");
    return 0;
}
