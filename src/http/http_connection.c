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
#include <assert.h>

#include "log.h"
#include "main_config.h"
#include "http_worker.h"
#include "http_stats.h"

#define ACTIVE_LINK_TO_CONN_ENT(_h) (http_connection_t *)((char*)_h - offsetof(http_connection_t, active_link))

/* Check if we have started all the connections */
int
http_connections_loaded_max_connections(struct http_worker_base_t *base)
{
    return (base->connection_pool->next_connection_to_run >=
            base->config->total_connections);
}

/* Check if we have already sent our quota of HTTP requests */
int
http_connection_should_send_next_request(struct http_worker_base_t *base)
{
    if (base->stats.total_requests_sent >= 
            base->config->total_requests) {
        zlog_info(log_get_cat_http(), "We have already sent "
                "max requests. Sent: %u, Max: %u",
                base->stats.total_requests_sent, base->config->total_requests);
        return 0;
    }
    if (base->config->requests_per_second && 
        base->stats.requests_sent_in_a_second >= 
            base->config->requests_per_second) {
        return 0;
    }
    return 1;
}

int
http_connection_should_run_next_connection(struct http_worker_base_t *base)
{
    /* No need if the test is completed */
    if (http_worker_is_test_completed(base)) {
        return 0;
    }

    /* No need to run a connection if we don't have to send
     * any requests */
    if (!http_connection_should_send_next_request(base)) {
        return 0;
    }

    /* One second timer is going to run the connections.
     * We shouldn't do it here */
    if (base->config->connections_per_second) {
        return 0;
    }

    /* We have loaded max connections */
    if (http_connections_loaded_max_connections(base)) {
        return 0;
    }

    /* No need if we are running at max */
    if (base->config->concurrency) {
        if (base->connection_pool->total_connections_running >=
                base->config->concurrency) {
            zlog_info(log_get_cat_http(), "Already running "
                    "max concurrent connections");
            return 0;
        }
    }
    return 1;
}

/* Send HTTP request */
void
http_connection_send_request(http_connection_t *conn)
{
    if (conn && conn->ev_conn && conn->ev_req) {  
        http_stats_increment_requests_sent(conn);
        http_stats_start_time(&conn->stats.request_transfer_time);
        evhtp_make_request(conn->ev_conn, conn->ev_req, htp_method_GET, 
                http_config_get()->url_path);
        zlog_info(log_get_cat_http(), "Conn(%u): Sending request. Total sent:%u"
                "  Connection sent: %u request path: %s", 
                conn->conn_id, conn->base->stats.total_requests_sent, 
                conn->stats.total_requests_sent, http_config_get()->url_path);
    } else {
        assert(0);
    }
}


/* Called when running rps */
void
http_connection_re_run(struct http_worker_base_t *base)
{
    pi_dll_t *list, *dll;
    http_connection_t *conn = NULL;
    
    list = &base->connection_pool->active_list;

    /* Check if there are any active connections */
    for(dll = list->dll_next; dll != list && dll;
            dll = dll->dll_next) { 

        if (!(http_connection_should_send_next_request(base))) {
            break;
        }
        conn = (http_connection_t *)dll;

        assert(conn && conn->ev_req && conn->ev_conn);
        http_connection_send_request(conn);
    }
    /* Run next connection if there are no active connections to run */
    if (!conn && 
        base->stats.total_requests_sent >= base->config->total_requests) {
        http_connection_run_next_connection(base);
        return;
    }
}

/* Free connection pool */
void
http_connection_pool_cleanup(struct http_worker_base_t *base, 
        http_connection_pool_t *pool)
{
    /* TODO:  Conn FSM cleanup to De-queue active links and 
     * close TCP connections */
    free(pool);
    pool = NULL;
}

