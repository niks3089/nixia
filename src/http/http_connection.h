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

#ifndef __HTTP_CONNECTION_H
#define __HTTP_CONNECTION_H

#include <event2/event.h>
#include "http_stats.h" 
#include "pi_dll.h"
#include <evhtp.h>

struct http_worker_base_t; 
struct http_request_handle_t;

typedef enum http_connection_state_s 
{
    HTTP_CONN_STATE_UNITITIALIZED = 0,
    HTTP_CONN_STATE_INIT,
    HTTP_CONN_STATE_RUNNING,
    HTTP_CONN_STATE_ERROR,
    HTTP_CONN_STATE_CLOSING,
    HTTP_CONN_STATE_CLOSED,

    /* Add new states above */
    HTTP_CONN_MAX_STATES,

} http_connection_fsm_state;

typedef enum http_connection_error_code_s
{
    HTTP_CONN_ERR_NO_MEM = 1,
    HTTP_CONN_ERR_EVHTP,
    HTTP_CONN_ERR_TRANSFER,

} http_connection_error_code;

typedef struct http_connection_s {

    /* Attach it to the active list when the 
     * connection is setup */
    pi_dll_t                     active_link;

    /* EvHTP Connection object */
    evhtp_connection_t           *ev_conn;

    /* EvHTP Request object */
    evhtp_request_t              *ev_req;

    /* EvHTP response status */
    evhtp_res                    status;
    struct http_worker_base_t    *base;
    uint32_t                     conn_id; 
    uint8_t                      is_https:1;
    uint8_t                      done:1;
    uint8_t                      reserved:6;
    int                          error;
    char                         *error_msg;                        
    http_connection_fsm_state    state; 
    http_stats_connection_t      stats;
} http_connection_t;

typedef struct http_connection_pool_s {
    pi_dll_t                      active_list;
    uint32_t                      total_connections;
    uint32_t                      next_connection_to_run;
    uint32_t                      total_pending_connections;
    uint32_t                      total_connections_running;
    struct http_worker_base_t     *base;
    http_connection_t             connection_table[1];
} http_connection_pool_t;

/* FSM stuff */
void http_connection_fsm_process(http_connection_t *conn);
void http_connection_fsm_init();
void http_connection_fsm_process_all(struct http_worker_base_t *base);
void http_connection_fsm_close_all(struct http_worker_base_t *base);
void http_connection_fsm_close(http_connection_t *conn);
char *http_connection_fsm_state_name(http_connection_t *conn);
void http_stats_update_connection_error(http_connection_t *conn);

/* Connection stuff */
http_connection_pool_t* http_connection_init_pool(void *config); 
void  http_connection_check_completed_transfer(evhtp_request_t *req, void *arg);
void http_connection_dump_stats(http_connection_t *conn);
void http_connection_kickstart(struct http_worker_base_t *base);
int  http_connection_run_next_connection(struct http_worker_base_t *base);
void http_connection_run_connections(struct http_worker_base_t *base, uint32_t count);
void http_stats_update_base(http_connection_t *conn);
void http_stats_update_base_after_conn_completion(
        http_connection_t *conn);
void http_connection_send_request(http_connection_t *conn);
void http_connection_pool_cleanup(struct http_worker_base_t *base,
        http_connection_pool_t *pool);
void http_connection_get_connection_summary(http_connection_pool_t *pool,
        char **output_str, int32_t *allocated_mem, int32_t *mem_left);
void http_connection_re_run(struct http_worker_base_t *base);
void http_connection_write_summary_to_file(struct http_worker_base_t *base);
void http_stats_update_for_connection(http_connection_t *conn);
void http_stats_increment_requests_sent(http_connection_t *conn);
void http_stats_increment_responses_received(http_connection_t *conn);
void http_stats_reset_per_second_stats(struct http_worker_base_t *base);
void http_stats_update_request_error(http_connection_t *conn);
void http_stats_update_base_run_time(struct http_worker_base_t *base);
void http_stats_update_response_stats(http_connection_t *conn);
void http_stats_start_time(struct timespec *now);
#endif /* __HTTP_CONNECTION_H */
