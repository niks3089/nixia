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

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <string.h>
#include <signal.h>
#include <event2/thread.h>

#include "log.h"
#include "protocol.h"
#include "main_config.h"

#include "http_defaults.h"
#include "http_engine.h"
#include "http_worker.h"
#include "http_process.h"
#include "http_output.h"

extern int pthread_tryjoin_np (pthread_t __th, void **__thread_return) __THROW;
static http_engine_t *g_http_engine = NULL;;

/* TODO: Spilt the job */
http_worker_test_config_t *
http_engine_init_worker_config(int workers, char *directory)
{
    http_config_t *global_config = http_config_get();
    http_worker_test_config_t* config;

    config = (http_worker_test_config_t *) calloc(1, sizeof(
                http_worker_test_config_t));

    if (!config) {
        return NULL;
    }

    /* First..Copy global config to worker config */
    memcpy(config, global_config, sizeof(http_worker_test_config_t));

    /* Change some values and divide the work */
    config->output_directory = directory;
    config->total_connections = (global_config->total_connections) / workers;
    config->concurrency = (global_config->concurrency) / workers;
    config->connections_per_second = (global_config->connections_per_second)
        / workers;
    config->total_requests = (global_config->total_requests) / workers;
    config->requests_per_second = (global_config->requests_per_second)
        / workers;
    config->requests_per_connections = (global_config->requests_per_connections)
        / workers;

    return config;
}

void
http_engine_display_current_worker_status(http_stats_base_t *stats)
{
    protocol_helper_display_result(
            http_output_worker_summary_to_string(stats));
}

void
http_engine_display_connection_summary(http_connection_pool_t *pool)
{
    //protocol_helper_display_result(
    //    http_output_connection_summary_to_string(pool));
}

void
http_engine_display_current_status()
{
    int i;

    if (!g_http_engine) {
        return;
    }

    for (i = 0; i < g_http_engine->total_workers; i++) {
        /* Check health of each worker */
        if (g_http_engine->workers[i].worker) {
            http_engine_display_current_worker_status(
                    &(g_http_engine->workers[i].running_stats));

            if (g_http_engine->workers[i].connection_pool) {
                http_engine_display_connection_summary(
                        g_http_engine->workers[i].connection_pool);

                /* Update only once */
                g_http_engine->workers[i].connection_pool = NULL;
            }
        }
    }
}

void
http_engine_display_test_config()
{
#if 0
    http_config_t *config = http_config_get();
    char *output_str;

    http_output_test_config_to_string(&output_str);

    gtk_text_buffer_set_text(
        config->widgets->output_widgets->http_out_test_config,
        output_str, strlen(output_str));

    free(output_str);
#endif
}

/* Called every second */
void
http_engine_display_output()
{
    http_engine_display_current_status();
}

void
http_engine_setup_user_event(struct event *event, pthread_t id)
{
    int i;

    pthread_mutex_lock(&g_http_engine->lock);
    for (i = 0; i < g_http_engine->total_workers; i++) {
        if (g_http_engine->workers[i].worker == id) {
            g_http_engine->workers[i].user_event = event;
        }
    }
    pthread_mutex_unlock(&g_http_engine->lock);
}

void
http_engine_remove_user_event(struct event *event, pthread_t id)
{
    int i;

    pthread_mutex_lock(&g_http_engine->lock);
    for (i = 0; i < g_http_engine->total_workers; i++) {
        if (g_http_engine->workers[i].worker == id) {
            g_http_engine->workers[i].user_event = NULL;
        }
    }
    pthread_mutex_unlock(&g_http_engine->lock);
}

void
http_engine_update_worker_stats(http_stats_base_t stats, pthread_t id)
{
    int i;

    pthread_mutex_lock(&g_http_engine->lock);
    for (i = 0; i < g_http_engine->total_workers; i++) {
        if (g_http_engine->workers[i].worker == id) {
            g_http_engine->workers[i].running_stats = stats;
        }
    }
    pthread_mutex_unlock(&g_http_engine->lock);
}

void
http_engine_update_connection_pool(http_connection_pool_t *pool, pthread_t id)
{
    int i;

    pthread_mutex_lock(&g_http_engine->lock);
    for (i = 0; i < g_http_engine->total_workers; i++) {
        if (g_http_engine->workers[i].worker == id) {
            g_http_engine->workers[i].connection_pool = pool;
        }
    }
    pthread_mutex_unlock(&g_http_engine->lock);
}

