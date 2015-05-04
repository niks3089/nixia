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
#include <assert.h>
#include <sys/time.h>
#include <errno.h>
#include <evhtp.h> 

#include "http_worker.h"
#include "http_connection.h"
#include "http_engine.h"
#include "log.h"

/* state names */
static struct {
    char *state_name;
} http_connection_fsm_state_name_t[] = {
    [HTTP_CONN_STATE_UNITITIALIZED] =  { "unintialized" } ,
    [HTTP_CONN_STATE_INIT] = { "init" } ,
    [HTTP_CONN_STATE_RUNNING] = { "running" } ,
    [HTTP_CONN_STATE_CLOSED] = { "closed" },
    [HTTP_CONN_STATE_ERROR] = { "error" },
};

/* Stats functions */
void http_stats_update_for_connection(http_connection_t *conn);

typedef void (*http_connection_fsm_state_handler) (http_connection_t *conn);
static http_connection_fsm_state_handler 
    http_connection_fsm_state_handlers[HTTP_CONN_MAX_STATES];

void http_connection_fsm_state_uninitialized(http_connection_t *conn);
void http_connection_fsm_state_init(http_connection_t *conn);
void http_connection_fsm_state_running(http_connection_t *conn);
void http_connection_fsm_state_re_running(http_connection_t *conn);
void http_connection_fsm_state_closed(http_connection_t *conn);
void http_connection_fsm_state_error(http_connection_t *conn);
void http_connection_fsm_state_unrecoverable_error(http_connection_t *conn);
char* http_connection_fsm_state_name(http_connection_t *conn);


char*
http_connection_fsm_state_name(http_connection_t *conn)
{
    if (conn->state < HTTP_CONN_MAX_STATES) {
        return http_connection_fsm_state_name_t[conn->state].state_name;
    }
    return NULL;
}

static void
http_connection_fsm_set_state(http_connection_t *conn,
        http_connection_fsm_state state)
{
    zlog_info(log_get_cat_http(), "Conn(%u): Changing state from %s to %s",
            conn->conn_id, 
            http_connection_fsm_state_name_t[conn->state].state_name,
            http_connection_fsm_state_name_t[state].state_name);

    conn->state = state;
}

static void
http_connection_fsm_state_change(http_connection_t *conn, 
        http_connection_fsm_state state)
{
    if (conn->state != state && conn->state < state) {
        http_connection_fsm_set_state(conn, state);
        http_connection_fsm_process(conn);
    }
}

void
http_connection_fsm_close(http_connection_t *conn) 
{
    if (!conn) {
        return;
    }
    http_connection_fsm_state_change(conn, HTTP_CONN_STATE_CLOSED);
}

void
http_connection_fsm_close_all(struct http_worker_base_t *base)
{
    http_connection_pool_t *pool = base->connection_pool;
    uint32_t total_conn = base->config->total_connections;
    int i;

    for (i = 0; i < total_conn; i++) {
        http_connection_fsm_state_change(&pool->connection_table[i], 
                HTTP_CONN_STATE_CLOSED);
    }
}

void
http_connection_fsm_process_all(struct http_worker_base_t *base)
{
    http_connection_pool_t *pool = base->connection_pool;
    uint32_t total_conn = base->config->total_connections;
    int i;

    for (i = 0; i < total_conn; i++) {
        http_connection_fsm_process(&pool->connection_table[i]);
    }
}

void
http_connection_fsm_process(http_connection_t *conn)
{
    if (!conn) {
        /* TODO: Add a log Crash for now */
    }

    http_connection_fsm_state_handlers[conn->state](conn);
}

/* EVHTP request is done. This is a callback when that happens */
static evhtp_res
http_connection_fsm_request_completed_cb(evhtp_request_t *req, void *arg)
{
    http_connection_t *conn = (http_connection_t *)arg;

    conn->ev_req = NULL;
    zlog_info(log_get_cat_http(), " %s: conn id: %u status: %u, error: %d\n",
            __FUNCTION__, conn->conn_id, req->status, req->error);
    return EVHTP_RES_OK;
}

void
http_connection_fsm_req_error_cb(evhtp_request_t *req,
        evhtp_error_flags errtype, void *arg)
{
    http_connection_t *conn = (http_connection_t *)arg;
    http_stats_update_request_error(conn);
}

