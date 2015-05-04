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

#ifndef __COMMON_H
#define __COMMON_H

#include <stdint.h>

/* Do not include any of our header files here! */

#define LOG_CONFIG_FILE "/etc/nixia/log.conf"
#define LOG_CATEGORY "my_cat"
#define HTTP_LOG_CATEGORY "http_cat"
#define CSPERF_LOG_CATEGORY "csperf_cat"

/* XSD files after installation */
#define CONFIG_XSD_FILE   "/etc/nixia/config.xsd"

/* Some lengths to use. */
#define MESSAGE_ERR_LENGTH 1500 
#define MAX_MSG_LENGTH     1500
#define MAX_NAME_LENGTH    100

/* Default test name and directory */
#define DEFAULT_TEST_NAME "output_test"
#endif /* __COMMON_H */
