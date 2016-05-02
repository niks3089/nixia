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

#ifndef __HTTP_WORKER_H
#define __HTTP_WORKER_H

#include <event2/event.h>

#include "http_connection.h" 
#include "http_config.h" 

/* TODO: This is okay but make sure we divide the work */
typedef http_config_t http_worker_test_config_t; 

/* Per thread configuration */
struct http_worker_base_t {
    struct event_base         *evbase;
    struct event              *per_second_timer_event;
    struct event              *user_event;
    http_worker_test_config_t *config;
    http_connection_pool_t    *connection_pool;
    FILE                      *worker_summary_file;
    FILE                      *connection_summary_file;

    uint32_t                  worker_id;
    int                       running_connections;
    char                      statistics[MAX_NAME_LENGTH];

    /* Stats per thread */
    http_stats_base_t         stats; 
};

void* http_worker_thread(void *data);
void http_worker_stop_test_if_completed(struct http_worker_base_t *base);
int http_worker_is_test_completed(struct http_worker_base_t *base);
void http_worker_stop_test(struct http_worker_base_t *base);
int http_worker_finished_sending_requests(struct http_worker_base_t *base);
#endif /* __HTTP_WORKER_H */
