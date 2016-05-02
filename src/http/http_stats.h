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

#ifndef __HTTP_STATS_H
#define __HTTP_STATS_H

typedef struct http_stats_connection_s {
    struct timespec request_transfer_time;
    struct timespec conn_run_time;

    uint8_t http_transfer_error: 1,
            http_internal_error: 1,
            http_code_unused : 6;

    uint32_t     http_code_1xx;
    uint32_t     http_code_2xx;
    uint32_t     http_code_3xx;
    uint32_t     http_code_4xx;
    uint32_t     http_code_5xx;

    /* The total time in seconds for the transfer */
    double        total_transfer_time;
    double        total_connection_time;

    /* Total amount of bytes that were uploaded. */
    double        uploaded_bytes;

    /* Average upload speed in bytes/second */
    double        upload_speed;

    /* Total amount of bytes that were downloaded for last transfer. */
    double        downloaded_bytes;

    /* Average upload speed in bytes/second */
    double        download_speed;

    uint32_t      total_requests_sent;
    uint32_t      total_responses_received;
} http_stats_connection_t;

typedef http_stats_connection_t http_stats_request_t;

typedef struct http_stats_request_table_s {
    uint32_t             total_requests;
    uint32_t             current_request;
    http_stats_request_t request_table[1];
} http_stats_request_table_t;

typedef struct http_stats_base_s {

    /* Time related stats */
    struct timespec run_time;
    double          total_run_time;

    /* Total stats */
    uint32_t total_running_connections;
    uint32_t total_completed_connections;
    uint32_t total_requests_sent;
    uint32_t total_responses_received;
    uint32_t total_errors;
    uint32_t total_successful_connects;
    uint32_t total_failed_connects;
    uint32_t total_failed_connections;
    uint32_t total_uploaded_bytes;
    uint32_t total_downloaded_bytes;
    uint32_t total_http_code_1xx;
    uint32_t total_http_code_2xx;
    uint32_t total_http_code_3xx;
    uint32_t total_http_code_4xx;
    uint32_t total_http_code_5xx;

    /* Average stats for successful connections */
    double average_transfer_time;
    double average_upload_speed;
    double average_download_speed;
    double average_requests_sent_in_a_second;
    double average_responses_received_in_a_second;

    /* Current stats */
    uint32_t requests_sent_in_a_second;
    uint32_t responses_received_in_a_second;

    /* Percentiles */
    double request_percentile_95;

    http_stats_request_table_t *requests_stat_table;

} http_stats_base_t;

void http_stats_calculate_average(double *curr_average,
        double new_value, uint32_t total);

http_stats_request_table_t *http_stats_init_request_table(uint32_t total_requests);
void http_stats_calculate_request_percentile(void *data);
void http_stats_update_engine_stats(http_stats_base_t *engine_stats,
        http_stats_base_t *worker_stats, int workers);
#endif /* __HTTP_STATS_H */
