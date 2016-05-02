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

#include<errno.h>

#include<string.h>
#include<signal.h>
#include<pthread.h>
#include<event2/thread.h>
#include<assert.h>

#include"protocol.h"

#include"http_engine.h"
#include"http_worker.h"
#include"http_defaults.h"
#include"http_stats.h"
#include"http_output.h"

#include"log.h"

voidhttp_worker_base_cleanup(structhttp_worker_base_t*base);

void
http_worker_dump_base_stats(structhttp_worker_base_t*base)
{
zlog_info(log_get_cat_http(),"Workerresult");
zlog_info(log_get_cat_http(),"---------------------");

zlog_info(log_get_cat_http(),"total_running_connections:%u",base->stats.total_running_connections);
zlog_info(log_get_cat_http(),"total_connectionscompleted:%u",base->stats.total_completed_connections);
zlog_info(log_get_cat_http(),"total_requestssent:%u",base->stats.total_requests_sent);
zlog_info(log_get_cat_http(),"total_responses_received:%u",base->stats.total_responses_received);
zlog_info(log_get_cat_http(),"total_errors:%u",base->stats.total_errors);
zlog_info(log_get_cat_http(),"total_successful_connects:%u",base->stats.total_successful_connects);
zlog_info(log_get_cat_http(),"total_failed_connects:%u",base->stats.total_failed_connects);
zlog_info(log_get_cat_http(),"total_failed_connections:%u",base->stats.total_failed_connections);
zlog_info(log_get_cat_http(),"total_run_time:%fms",base->stats.total_run_time);
zlog_info(log_get_cat_http(),"total_uploaded_bytes:%u",base->stats.total_uploaded_bytes);
zlog_info(log_get_cat_http(),"total_downloaded_bytes:%u",base->stats.total_downloaded_bytes);
zlog_info(log_get_cat_http(),"total_http_code_1xx:%u",base->stats.total_http_code_1xx);
zlog_info(log_get_cat_http(),"total_http_code_2xx:%u",base->stats.total_http_code_2xx);
zlog_info(log_get_cat_http(),"total_http_code_3xx:%u",base->stats.total_http_code_3xx);
zlog_info(log_get_cat_http(),"total_http_code_4xx:%u",base->stats.total_http_code_4xx);
zlog_info(log_get_cat_http(),"total_http_code_5xx:%u",base->stats.total_http_code_5xx);

zlog_info(log_get_cat_http(),"average_transfer_time:%f",base->stats.average_transfer_time);
zlog_info(log_get_cat_http(),"average_upload_speed:%f",base->stats.average_upload_speed);
zlog_info(log_get_cat_http(),"average_download_speed:%f",base->stats.average_download_speed);
zlog_info(log_get_cat_http(),"average_requests_sent_in_a_second:%f",base->stats.average_requests_sent_in_a_second);
zlog_info(log_get_cat_http(),"average_responses_received_in_a_second:%f",base->stats.average_responses_received_in_a_second);
}

int
http_worker_finished_sending_requests(structhttp_worker_base_t*base)
{
return((base->stats.total_requests_sent>=base->config->total_requests));
}

int
http_worker_is_test_completed(structhttp_worker_base_t*base)
{
return(((base->stats.total_responses_received>=base->config->total_requests))
||(base->stats.total_completed_connections>=base->config->total_connections));
}

void
http_worker_save_test_result(structhttp_worker_base_t*base)
{
http_output_worker_summary_to_file(&base->stats,base->worker_summary_file);
http_output_connection_summary_to_file(base);
}

void
http_worker_update_final_stats(structhttp_worker_base_t*base)
{
http_stats_calculate_request_percentile(base);

/*Lettheengineknowaboutthechangesdone*/
http_engine_update_worker_stats(base->stats,pthread_self());
}

void
http_worker_stop_test(structhttp_worker_base_t*base)
{
if(evtimer_pending(base->per_second_timer_event,NULL)){
evtimer_del(base->per_second_timer_event);
zlog_info(log_get_cat_http(),"Deletingsecondtimer");
}

zlog_info(log_get_cat_http(),"Stoppingtest");
http_stats_update_base_run_time(base);
event_base_loopbreak(base->evbase);
/*TODO:Closealltheevhtpconnections*/
http_connection_fsm_close_all(base);

/*Calcluatefinalstats*/
http_worker_update_final_stats(base);

/*Copytheconnectionpool*/
http_engine_copy_connection_pool(base->connection_pool,
sizeof(http_connection_pool_t)+
sizeof(http_connection_t)*base->connection_pool->total_connections,
pthread_self());

/*Savetheoutputtothefiles*/
http_worker_save_test_result(base);
}

void
http_worker_stop_test_if_completed(structhttp_worker_base_t*base)
{
if(http_worker_is_test_completed(base)){
http_worker_stop_test(base);
}
}

void
http_connection_run_requests_per_second(structhttp_worker_base_t*base)
{
staticintcur_second=0;
if(base->config->requests_per_second>
base->stats.requests_sent_in_a_second){
zlog_warn(log_get_cat_http(),"Couldnotsendthenecessary"
"amountrequestsinonesecond."
"Expectedtobesent:%u,Sent:%u",
base->config->requests_per_second,
base->stats.requests_sent_in_a_second);
}

if(base->config->requests_per_second){
zlog_info(log_get_cat_http(),"Requestsummaryinasecond:"
"Expectedtobesent:%u,Sent:%u"
"Received:%u",
base->config->requests_per_second,
base->stats.requests_sent_in_a_second,
base->stats.responses_received_in_a_second);

zlog_info(log_get_cat_http(),"Runningrps");
}
http_stats_calculate_average(
&base->stats.average_requests_sent_in_a_second,
base->stats.requests_sent_in_a_second,++cur_second);

http_stats_calculate_average(
&base->stats.average_responses_received_in_a_second,
base->stats.responses_received_in_a_second,++cur_second);

http_stats_reset_per_second_stats(base);

/*Startthetransfersforthenextsecond*/
if(base->config->requests_per_second){
http_connection_re_run(base);
}
}