void
http_engine_copy_connection_pool(http_connection_pool_t *pool, uint32_t size, pthread_t id)
{
    int i;

    pthread_mutex_lock(&g_http_engine->lock);
    for (i = 0; i < g_http_engine->total_workers; i++) {
        if (g_http_engine->workers[i].worker == id) {
            /* TODO: Free this memory */
            g_http_engine->workers[i].connection_pool =
                    (http_connection_pool_t *) malloc(size);
            memcpy(g_http_engine->workers[i].connection_pool, pool,
                    size);
        }
    }
    pthread_mutex_unlock(&g_http_engine->lock);
}

void
http_engine_wait_for_completion()
{
    int i;

    for (i = 0; i < g_http_engine->total_workers; i++) {
        if (g_http_engine->workers[i].worker) {
            pthread_join(g_http_engine->workers[i].worker, NULL);
            zlog_info(log_get_cat_http(), "Http worker %d terminated normally", i);
        }
    }

    pthread_mutex_lock(&g_http_engine->lock);
    /* TODO: It would be faster to stop activity thread using
     * another user event */
    g_http_engine->workers_running = 0;
    pthread_mutex_unlock(&g_http_engine->lock);

    pthread_join(g_http_engine->activity_thread, NULL);

    zlog_info(log_get_cat_http(), "Http activity thread terminated normally");
}

void
http_engine_save_test_results()
{
    static int done = 0;
    int i = 0;

    if (done) {
        return;
    }

    /* Collect stats from workers */
    for (i = 0; i < g_http_engine->total_workers; i++) {
        if (g_http_engine->workers[i].worker) {
            http_stats_update_engine_stats(&g_http_engine->total_worker_stats,
                    &(g_http_engine->workers[i].running_stats),
                    g_http_engine->total_workers);
        }
    }
    http_output_worker_summary_to_file(&g_http_engine->total_worker_stats,
            g_http_engine->test_summary_file);
    fflush(g_http_engine->test_summary_file);
    done = 1;
}


void
http_engine_stop()
{
    int i;

    if (!g_http_engine) {
        return;
    }

    zlog_info(log_get_cat_http(), "Stopping HTTP engine");

    /* Stop all the workers */
    pthread_mutex_lock(&g_http_engine->lock);

    if (g_http_engine->workers_running) {
        for (i = 0; i < g_http_engine->total_workers; i++) {
            if(g_http_engine->workers[i].worker &&
                    g_http_engine->workers[i].user_event) {
                event_active(g_http_engine->workers[i].user_event,
                        EV_READ|EV_WRITE, 1);
            }
        }
    }
    pthread_mutex_unlock(&g_http_engine->lock);

    /* Wait here for the threads to stop */
    http_engine_wait_for_completion();

    /* Save the test results */
    http_engine_save_test_results();

}

http_stats_base_t *
http_engine_worker_stats()
{
    if (!g_http_engine) {
        return NULL;
    }
    return &g_http_engine->total_worker_stats;
}

void
http_engine_workers_cleanup()
{
}

void
http_engine_cleanup()
{
    if (!g_http_engine) {
        return;
    }
    event_free(g_http_engine->activity_timer_event);
    event_base_free(g_http_engine->evbase);
    http_engine_workers_cleanup();

    if (g_http_engine->test_summary_file) {
        fclose(g_http_engine->test_summary_file);
    }

    free(g_http_engine);
    g_http_engine = NULL;
}

void
http_engine_shutdown()
{
    if (!g_http_engine) {
        return;
    }

    http_engine_stop();
    http_engine_cleanup();

    zlog_info(log_get_cat_http(), "HTTP Shutdown complete");
}

static int
http_engine_activity_timer_update()
{
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    evtimer_add(g_http_engine->activity_timer_event, &timeout);

    return 0;
}

/* Do the following:
 * Non-blocking pthread_condwait
 * If completed, return.
 * Check for a global variable to see if the test is supposed to be stopped.
 * If it is, trigger event_active on all the threads, do a blocking
 * pthread_join till the threads are completed.
 * Else, reset the timer and return */

