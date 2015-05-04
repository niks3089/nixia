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

#include <errno.h>
    
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <event2/thread.h>
#include <assert.h>

#include "protocol.h"

#include "http_engine.h"
#include "http_worker.h"
#include "http_defaults.h"
#include "http_stats.h"
#include "http_output.h"

#include "log.h"

void http_worker_base_cleanup(struct http_worker_base_t *base);

void
http_worker_dump_base_stats(struct http_worker_base_t *base)
{
    zlog_info(log_get_cat_http(), "Worker result");
    zlog_info(log_get_cat_http(), "---------------------");

    zlog_info(log_get_cat_http(), "total_running_connections: %u", base->stats.total_running_connections);
    zlog_info(log_get_cat_http(), "total_connections completed: %u", base->stats.total_completed_connections);
    zlog_info(log_get_cat_http(), "total_requests sent: %u", base->stats.total_requests_sent);
    zlog_info(log_get_cat_http(), "total_responses_received: %u", base->stats.total_responses_received);
    zlog_info(log_get_cat_http(), "total_errors: %u", base->stats.total_errors);
    zlog_info(log_get_cat_http(), "total_successful_connects: %u", base->stats.total_successful_connects);
    zlog_info(log_get_cat_http(), "total_failed_connects: %u", base->stats.total_failed_connects);
    zlog_info(log_get_cat_http(), "total_failed_connections: %u", base->stats.total_failed_connections);
    zlog_info(log_get_cat_http(), "total_run_time: %f ms", base->stats.total_run_time);
    zlog_info(log_get_cat_http(), "total_uploaded_bytes: %u", base->stats.total_uploaded_bytes);
    zlog_info(log_get_cat_http(), "total_downloaded_bytes: %u", base->stats.total_downloaded_bytes);
    zlog_info(log_get_cat_http(), "total_http_code_1xx: %u", base->stats.total_http_code_1xx);
    zlog_info(log_get_cat_http(), "total_http_code_2xx: %u", base->stats.total_http_code_2xx);
    zlog_info(log_get_cat_http(), "total_http_code_3xx: %u", base->stats.total_http_code_3xx);
    zlog_info(log_get_cat_http(), "total_http_code_4xx: %u", base->stats.total_http_code_4xx);
    zlog_info(log_get_cat_http(), "total_http_code_5xx: %u", base->stats.total_http_code_5xx);

    zlog_info(log_get_cat_http(), "average_transfer_time: %f", base->stats.average_transfer_time);
    zlog_info(log_get_cat_http(), "average_upload_speed: %f", base->stats.average_upload_speed);
    zlog_info(log_get_cat_http(), "average_download_speed: %f", base->stats.average_download_speed);
    zlog_info(log_get_cat_http(), "average_requests_sent_in_a_second: %f", base->stats.average_requests_sent_in_a_second);
    zlog_info(log_get_cat_http(), "average_responses_received_in_a_second: %f", base->stats.average_responses_received_in_a_second);
}

int
http_worker_finished_sending_requests(struct http_worker_base_t *base)
{
    return ((base->stats.total_requests_sent >= base->config->total_requests));
}

int
http_worker_is_test_completed(struct http_worker_base_t *base)
{
    return (((base->stats.total_responses_received >= base->config->total_requests))
        || (base->stats.total_completed_connections >= base->config->total_connections));
}

void
http_worker_save_test_result(struct http_worker_base_t *base)
{
    http_output_worker_summary_to_file(&base->stats, base->worker_summary_file);
    http_output_connection_summary_to_file(base);
}

void
http_worker_update_final_stats(struct http_worker_base_t *base)
{
    http_stats_calculate_request_percentile(base);

    /* Let the engine know about the changes done */
    http_engine_update_worker_stats(base->stats, pthread_self());
}

void
http_worker_stop_test(struct http_worker_base_t *base)
{
    if (evtimer_pending(base->per_second_timer_event, NULL)) {
        evtimer_del(base->per_second_timer_event);
        zlog_info(log_get_cat_http(), "Deleting second timer");
    }

    zlog_info(log_get_cat_http(), "Stopping test");
    http_stats_update_base_run_time(base);
    event_base_loopbreak(base->evbase);
    /* TODO: Close all the evhtp connections */
    http_connection_fsm_close_all(base);

    /* Calcluate final stats */
    http_worker_update_final_stats(base);

    /* Copy the connection pool */
    http_engine_copy_connection_pool(base->connection_pool, 
        sizeof(http_connection_pool_t) + 
        sizeof(http_connection_t) * base->connection_pool->total_connections,
        pthread_self());

    /* Save the output to the files */
    http_worker_save_test_result(base);
}

void
http_worker_stop_test_if_completed(struct http_worker_base_t *base)
{
    if (http_worker_is_test_completed(base)) {
        http_worker_stop_test(base);
    }
}

void
http_connection_run_requests_per_second(struct http_worker_base_t *base)
{
    static int cur_second = 0;
    if (base->config->requests_per_second > 
            base->stats.requests_sent_in_a_second) {
        zlog_warn(log_get_cat_http(), "Could not send the necessary "
                "amount requests in one second."
                "Expected to be sent: %u, Sent: %u",
                base->config->requests_per_second, 
                base->stats.requests_sent_in_a_second);
    }

    if (base->config->requests_per_second) {
        zlog_info(log_get_cat_http(), "Request summary in a second:"
                "Expected to be sent: %u, Sent: %u"
                " Received: %u",
                base->config->requests_per_second, 
                base->stats.requests_sent_in_a_second,
                base->stats.responses_received_in_a_second);

        zlog_info(log_get_cat_http(), "Running rps");
    }
    http_stats_calculate_average(
        &base->stats.average_requests_sent_in_a_second,
        base->stats.requests_sent_in_a_second, ++cur_second);

    http_stats_calculate_average(
        &base->stats.average_responses_received_in_a_second,
        base->stats.responses_received_in_a_second, ++cur_second);

    http_stats_reset_per_second_stats(base);

    /* Start the transfers for the next second */
    if (base->config->requests_per_second) {
        http_connection_re_run(base);
    }
}

