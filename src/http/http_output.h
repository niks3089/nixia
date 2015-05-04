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

#include<stdint.h>

#include "http_worker.h"
#include "http_stats.h"
#include "http_connection.h"
void http_output_worker_summary_to_file(http_stats_base_t *stats, FILE *fp);
char* http_output_worker_summary_to_string(http_stats_base_t *stats);

void  http_output_connection_summary_to_file(struct http_worker_base_t *base);
char* http_output_connection_summary_to_string(http_connection_pool_t *pool);
void http_output_test_config_to_string(char **output_str);
