/*
*Copyright(C)2015NikhilAP
*
*Thisprogramisfreesoftware:youcanredistributeitand/ormodify
*itunderthetermsoftheGNUGeneralPublicLicenseaspublishedby
*theFreeSoftwareFoundation,eitherversion3oftheLicense,or
*(atyouroption)anylaterversion.
*
*Thisprogramisdistributedinthehopethatitwillbeuseful,
*butWITHOUTANYWARRANTY;withouteventheimpliedwarrantyof
*MERCHANTABILITYorFITNESSFORAPARTICULARPURPOSE.Seethe
*GNUGeneralPublicLicenseformoredetails.
*
*YoushouldhavereceivedacopyoftheGNUGeneralPublicLicense
*alongwiththisprogram.Ifnot,see<http://www.gnu.org/licenses/>.
*/

#include<assert.h>

#include"http_engine.h"
#include"http_worker.h"
#include"http_connection.h"
#include"log.h"

double
http_stats_get_timediff(structtimespec*now,structtimespec*before)
{
if(!now||!before){
return0;
}
return(
(now->tv_sec-before->tv_sec)*1000LL+
(now->tv_nsec-before->tv_nsec)/1000000LL);
}

void
http_stats_update_transfer_request_time(http_connection_t*conn)
{
structtimespecnow;
if(!conn){
return;
}
clock_gettime(CLOCK_MONOTONIC,&now);
conn->stats.total_transfer_time=http_stats_get_timediff(&now,
&conn->stats.request_transfer_time);
}

void
http_stats_start_time(structtimespec*now)
{
clock_gettime(CLOCK_MONOTONIC,now);
}

void
http_stats_update_conn_run_time(http_connection_t*conn)
{
structtimespecnow;

if(!conn){
return;
}
clock_gettime(CLOCK_MONOTONIC,&now);
conn->stats.total_connection_time=
http_stats_get_timediff(&now,&conn->stats.conn_run_time);
}

void
http_stats_update_base_run_time(structhttp_worker_base_t*base)
{
structtimespecnow;

if(!base){
return;
}
clock_gettime(CLOCK_MONOTONIC,&now);
base->stats.total_run_time=
http_stats_get_timediff(&now,&base->stats.run_time);
}

void
http_stats_update_connection_code(http_connection_t*conn)
{
uint16_tresponse_status;
uint16_tresponse_module=0;

response_status=conn->status;
response_module=response_status/(long)100;

switch(response_module){
case1:
conn->stats.http_code_1xx++;
conn->base->stats.total_http_code_1xx++;
break;
case2:
conn->stats.http_code_2xx++;
conn->base->stats.total_http_code_2xx++;
break;
case3:
conn->stats.http_code_3xx++;
conn->base->stats.total_http_code_3xx++;
break;
case4:
conn->stats.http_code_4xx++;
conn->base->stats.total_http_code_4xx++;
break;
case5:
conn->stats.http_code_5xx++;
conn->base->stats.total_http_code_5xx++;
break;
default:
;
}
}

void
http_stats_calculate_average(double*curr_average,
doublenew_value,uint32_ttotal)
{
doubleaverage;

if(total==0){
return;
}

if(total==1){
*curr_average=new_value;
return;
}

average=(((*curr_average)*(total-1))+new_value)/total;

*curr_average=average;
}

void
http_stats_update_request_stats(http_stats_request_table_t*table,
doubletotal_transfer_time)
{
if(!table){
return;
}

assert(table->current_request<=table->total_requests);
table->request_table[table->current_request++].total_transfer_time
=total_transfer_time;
}

void
http_stats_update_base_after_conn_completion(http_connection_t*conn)
{
structhttp_worker_base_t*base;

if(!conn){
return;
}

http_stats_update_conn_run_time(conn);

base=conn->base;
base->stats.total_completed_connections++;
base->stats.total_running_connections--;
http_engine_update_worker_stats(base->stats,pthread_self());
}

void
http_stats_update_base(http_connection_t*conn)
{
}