static int
http_worker_per_second_timer_update(struct http_worker_base_t *base)
{
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    evtimer_add(base->per_second_timer_event, &timeout);

    return 0;
}

static void 
http_worker_per_second_timer_callback(int fd, short kind, void *userp)
{
    struct http_worker_base_t *base = (struct http_worker_base_t *)userp;

    if (base->config->connections_per_second) {
        http_connection_run_connections(base, 
                base->config->connections_per_second);
    }

    http_connection_run_requests_per_second(base);
    http_worker_per_second_timer_update(base);
}

static void 
http_worker_user_event_handler(evutil_socket_t fd, short events, void *user_data)
{
    struct http_worker_base_t *base;

    base = (struct http_worker_base_t *)user_data;
    zlog_info(log_get_cat_http(), "Got a user triggered event");
    http_worker_stop_test(base);
}

static struct http_worker_base_t*
http_worker_init_base(http_worker_test_config_t *config)
{
    struct http_worker_base_t *base;

    base = (struct http_worker_base_t*) malloc(sizeof(struct http_worker_base_t));

    if (!base) {
        return NULL;
    }
    memset(base, 0, sizeof(struct http_worker_base_t));
    base->evbase = event_base_new();

    evthread_use_pthreads();

    if (evthread_make_base_notifiable(base->evbase) < 0) {
        free(base);
        return NULL;
    }

    base->per_second_timer_event = evtimer_new(base->evbase, 
            http_worker_per_second_timer_callback, base);
    base->user_event = event_new(base->evbase, -1, EV_READ|EV_WRITE,
            http_worker_user_event_handler, base);
    event_add(base->user_event, NULL);

    http_engine_setup_user_event(base->user_event, pthread_self());

    base->config = config;

    /* Set up requests stats */
    base->stats.requests_stat_table = 
        http_stats_init_request_table(config->total_requests);

    if (!(base->stats.requests_stat_table)) {
        zlog_warn(log_get_cat_http(), "Failed to setup request stats table");
        http_worker_base_cleanup(base);
        return NULL;
    }

    /* Set up file to write the worker test summary */
    if (!(base->worker_summary_file = protocol_create_file(
            config->output_directory, HTTP_DEFAULT_TEST_SUMMARY_FILE))) {
        zlog_warn(log_get_cat_http(), "Failed to create test summary file");
        http_worker_base_cleanup(base);
        return NULL;
    }

    /* Set up file to write the connection summary */
    if (!(base->connection_summary_file = protocol_create_file(
            config->output_directory,HTTP_DEFAULT_TEST_CONNECTION_FILE))) {
        zlog_warn(log_get_cat_http(), "Failed to create test "
                "connection summary file");
        http_worker_base_cleanup(base);
        return NULL;
    }

    return base;
}

int
http_worker_setup_events(struct http_worker_base_t *base)
{
    return 0;
}

int
http_worker_setup_timer(struct http_worker_base_t *base)
{
    /* See alloc_init_timer_waiting_queue */
    http_worker_per_second_timer_update(base);
    return 0;
}

int
http_worker_setup_timers(struct http_worker_base_t *base)
{
    http_worker_setup_timer(base);
    return 0;
}

void
http_worker_base_cleanup(struct http_worker_base_t *base)
{
    http_engine_remove_user_event(base->user_event, pthread_self());
    http_connection_pool_cleanup(base, base->connection_pool);
    event_free(base->per_second_timer_event);
    event_free(base->user_event);
    event_base_free(base->evbase);

    if (base->worker_summary_file) {
        fclose(base->worker_summary_file);
    }

    if (base->connection_summary_file) {
        fclose(base->connection_summary_file);
    }

    if (base->config->output_directory) {
        free(base->config->output_directory);
    }

    if (base->config) {
        free(base->config);
    }

    if (base->stats.requests_stat_table) {
        free(base->stats.requests_stat_table);
    }

    free(base);
    base = NULL;
}

int
http_worker_run_test(struct http_worker_base_t *base)
{
    //base->running_connections = 1;

    http_connection_kickstart(base);

    event_base_dispatch(base->evbase);
    return 0;
}

/* Worker thread. */
void*
http_worker_thread(void *data) 
{
    http_worker_test_config_t *config;
    struct http_worker_base_t *base;

    config = (http_worker_test_config_t *) data;
    zlog_error(log_get_cat_http(), " In worker thread");

    /* Init Connection fsm */
    http_connection_fsm_init();

    /* Init Base config */
    if (!(base = http_worker_init_base(config))) {
        zlog_error(log_get_cat_http(), " Failed to init http base");
        return NULL;
    }

    http_worker_setup_timers(base);

    /* Initialize the connection pool */
    if (!(base->connection_pool = http_connection_init_pool(base))) {
        zlog_error(log_get_cat_http(), "Failed to init http connection pool");
        free(base);
        return NULL;
    }

    http_stats_start_time(&base->stats.run_time);
    http_worker_run_test(base);

    zlog_info(log_get_cat_http(), "Done testing: Connections left: %d",
            base->running_connections);

    http_config_dump();
    http_worker_dump_base_stats(base);

    /* Collect stats */
    /* Cleanup the engine */
    http_worker_base_cleanup(base);
    return NULL;
}
