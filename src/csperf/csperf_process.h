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

#ifndef __CSPERF_PROCESS_H
#define __CSPERF_PROCESS_H

#include <pthread.h>
#include <event2/event.h>

typedef struct csperf_process_s {
    FILE                  *test_summary_file;
    struct event_base     *evbase;
    struct event          *activity_timer_event;
    pthread_mutex_t       lock;
    pthread_t             activity_thread;
    //csperf_stats_base_t   total_worker_stats;
} csperf_process_t;

void csperf_process_start(char *directory);
void csperf_process_display_output();
void csperf_process_stop();
void csperf_process_shutdown();
#endif /* __CSPERF_PROCESS_H */