static void
http_engine_check_activity(int fd, short kind, void *userp)
{
    int i, error = 0;

    if (!g_http_engine->workers_running) {
        if (evtimer_pending(g_http_engine->activity_timer_event, NULL)) {
            evtimer_del(g_http_engine->activity_timer_event);
        }
        return;
    }

    for (i = 0; i < g_http_engine->total_workers; i++) {
        if (g_http_engine->workers[i].worker) {
            error |= pthread_tryjoin_np(g_http_engine->workers[i].worker, NULL);
        }
    }

    if (!error) {
        /* Workers are done */

        pthread_mutex_lock(&g_http_engine->lock);
        g_http_engine->workers_running = 0;
        pthread_mutex_unlock(&g_http_engine->lock);

        if (evtimer_pending(g_http_engine->activity_timer_event, NULL)) {
            evtimer_del(g_http_engine->activity_timer_event);
        }

        zlog_info(log_get_cat_http(), "Http workers terminated normally");
        http_process_test_completed();
        return;
    }

    http_engine_activity_timer_update();
}

static void*
http_engine_activity_thread(void *nothing)
{
    http_engine_check_activity(0, 0, NULL);

    /* Run till all the workers are done */
    event_base_dispatch(g_http_engine->evbase);
    return NULL;
}

int
http_engine_setup_output_files()
{
    http_config_t *config = http_config_get();
    main_config_t *main_config = main_config_get();

    if (!main_config) {
        return 0;
    }

    /* Set up file to write the total test summary */
    if (!(g_http_engine->test_summary_file = protocol_create_file(
            config->output_directory, HTTP_DEFAULT_TEST_SUMMARY_FILE))) {
        zlog_warn(log_get_cat_http(), "Failed to create test summary file");
        return -1;
    }
    return 0;
}

char *
http_engine_setup_worker_directories()
{
    static int worker_id = 0;

    char dir_name[MAX_NAME_LENGTH] = {0};
    char worker_id_str[10] = {0};
    struct stat st = {0};
    int error = 0;
    http_config_t *global_config = http_config_get();

    sprintf(worker_id_str, "%d", ++worker_id);

    /* Setup directory name */
    strncpy(dir_name, global_config->output_directory,
            strlen(global_config->output_directory));
    strcat(dir_name, "/worker_");
    strncat(dir_name, worker_id_str, strlen(worker_id_str));

    if (stat(dir_name, &st) == -1) {
        zlog_info(log_get_cat(), "Creating directory: %s", dir_name);
        error = mkdir(dir_name, 0777);
    }

    if (error) {
        return NULL;
    }
    return(strdup(dir_name));
}

int
http_engine_start(int workers)
{
    int i, error;
    http_worker_test_config_t *config;
    http_config_t *global_config = http_config_get();
    char *directory = NULL;

    /* Check if we are engine is already set up. If it is,
     * restart the test */

    if (!g_http_engine) {
        g_http_engine = (http_engine_t *) calloc (1, sizeof(http_engine_t));

        if (!g_http_engine) {
            return -1;
        }

        pthread_mutex_init(&g_http_engine->lock, 0);
        g_http_engine->total_workers = workers;
        g_http_engine->evbase = event_base_new();
        g_http_engine->activity_timer_event = evtimer_new(g_http_engine->evbase,
                http_engine_check_activity, NULL);

        /* Setup the output files for this test */
        if ((error = http_engine_setup_output_files(
                global_config->output_directory, workers))) {
            zlog_warn(log_get_cat_http(), "Failed to setup HTTP output files");
            return -1;
        }
        zlog_info(log_get_cat_http(), "Starting http");
    }
    g_http_engine->workers_running = 1;

    /* Start the workers */
    for (i = 0; i < workers; i++) {
        directory = http_engine_setup_worker_directories();

        if (!directory) {
            /* TODO: How do we stop the already running worker? */
            http_engine_cleanup();
            return -1;
        }

        /* Set up the configuration for the workers to work with */
        if (!(config = http_engine_init_worker_config(workers, directory))) {
            http_engine_cleanup();
            return -1;
        }

        /* Start the worker and feed in the config */
        error = pthread_create(&(g_http_engine->workers[i].worker), NULL,
                http_worker_thread, config);

        if (error) {
            zlog_error(log_get_cat_http(), "%s: Couldn't run http worker thread %d, "
                    " error %d", __func__, i, errno);
            http_engine_cleanup();
            return -1;
        } else {
            zlog_info(log_get_cat_http(), "Started http worker %d", i);
        }
    }

    /* Set up an activity thread that monitors the workers and
     * writes output from them to gtk */
    error = pthread_create(&(g_http_engine->activity_thread), NULL,
                http_engine_activity_thread, NULL);

    if (error) {
        /* TODO: Stop the threads */
        http_engine_cleanup();
        return -1;
    }

    zlog_info(log_get_cat_http(), "Starting HTTP Engine");

    //thread_openssl_cleanup();
    return 0;
}