staticint
http_worker_per_second_timer_update(structhttp_worker_base_t*base)
{
structtimevaltimeout;
timeout.tv_sec=1;
timeout.tv_usec=0;

evtimer_add(base->per_second_timer_event,&timeout);

return0;
}

staticvoid
http_worker_per_second_timer_callback(intfd,shortkind,void*userp)
{
structhttp_worker_base_t*base=(structhttp_worker_base_t*)userp;

if(base->config->connections_per_second){
http_connection_run_connections(base,
base->config->connections_per_second);
}

http_connection_run_requests_per_second(base);
http_worker_per_second_timer_update(base);
}

staticvoid
http_worker_user_event_handler(evutil_socket_tfd,shortevents,void*user_data)
{
structhttp_worker_base_t*base;

base=(structhttp_worker_base_t*)user_data;
zlog_info(log_get_cat_http(),"Gotausertriggeredevent");
http_worker_stop_test(base);
}

staticstructhttp_worker_base_t*
http_worker_init_base(http_worker_test_config_t*config)
{
structhttp_worker_base_t*base;

base=(structhttp_worker_base_t*)malloc(sizeof(structhttp_worker_base_t));

if(!base){
returnNULL;
}
memset(base,0,sizeof(structhttp_worker_base_t));
base->evbase=event_base_new();

evthread_use_pthreads();

if(evthread_make_base_notifiable(base->evbase)<0){
free(base);
returnNULL;
}

base->per_second_timer_event=evtimer_new(base->evbase,
http_worker_per_second_timer_callback,base);
base->user_event=event_new(base->evbase,-1,EV_READ|EV_WRITE,
http_worker_user_event_handler,base);
event_add(base->user_event,NULL);

http_engine_setup_user_event(base->user_event,pthread_self());

base->config=config;

/*Setuprequestsstats*/
base->stats.requests_stat_table=
http_stats_init_request_table(config->total_requests);

if(!(base->stats.requests_stat_table)){
zlog_warn(log_get_cat_http(),"Failedtosetuprequeststatstable");
http_worker_base_cleanup(base);
returnNULL;
}

/*Setupfiletowritetheworkertestsummary*/
if(!(base->worker_summary_file=protocol_create_file(
config->output_directory,HTTP_DEFAULT_TEST_SUMMARY_FILE))){
zlog_warn(log_get_cat_http(),"Failedtocreatetestsummaryfile");
http_worker_base_cleanup(base);
returnNULL;
}

/*Setupfiletowritetheconnectionsummary*/
if(!(base->connection_summary_file=protocol_create_file(
config->output_directory,HTTP_DEFAULT_TEST_CONNECTION_FILE))){
zlog_warn(log_get_cat_http(),"Failedtocreatetest"
"connectionsummaryfile");
http_worker_base_cleanup(base);
returnNULL;
}

returnbase;
}

int
http_worker_setup_events(structhttp_worker_base_t*base)
{
return0;
}

int
http_worker_setup_timer(structhttp_worker_base_t*base)
{
/*Seealloc_init_timer_waiting_queue*/
http_worker_per_second_timer_update(base);
return0;
}

int
http_worker_setup_timers(structhttp_worker_base_t*base)
{
http_worker_setup_timer(base);
return0;
}

void
http_worker_base_cleanup(structhttp_worker_base_t*base)
{
http_engine_remove_user_event(base->user_event,pthread_self());
http_connection_pool_cleanup(base,base->connection_pool);
event_free(base->per_second_timer_event);
event_free(base->user_event);
event_base_free(base->evbase);

if(base->worker_summary_file){
fclose(base->worker_summary_file);
}

if(base->connection_summary_file){
fclose(base->connection_summary_file);
}

if(base->config->output_directory){
free(base->config->output_directory);
}

if(base->config){
free(base->config);
}

if(base->stats.requests_stat_table){
free(base->stats.requests_stat_table);
}

free(base);
base=NULL;
}

int
http_worker_run_test(structhttp_worker_base_t*base)
{
//base->running_connections=1;

http_connection_kickstart(base);

event_base_dispatch(base->evbase);
return0;
}

/*Workerthread.*/
void*
http_worker_thread(void*data)
{
http_worker_test_config_t*config;
structhttp_worker_base_t*base;

config=(http_worker_test_config_t*)data;
zlog_error(log_get_cat_http(),"Inworkerthread");

/*InitConnectionfsm*/
http_connection_fsm_init();

/*InitBaseconfig*/
if(!(base=http_worker_init_base(config))){
zlog_error(log_get_cat_http(),"Failedtoinithttpbase");
returnNULL;
}

http_worker_setup_timers(base);

/*Initializetheconnectionpool*/
if(!(base->connection_pool=http_connection_init_pool(base))){
zlog_error(log_get_cat_http(),"Failedtoinithttpconnectionpool");
free(base);
returnNULL;
}

http_stats_start_time(&base->stats.run_time);
http_worker_run_test(base);

zlog_info(log_get_cat_http(),"Donetesting:Connectionsleft:%d",
base->running_connections);

http_config_dump();
http_worker_dump_base_stats(base);

/*Collectstats*/
/*Cleanuptheengine*/
http_worker_base_cleanup(base);
returnNULL;
}
