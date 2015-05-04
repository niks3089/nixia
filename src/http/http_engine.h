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

#ifndef __HTTP_ENGINE_H
#define __HTTP_ENGINE_H

#include <pthread.h>
#include <event2/event.h>

#include "http_stats.h" 
#include "http_connection.h" 

#define MAX_WORKERS 16

typedef struct http_engine_workers_s {
    pthread_t               worker;
    struct event            *user_event;
    http_connection_pool_t  *connection_pool;
    http_stats_base_t       running_stats; 
} http_engine_workers_t;

typedef struct http_engine_s {
    FILE                  *test_summary_file;
    struct event_base     *evbase;
    struct event          *activity_timer_event;
    int                   workers_running;
    int                   total_workers;
    pthread_mutex_t       lock;
    pthread_t             activity_thread;
    http_stats_base_t     total_worker_stats;
    http_engine_workers_t workers[MAX_WORKERS];
} http_engine_t;

int http_engine_start(int workers);
void http_engine_stop();
void http_engine_shutdown();
void http_engine_setup_user_event(struct event *event, pthread_t id);
void http_engine_remove_user_event(struct event *event, pthread_t id);
void http_engine_update_worker_stats(http_stats_base_t stats, pthread_t id);
void http_engine_display_output();
void http_engine_update_connection_pool(http_connection_pool_t *pool, pthread_t id);
void http_engine_copy_connection_pool(http_connection_pool_t *pool,
        uint32_t size, pthread_t id);

http_stats_base_t* http_engine_worker_stats();
#endif /* __HTTP_ENGINE_H */