static evhtp_res 
http_connection_fsm_error_cb(evhtp_connection_t *connection,
        evhtp_error_flags errtype, void *arg)
{
    http_connection_t *conn = (http_connection_t *)arg;
    zlog_info(log_get_cat_http(), " %s: Conn(%u): error_no %d %s evhtp: %d\n",
            __FUNCTION__, conn->conn_id, 
            errno, strerror(errno), errtype);

    if (pi_dll_queued(&conn->active_link)) {
        pi_dll_unlink(&conn->active_link);
    }
    if (errno) {
        conn->error = errno; 
        conn->error_msg = strdup(strerror(errno));
    }
    conn->ev_conn = NULL;

    /* Run the FSM for this connection */
    http_connection_fsm_process(conn);

    /* TODO: Bug here. IF the httpd service is stopped,
     * we can never exit */
    if (http_worker_is_test_completed(conn->base)) {
        http_worker_stop_test(conn->base);
    }
    return EVHTP_RES_OK;
}


/* EVHTP connection is done. This is a callback when that happens */
static evhtp_res
http_connection_fsm_conn_completed_cb(evhtp_connection_t *connection, void *arg)
{
    http_connection_t *conn = (http_connection_t *)arg;
    zlog_info(log_get_cat_http(), " %s: Conn(%u): error_no %d %s\n",
            __FUNCTION__, conn->conn_id, 
            errno, strerror(errno));

    if (pi_dll_queued(&conn->active_link)) {
        pi_dll_unlink(&conn->active_link);
    }
    conn->ev_conn = NULL;

    /* Run the FSM for this connection */
    http_connection_fsm_process(conn);
    return EVHTP_RES_OK;
}


/* Just set the state to INIT */
void
http_connection_fsm_state_uninitialized(http_connection_t *conn)
{
    http_connection_fsm_set_state(conn, HTTP_CONN_STATE_INIT);
}

/* Setup EVHTP connection object which attempts to connect to the server.
 * Setup EVHTP request object and send a request */
void
http_connection_fsm_state_init(http_connection_t *conn)
{
    conn->base->connection_pool->total_connections_running++;
    conn->base->stats.total_running_connections++;
    http_stats_start_time(&conn->stats.conn_run_time);

    /* Setup connection */
    pi_dll_init(&conn->active_link);
    pi_dll_insert_tail(&conn->base->connection_pool->active_list, 
            &conn->active_link);

    conn->ev_conn = evhtp_connection_new(conn->base->evbase, 
            conn->base->config->url, 80);

    if (!conn->ev_conn) {
        conn->error = HTTP_CONN_ERR_EVHTP;
        http_connection_fsm_state_change(conn, HTTP_CONN_STATE_ERROR);
        return;
    }

    /* Setup request for this connection */
    conn->ev_req = 
        evhtp_request_new(http_connection_check_completed_transfer, conn);

    if (!conn->ev_req) {
        conn->error = HTTP_CONN_ERR_EVHTP;
        http_connection_fsm_state_change(conn, HTTP_CONN_STATE_ERROR);
        return;
    }

    evhtp_set_hook(&conn->ev_conn->hooks, evhtp_hook_on_connection_fini, 
            http_connection_fsm_conn_completed_cb, conn); 
    evhtp_set_hook(&conn->ev_req->hooks, evhtp_hook_on_request_fini, 
            http_connection_fsm_request_completed_cb, conn); 
    evhtp_set_hook(&conn->ev_conn->hooks, evhtp_hook_on_conn_error,
        (evhtp_hook)http_connection_fsm_error_cb, conn);
    evhtp_set_hook(&conn->ev_req->hooks, evhtp_hook_on_error,
        (evhtp_hook)http_connection_fsm_req_error_cb, conn);
    evhtp_headers_add_header(conn->ev_req->headers_out,
                evhtp_header_new("Host", "ieatfood.net", 0, 0));
    evhtp_headers_add_header(conn->ev_req->headers_out,
                evhtp_header_new("User-Agent", "libevhtp", 0, 0));

    /* Send the request */
    http_connection_send_request(conn);
    http_connection_fsm_set_state(conn, HTTP_CONN_STATE_RUNNING);
}

/* Invoked by FSM processing under http_connection_check_completed_transfer()
 * Process the Response and update the stats. 
 *      IF done with the connection, go to CLOSE
 *      IF something is wrong, go to ERROR.
 *      IF need to run more requests, send requests and stay in this state */
