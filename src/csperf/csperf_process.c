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

#include <pthread.h> 

#include "csperf_process.h"
#include "csperf_defaults.h"
#include "csperf_config.h"
#include "csperf_client.h"
#include "csperf_server.h"

#include "log.h" 
#include "protocol.h" 

static void*
csperf_process_worker(void *nothing)
{
    csperf_config_t *config = csperf_config_get();

    zlog_info(log_get_cat_csperf(), "Started csperf worker");

    if (config->role == CS_CLIENT) {
        csperf_client_run(config);
    } else {
        csperf_server_run(config);
    }
    protocol_announce_test_is_completed(PROTO_CSPERF);
    return NULL;
}

int
csperf_process_start_worker_thread()
{
    pthread_t worker_thread;
    int error = 0;

    error = pthread_create(&worker_thread, NULL,
                csperf_process_worker, NULL); 

    return error;
}

void
csperf_process_start(char *directory)
{
    int error;
    csperf_config_t *config = csperf_config_get();

    config->output_directory = directory;

    if ((error = csperf_process_start_worker_thread())) {
    }
}

void
csperf_process_display_output()
{
}

void
csperf_process_stop()
{
}

void
csperf_process_shutdown()
{
}
