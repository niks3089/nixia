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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "http_output.h"
#include "http_worker.h"
#include "http_connection.h"
#include "common.h"
#include "log.h"
#include "cJSON.h"
#include "main_config.h"

char*
http_output_worker_summary(http_stats_base_t *stats)
{

    cJSON *root, *connection_summary, *request_summary, 
          *transfer_summary, *header;
    root = cJSON_CreateObject();

    if (!main_config_is_cli_mode()) {
        cJSON_AddItemToObject(root, "header", 
                header = cJSON_CreateObject());
        cJSON_AddStringToObject(header, "protocol", "http"); 
    }

    cJSON_AddItemToObject(root, "connection summary", 
            connection_summary = cJSON_CreateObject());
    cJSON_AddItemToObject(root, "request summary", 
            request_summary = cJSON_CreateObject());
    cJSON_AddItemToObject(root, "transfer summary", 
            transfer_summary = cJSON_CreateObject());

    cJSON_AddNumberToObject(connection_summary, "Total Running connections", 
            stats->total_running_connections);
    cJSON_AddNumberToObject(connection_summary, "Total completed connections",
            stats->total_completed_connections);
    cJSON_AddNumberToObject(connection_summary, "Total successful connects",
            stats->total_successful_connects);
    cJSON_AddNumberToObject(connection_summary, "Total failed connects",
            stats->total_failed_connects);

    cJSON_AddNumberToObject(request_summary, "Total requests sent",
            stats->total_requests_sent);
    cJSON_AddNumberToObject(request_summary, "Total responses received",
            stats->total_responses_received);
    cJSON_AddNumberToObject(request_summary, "Total errors",
            stats->total_errors);
    cJSON_AddNumberToObject(request_summary, "Total 1xx responses",
            stats->total_http_code_1xx);
    cJSON_AddNumberToObject(request_summary, "Total 2xx responses",
            stats->total_http_code_2xx);
    cJSON_AddNumberToObject(request_summary, "Total 3xx responses",
            stats->total_http_code_3xx);
    cJSON_AddNumberToObject(request_summary, "Total 4xx responses",
            stats->total_http_code_4xx);
    cJSON_AddNumberToObject(request_summary, "Total 5xx responses",
            stats->total_http_code_5xx);

    cJSON_AddNumberToObject(transfer_summary, "Total run time",
            stats->total_run_time);
    cJSON_AddNumberToObject(transfer_summary, "Total uploaded bytes",
            stats->total_uploaded_bytes);
    cJSON_AddNumberToObject(transfer_summary, "Total downloaded bytes",
            stats->total_downloaded_bytes);
    cJSON_AddNumberToObject(transfer_summary, "Average transfer time",
            stats->average_transfer_time);
    cJSON_AddNumberToObject(transfer_summary, "Average requests sent in a second",
            stats->average_requests_sent_in_a_second);
    cJSON_AddNumberToObject(transfer_summary, "Average requests received in a second",
            stats->average_requests_sent_in_a_second);
    cJSON_AddNumberToObject(transfer_summary, "95 percentile of Total transfer time",
            stats->request_percentile_95);

    zlog_info(log_get_cat_http(), "Json dump is :%s", cJSON_Print(root));
    return cJSON_Print(root); 
}

char *
http_output_connection_summary(http_connection_pool_t *pool)
{
    http_connection_t         *conn;
    int                       i;

    cJSON *root, *connection_summary;
    root = cJSON_CreateObject();

    if (!pool) {
        return NULL;
    }

    for(i = 0; i < pool->total_connections; i++) {
        conn = &pool->connection_table[i];
        cJSON_AddItemToObject(root, "connection detail", 
                connection_summary = cJSON_CreateObject());
        cJSON_AddNumberToObject(connection_summary, "Connection id",
                conn->conn_id);
        cJSON_AddStringToObject(connection_summary, "Connection state",
                http_connection_fsm_state_name(conn));
        cJSON_AddNumberToObject(connection_summary, "Requests sent",
                conn->stats.total_requests_sent);
        cJSON_AddNumberToObject(connection_summary, "Responses received",
                conn->stats.total_responses_received);
        cJSON_AddNumberToObject(connection_summary, "Run time",
                conn->stats.total_connection_time);
    }
    return cJSON_Print(root); 
}

#if 0
void
http_output_test_config(char **writer, int32_t *allocated_mem, 
        int32_t *mem_left, myprintf_t myprintf)
{
    http_config_t *config = http_config_get();

    http_output_write(writer, allocated_mem, mem_left, myprintf,
            "                     Test Config                  \n");

    http_output_write(writer, allocated_mem, mem_left, myprintf,
            "--------------------------------------------------\n");

    http_output_write(writer, allocated_mem, mem_left, myprintf,
            "Total connections: %u\n", config->total_connections);
    http_output_write(writer, allocated_mem, mem_left, myprintf,
            "Concurrency: %u\n", config->concurrency);
    http_output_write(writer, allocated_mem, mem_left, myprintf,
            "Connections per second: %u\n", config->connections_per_second);

    http_output_write(writer, allocated_mem, mem_left, myprintf,
            "Total Requests: %u\n", config->total_requests);
    http_output_write(writer, allocated_mem, mem_left, myprintf,
            "Requests per second: %u\n", config->requests_per_second);
    http_output_write(writer, allocated_mem, mem_left, myprintf,
            "Pipeline requests: %s\n", (config->pipeline_requests) ?
            "YES" : "NO");
    http_output_write(writer, allocated_mem, mem_left, myprintf,
            "Divide requests: %s\n", (config->divide_requests_equally) ?
            "YES" : "NO");
}
#endif

void
http_output_worker_summary_to_file(http_stats_base_t *stats, FILE *fp)
{
    char *output;
    output = http_output_worker_summary(stats);
    fprintf(fp, output);
}

char*
http_output_worker_summary_to_string(http_stats_base_t *stats)
{
    return http_output_worker_summary(stats);
}

void
http_output_connection_summary_to_file(struct http_worker_base_t *base)
{
    char *output;
    output = http_output_connection_summary(base->connection_pool);
    fprintf(base->connection_summary_file, output);
}

char*
http_output_connection_summary_to_string(http_connection_pool_t *pool)
{
    return http_output_connection_summary(pool);
}

void
http_output_test_config_to_string(char **output_str)
{
#if 0
    int32_t allocated_mem, mem_left;

    *output_str = (char *) calloc (1, MAX_MSG_LENGTH);

    if (!*output_str) {
        return;
    }

    allocated_mem = mem_left = MAX_MSG_LENGTH;

    http_output_test_config(output_str,
            &allocated_mem, &mem_left, http_output_write_to_string);
#endif
}