void
http_stats_update_engine_stats(http_stats_base_t*engine_stats,
http_stats_base_t*worker_stats,intworkers)
{
/*Connectionstuff*/
engine_stats->total_running_connections+=
worker_stats->total_running_connections;

engine_stats->total_completed_connections+=
worker_stats->total_completed_connections;

engine_stats->total_successful_connects+=
worker_stats->total_successful_connects;

engine_stats->total_failed_connects+=
worker_stats->total_failed_connects;

/*Requeststuff*/
engine_stats->total_requests_sent+=
worker_stats->total_requests_sent;

engine_stats->total_responses_received+=
worker_stats->total_responses_received;

engine_stats->total_errors+=
worker_stats->total_errors;

/*Responsestuff*/
engine_stats->total_http_code_1xx+=
worker_stats->total_http_code_1xx;

engine_stats->total_http_code_2xx+=
worker_stats->total_http_code_2xx;

engine_stats->total_http_code_3xx+=
worker_stats->total_http_code_3xx;

engine_stats->total_http_code_4xx+=
worker_stats->total_http_code_4xx;

engine_stats->total_http_code_5xx+=
worker_stats->total_http_code_5xx;

/*Transferstuff*/
engine_stats->total_uploaded_bytes+=
worker_stats->total_uploaded_bytes;

engine_stats->total_downloaded_bytes+=
worker_stats->total_downloaded_bytes;

http_stats_calculate_average(&engine_stats->average_transfer_time,
worker_stats->average_transfer_time,workers);

http_stats_calculate_average(&engine_stats->average_download_speed,
worker_stats->average_download_speed,workers);

http_stats_calculate_average(&engine_stats->average_upload_speed,
worker_stats->average_upload_speed,workers);

http_stats_calculate_average(
&engine_stats->average_requests_sent_in_a_second,
worker_stats->average_requests_sent_in_a_second,workers);

http_stats_calculate_average(
&engine_stats->average_responses_received_in_a_second,
worker_stats->average_responses_received_in_a_second,workers);

http_stats_calculate_average(
&engine_stats->total_run_time,
worker_stats->total_run_time,workers);
/*Percentilesummary*/
http_stats_calculate_average(
&engine_stats->request_percentile_95,
worker_stats->request_percentile_95,workers);
}

http_stats_request_table_t*
http_stats_init_request_table(uint32_ttotal_requests)
{
http_stats_request_table_t*table=NULL;

table=(http_stats_request_table_t*)calloc(1,
sizeof(http_stats_request_table_t)+
sizeof(http_stats_request_t)*total_requests);

table->total_requests=total_requests;

returntable;

}

int
http_stats_request_compare(http_stats_request_t*stats1,http_stats_request_t*stats2)
{
if(stats1->total_transfer_time<stats2->total_transfer_time){
return-1;
}
if(stats1->total_transfer_time>stats2->total_transfer_time){
return+1;
}
return0;
}

void
http_stats_calculate_request_percentile(void*data)
{
structhttp_worker_base_t*base=(structhttp_worker_base_t*)data;
http_stats_request_table_t*table=base->stats.requests_stat_table;

if(!table){
return;
}

qsort(table->request_table,base->config->total_requests,
sizeof(http_stats_request_t),
(int(*)(constvoid*,constvoid*))http_stats_request_compare);

base->stats.request_percentile_95=table->request_table[
(95*base->config->total_requests)/100].total_transfer_time;
}

/*Statsfunctionsfortheotherfiles*/
void
http_stats_increment_requests_sent(http_connection_t*conn)
{
if(!conn){
return;
}
conn->stats.total_requests_sent++;
conn->base->stats.total_requests_sent++;
conn->base->stats.requests_sent_in_a_second++;
}

void
http_stats_increment_responses_received(http_connection_t*conn)
{
conn->stats.total_responses_received++;
conn->base->stats.total_responses_received++;
conn->base->stats.responses_received_in_a_second++;
}

void
http_stats_update_response_stats(http_connection_t*conn)
{
structhttp_worker_base_t*base;
http_stats_base_t*stats;
base=conn->base;
stats=&base->stats;

if(!conn){
return;
}

http_stats_update_connection_code(conn);
http_stats_increment_responses_received(conn);

if(conn->status==200){
stats->total_successful_connects++;
}else{
http_engine_update_worker_stats(base->stats,pthread_self());
return;
}

/*Calculatethetransfertimeforthisrequest*/
http_stats_update_transfer_request_time(conn);

conn->stats.downloaded_bytes+=evbuffer_get_length(conn->ev_req->buffer_in);

/*Updaterequestsstatstable*/
http_stats_update_request_stats(stats->requests_stat_table,
conn->stats.total_transfer_time);

/*Totalstats*/
stats->total_uploaded_bytes+=conn->stats.uploaded_bytes;
stats->total_downloaded_bytes+=conn->stats.downloaded_bytes;

/*Averagestats*/
http_stats_calculate_average(&stats->average_transfer_time,
conn->stats.total_transfer_time,
base->config->total_requests);

http_engine_update_worker_stats(base->stats,pthread_self());
}

void
http_stats_reset_per_second_stats(structhttp_worker_base_t*base)
{
base->stats.requests_sent_in_a_second=0;
base->stats.responses_received_in_a_second=0;
}

void
http_stats_update_connection_error(http_connection_t*conn)
{
if(!conn){
return;
}
conn->base->stats.total_errors++;
conn->base->stats.total_failed_connections++;
}

void
http_stats_update_request_error(http_connection_t*conn)
{
if(!conn){
return;
}
conn->base->stats.total_failed_connects++;
}

