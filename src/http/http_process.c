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

#include <stdint.h>
#include <string.h>

#include "protocol.h"
#include "common.h"
#include "log.h"

#include "http_defaults.h"
#include "http_process.h"
#include "http_engine.h"
#include "http_config.h"

int
http_process_get_worker_threads(http_config_t *config) 
{
    return 1;
}

void
http_process_stop()
{
    /* Stop the threads */
    /* Collect stats and information */
    http_engine_stop();
}

void
http_process_shutdown()
{
    /* Release the resources */
    zlog_info(log_get_cat_http(), "Shutting down HTTP");
    http_engine_shutdown();
    http_config_cleanup();
}

void
http_process_start(char *directory)
{
    int workers, error;
    http_config_t *config = http_config_get();

    if (!config || !directory) {
	return;
    }

    config->output_directory = directory;

    workers = http_process_get_worker_threads(http_config_get());

    /* Start our engine */
    if ((error = http_engine_start(workers))) {
        /* TODO: This exit will not work if other protocols are running */
        //common_gui_exit_with_error("Failed to start HTTP engine");
    }
}

void
http_process_display_output()
{
    http_engine_display_output();
}

void
http_process_test_completed()
{
    protocol_announce_test_is_completed(PROTO_HTTP);
}