/* Called when a HTTP resp is received. */
void 
http_connection_check_completed_transfer(evhtp_request_t *req, void *arg)
{
    http_connection_t *conn = (http_connection_t *)arg;
    struct http_worker_base_t *base = conn->base;

    /* Update connection stats */
    if (req) { 
        conn->status = evhtp_request_status(req);
    }
    assert(conn->state == HTTP_CONN_STATE_RUNNING);

    /* Run connection fsm here */
    http_stats_update_response_stats(conn);
    http_connection_fsm_process(conn);

    zlog_info(log_get_cat_http(),"Conn(%u): Request completed. Response status: %u "
            "total req compl: %u conn req completd: %u Total req sent:%u, "
            "Total connectons running: %u, Connections completed: %u",
            conn->conn_id, conn->status, base->stats.total_responses_received, 
            conn->stats.total_responses_received, base->stats.total_requests_sent,
            conn->base->connection_pool->total_connections_running, 
            base->stats.total_completed_connections);

    if (!http_worker_is_test_completed(base)) {
        /* Check if we need to run new connection */
        if (http_connection_should_run_next_connection(base)) {
            http_connection_run_next_connection(base);
        }
    } else {
        /* Test is completed. Collect stats */
        zlog_info(log_get_cat_http(), "Tests completed. Total req sent:%u "
                " Total req completed: %u, Connections completed: %u",
                base->stats.total_requests_sent,
                base->stats.total_responses_received, base->stats.total_completed_connections);
        http_worker_stop_test_if_completed(base);
    }
}

/* Setup our connection object */
void
http_connection_setup(http_connection_t *conn, uint32_t id,
        struct http_worker_base_t *base) 
{
    /* Setup connection */
    conn->conn_id = id;
    conn->base = base;

    /* Set to Init state */
    http_connection_fsm_process(conn);
}

/* Init the connection pool and all the connection objects */
http_connection_pool_t*
http_connection_init_pool(void *data)
{
    int i;
    http_connection_pool_t *pool;
    struct http_worker_base_t *base = (struct http_worker_base_t *)data;
    http_worker_test_config_t *config = base->config; 
    uint16_t total_connections = config->total_connections;

    pool = (http_connection_pool_t *) calloc (1, sizeof(http_connection_pool_t) + 
            (sizeof(http_connection_t) * total_connections)); 

    if (!pool) {
        return NULL;
    }

    pool->total_connections = total_connections;
    pool->total_pending_connections = 0;

    pool->base = base;
    pi_dll_init(&pool->active_list);

    for (i = 0; i < total_connections; i++) {
        http_connection_setup(&pool->connection_table[i], i, base);
    }
    return pool;
}

int
http_connection_run_next_connection(struct http_worker_base_t *base)
{
    http_connection_pool_t *pool = base->connection_pool;
    http_worker_test_config_t *config = base->config;
    http_connection_t *conn = NULL;

    if (pool->next_connection_to_run < config->total_connections) {
        conn = &pool->connection_table[pool->next_connection_to_run++];
        zlog_info(log_get_cat_http(), "Conn(%u): Loading connection",
                conn->conn_id);

        /* Connection should be in INIT state. Connect to the server
         * and begin transfer */
        http_connection_fsm_process(conn);
        return 0;
    }
    return -1;
}

void
http_connection_run_connections(struct http_worker_base_t *base, uint32_t count)
{
    uint32_t i;

    zlog_info(log_get_cat_http(), "Loading %u connections. "
            "next con to run:%u", count, 
            base->connection_pool->next_connection_to_run);

    for (i = 0; i < count; i++) {
        http_connection_run_next_connection(base);
    }
}

/* Initially, how many connections to run? */
void
http_connection_kickstart(struct http_worker_base_t *base)
{
    http_worker_test_config_t *config = base->config;

    if (config->concurrency) {
        http_connection_run_connections(base, config->concurrency);
    } else if (config->connections_per_second) {
        http_connection_run_connections(base, config->connections_per_second);
    } else if (config->requests_per_second) {
        http_connection_run_connections(base, config->requests_per_second);
    } else {
        http_connection_run_connections(base, 1);
    }
}

void
http_connection_dump_stats(http_connection_t *conn)
{
    if (!conn) {
        return;
    }

    zlog_info(log_get_cat_http(), "Connection stats for conn %u", conn->conn_id);
    zlog_info(log_get_cat_http(), "-----------------");
    zlog_info(log_get_cat_http(), "total_transfer_time: %f", conn->stats.total_transfer_time);
    zlog_info(log_get_cat_http(), "uploaded_bytes: %f", conn->stats.uploaded_bytes);
    zlog_info(log_get_cat_http(), "upload_speed: %f", conn->stats.upload_speed);
    zlog_info(log_get_cat_http(), "downloaded_bytes: %f", conn->stats.downloaded_bytes);
    zlog_info(log_get_cat_http(), "download_speed: %f", conn->stats.download_speed);
    zlog_info(log_get_cat_http(), "total_requests: %u", conn->stats.total_requests_sent);

    zlog_info(log_get_cat_http(), "--------------------------------------------");
}