void
http_connection_fsm_state_running(http_connection_t *conn)
{
    /* Check if there was any error in the transfer */
    if (conn->error) {
        http_connection_fsm_state_change(conn, HTTP_CONN_STATE_ERROR);
        return;
    }
    /* Bunch of checks to determine if this connection is done */

    /* Check if Test is completed. */
    if (http_worker_is_test_completed(conn->base)) {
        zlog_info(log_get_cat_http(), "Conn(%u): Test completed. closing conn", 
                conn->conn_id);
        http_connection_fsm_state_change(conn, HTTP_CONN_STATE_CLOSED);
        return;
    }

    /* Check if EVHTP conn is closed. */
    if (!conn->ev_conn) {
        zlog_warn(log_get_cat_http(), "Conn(%u): EVHTP conn is NULL! Closing conn", 
                conn->conn_id);
        http_connection_fsm_state_change(conn, HTTP_CONN_STATE_CLOSED);
        return;
    }

    /* Check if we are done sending max requests for this conn */
    if (conn->base->config->requests_per_connections &&
            conn->stats.total_responses_received >=
            conn->base->config->requests_per_connections) {
        /* Don't send anymore. */
        zlog_info(log_get_cat_http(), "Conn(%u): Sent max requests for this conn",
                conn->conn_id);
        http_connection_fsm_state_change(conn, HTTP_CONN_STATE_CLOSED);
        return;
    } 

    /* Check if we have sent max_requests */
    if (conn->base->stats.total_requests_sent >=
            conn->base->config->total_requests) {
        /* Don't send anymore. */
        zlog_info(log_get_cat_http(), "Conn(%u): We  have sent out max requests. "
            " Total sent: %u, Total completed: %u, Sent: %u, Received: %u",
            conn->conn_id, conn->stats.total_requests_sent, conn->base->stats.total_responses_received, 
            conn->base->stats.total_requests_sent, conn->stats.total_responses_received);
        if (conn->stats.total_requests_sent == 
                conn->base->stats.total_responses_received) {
            http_connection_fsm_state_change(conn, HTTP_CONN_STATE_CLOSED);
        }
        return;
    } 

    /* Check if we are done sending max requests for this conn */
    if (conn->base->config->requests_per_connections &&
            conn->stats.total_responses_received >=
            conn->base->config->requests_per_connections) {
        /* Don't send anymore. */
        zlog_info(log_get_cat_http(), "Conn(%u): Sent max requests for this conn",
                conn->conn_id);
        http_connection_fsm_state_change(conn, HTTP_CONN_STATE_CLOSED);
        return;
    } 
    
    /* Check if we have already sent max requests in this second.
     * IF we have, do nothing. Per second timer will invoke 
     * http_connection_re_run() which will send the request next second */
    if ((conn->base->config->requests_per_second) &&
                (conn->base->config->requests_per_second <=
                conn->base->stats.requests_sent_in_a_second)) {
        return; 
    } 

    if (conn->ev_req) {
        http_connection_send_request(conn);
    } else {
        zlog_warn(log_get_cat_http(), "Conn(%u): EVHTP Req is NULL! Go to Error", 
                conn->conn_id);
        http_connection_fsm_state_change(conn, HTTP_CONN_STATE_ERROR);
    }
}

/* Close the TCP connection and remove the conn from active list */
void
http_connection_fsm_state_closed(http_connection_t *conn)
{
    http_connection_pool_t *pool;
    
    /* We have already closed once */
    if (conn->done) {
        return;
    }
    pool = conn->base->connection_pool;

    /* Below line is a Hack! evhtp_connection_free() results in setting owner to 0 which
     * results in double free. Doing this will have libevent free the  connection
     * for us. */
    if (conn->ev_conn) {
        conn->ev_conn->owner = 0;
        /* evhtp_connection_free(conn->ev_conn); */
    }
    
    if (pi_dll_queued(&conn->active_link)) {
        pi_dll_unlink(&conn->active_link);
    }
    conn->ev_conn = NULL;

    pool->total_connections_running--;
    http_stats_update_base_after_conn_completion(conn);
    http_engine_update_connection_pool(conn->base->connection_pool,
                    pthread_self());
    http_connection_dump_stats(conn);
#if 0
    if (conn->error_msg) {
        free(conn->error_msg);
        conn->error_msg = NULL;
    }
#endif
    conn->done = 1;
}

void
http_connection_fsm_state_error(http_connection_t *conn)
{
    http_stats_update_connection_error(conn);
    http_connection_fsm_state_change(conn, HTTP_CONN_STATE_CLOSED);
}

void
http_connection_fsm_init()
{
    http_connection_fsm_state_handlers[HTTP_CONN_STATE_UNITITIALIZED] = 
        http_connection_fsm_state_uninitialized;

    http_connection_fsm_state_handlers[HTTP_CONN_STATE_INIT] = 
        http_connection_fsm_state_init;

    http_connection_fsm_state_handlers[HTTP_CONN_STATE_RUNNING] = 
        http_connection_fsm_state_running;

    http_connection_fsm_state_handlers[HTTP_CONN_STATE_CLOSED] = 
        http_connection_fsm_state_closed;

    http_connection_fsm_state_handlers[HTTP_CONN_STATE_ERROR] = 
        http_connection_fsm_state_error;
}
