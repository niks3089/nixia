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

#include<string.h>
#include<assert.h>
#include<sys/time.h>
#include<errno.h>
#include<evhtp.h>

#include"http_worker.h"
#include"http_connection.h"
#include"http_engine.h"
#include"log.h"

/*statenames*/
staticstruct{
char*state_name;
}http_connection_fsm_state_name_t[]={
[HTTP_CONN_STATE_UNITITIALIZED]={"unintialized"},
[HTTP_CONN_STATE_INIT]={"init"},
[HTTP_CONN_STATE_RUNNING]={"running"},
[HTTP_CONN_STATE_CLOSED]={"closed"},
[HTTP_CONN_STATE_ERROR]={"error"},
};

/*Statsfunctions*/
voidhttp_stats_update_for_connection(http_connection_t*conn);

typedefvoid(*http_connection_fsm_state_handler)(http_connection_t*conn);
statichttp_connection_fsm_state_handler
http_connection_fsm_state_handlers[HTTP_CONN_MAX_STATES];

voidhttp_connection_fsm_state_uninitialized(http_connection_t*conn);
voidhttp_connection_fsm_state_init(http_connection_t*conn);
voidhttp_connection_fsm_state_running(http_connection_t*conn);
voidhttp_connection_fsm_state_re_running(http_connection_t*conn);
voidhttp_connection_fsm_state_closed(http_connection_t*conn);
voidhttp_connection_fsm_state_error(http_connection_t*conn);
voidhttp_connection_fsm_state_unrecoverable_error(http_connection_t*conn);
char*http_connection_fsm_state_name(http_connection_t*conn);


char*
http_connection_fsm_state_name(http_connection_t*conn)
{
if(conn->state<HTTP_CONN_MAX_STATES){
returnhttp_connection_fsm_state_name_t[conn->state].state_name;
}
returnNULL;
}

staticvoid
http_connection_fsm_set_state(http_connection_t*conn,
http_connection_fsm_statestate)
{
zlog_info(log_get_cat_http(),"Conn(%u):Changingstatefrom%sto%s",
conn->conn_id,
http_connection_fsm_state_name_t[conn->state].state_name,
http_connection_fsm_state_name_t[state].state_name);

conn->state=state;
}

staticvoid
http_connection_fsm_state_change(http_connection_t*conn,
http_connection_fsm_statestate)
{
if(conn->state!=state&&conn->state<state){
http_connection_fsm_set_state(conn,state);
http_connection_fsm_process(conn);
}
}

void
http_connection_fsm_close(http_connection_t*conn)
{
if(!conn){
return;
}
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_CLOSED);
}

void
http_connection_fsm_close_all(structhttp_worker_base_t*base)
{
http_connection_pool_t*pool=base->connection_pool;
uint32_ttotal_conn=base->config->total_connections;
inti;

for(i=0;i<total_conn;i++){
http_connection_fsm_state_change(&pool->connection_table[i],
HTTP_CONN_STATE_CLOSED);
}
}

void
http_connection_fsm_process_all(structhttp_worker_base_t*base)
{
http_connection_pool_t*pool=base->connection_pool;
uint32_ttotal_conn=base->config->total_connections;
inti;

for(i=0;i<total_conn;i++){
http_connection_fsm_process(&pool->connection_table[i]);
}
}

void
http_connection_fsm_process(http_connection_t*conn)
{
if(!conn){
/*TODO:AddalogCrashfornow*/
}

http_connection_fsm_state_handlers[conn->state](conn);
}

/*EVHTPrequestisdone.Thisisacallbackwhenthathappens*/
staticevhtp_res
http_connection_fsm_request_completed_cb(evhtp_request_t*req,void*arg)
{
http_connection_t*conn=(http_connection_t*)arg;

conn->ev_req=NULL;
zlog_info(log_get_cat_http(),"%s:connid:%ustatus:%u,error:%d\n",
__FUNCTION__,conn->conn_id,req->status,req->error);
returnEVHTP_RES_OK;
}

void
http_connection_fsm_req_error_cb(evhtp_request_t*req,
evhtp_error_flagserrtype,void*arg)
{
http_connection_t*conn=(http_connection_t*)arg;
http_stats_update_request_error(conn);
}

staticevhtp_res
http_connection_fsm_error_cb(evhtp_connection_t*connection,
evhtp_error_flagserrtype,void*arg)
{
http_connection_t*conn=(http_connection_t*)arg;
zlog_info(log_get_cat_http(),"%s:Conn(%u):error_no%d%sevhtp:%d\n",
__FUNCTION__,conn->conn_id,
errno,strerror(errno),errtype);

if(pi_dll_queued(&conn->active_link)){
pi_dll_unlink(&conn->active_link);
}
if(errno){
conn->error=errno;
conn->error_msg=strdup(strerror(errno));
}
conn->ev_conn=NULL;

/*RuntheFSMforthisconnection*/
http_connection_fsm_process(conn);

/*TODO:Bughere.IFthehttpdserviceisstopped,
*wecanneverexit*/
if(http_worker_is_test_completed(conn->base)){
http_worker_stop_test(conn->base);
}
returnEVHTP_RES_OK;
}


/*EVHTPconnectionisdone.Thisisacallbackwhenthathappens*/
staticevhtp_res
http_connection_fsm_conn_completed_cb(evhtp_connection_t*connection,void*arg)
{
http_connection_t*conn=(http_connection_t*)arg;
zlog_info(log_get_cat_http(),"%s:Conn(%u):error_no%d%s\n",
__FUNCTION__,conn->conn_id,
errno,strerror(errno));

if(pi_dll_queued(&conn->active_link)){
pi_dll_unlink(&conn->active_link);
}
conn->ev_conn=NULL;

/*RuntheFSMforthisconnection*/
http_connection_fsm_process(conn);
returnEVHTP_RES_OK;
}


/*JustsetthestatetoINIT*/
void
http_connection_fsm_state_uninitialized(http_connection_t*conn)
{
http_connection_fsm_set_state(conn,HTTP_CONN_STATE_INIT);

}

/*SetupEVHTPconnectionobjectwhichattemptstoconnecttotheserver.
*SetupEVHTPrequestobjectandsendarequest*/
void
http_connection_fsm_state_init(http_connection_t*conn)
{
conn->base->connection_pool->total_connections_running++;
conn->base->stats.total_running_connections++;
conn->initialised=1;
http_stats_start_time(&conn->stats.conn_run_time);

/*Setupconnection*/
pi_dll_init(&conn->active_link);
pi_dll_insert_tail(&conn->base->connection_pool->active_list,
&conn->active_link);

conn->ev_conn=evhtp_connection_new(conn->base->evbase,
conn->base->config->url,80);

if(!conn->ev_conn){
conn->error=HTTP_CONN_ERR_EVHTP;
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_ERROR);
return;
}

/*Setuprequestforthisconnection*/
conn->ev_req=
evhtp_request_new(http_connection_check_completed_transfer,conn);

if(!conn->ev_req){
conn->error=HTTP_CONN_ERR_EVHTP;
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_ERROR);
return;
}

evhtp_set_hook(&conn->ev_conn->hooks,evhtp_hook_on_connection_fini,
http_connection_fsm_conn_completed_cb,conn);
evhtp_set_hook(&conn->ev_req->hooks,evhtp_hook_on_request_fini,
http_connection_fsm_request_completed_cb,conn);
evhtp_set_hook(&conn->ev_conn->hooks,evhtp_hook_on_conn_error,
(evhtp_hook)http_connection_fsm_error_cb,conn);
evhtp_set_hook(&conn->ev_req->hooks,evhtp_hook_on_error,
(evhtp_hook)http_connection_fsm_req_error_cb,conn);
evhtp_headers_add_header(conn->ev_req->headers_out,
evhtp_header_new("Host","ieatfood.net",0,0));
evhtp_headers_add_header(conn->ev_req->headers_out,
evhtp_header_new("User-Agent","libevhtp",0,0));

/*Sendtherequest*/
http_connection_send_request(conn);
http_connection_fsm_set_state(conn,HTTP_CONN_STATE_RUNNING);
}

/*InvokedbyFSMprocessingunderhttp_connection_check_completed_transfer()
*ProcesstheResponseandupdatethestats.
*IFdonewiththeconnection,gotoCLOSE
*IFsomethingiswrong,gotoERROR.
*IFneedtorunmorerequests,sendrequestsandstayinthisstate*/
void
http_connection_fsm_state_running(http_connection_t*conn)
{
/*Checkiftherewasanyerrorinthetransfer*/
if(conn->error){
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_ERROR);
return;
}
/*Bunchofcheckstodetermineifthisconnectionisdone*/

/*CheckifTestiscompleted.*/
if(http_worker_is_test_completed(conn->base)){
zlog_info(log_get_cat_http(),"Conn(%u):Testcompleted.closingconn",
conn->conn_id);
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_CLOSED);
return;
}

/*CheckifEVHTPconnisclosed.*/
if(!conn->ev_conn){
zlog_warn(log_get_cat_http(),"Conn(%u):EVHTPconnisNULL!Closingconn",
conn->conn_id);
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_CLOSED);
return;
}

/*Checkifwearedonesendingmaxrequestsforthisconn*/
if(conn->base->config->requests_per_connections&&
conn->stats.total_responses_received>=
conn->base->config->requests_per_connections){
/*Don'tsendanymore.*/
zlog_info(log_get_cat_http(),"Conn(%u):Sentmaxrequestsforthisconn",
conn->conn_id);
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_CLOSED);
return;
}

/*Checkifwehavesentmax_requests*/
if(conn->base->stats.total_requests_sent>=
conn->base->config->total_requests){
/*Don'tsendanymore.*/
zlog_info(log_get_cat_http(),"Conn(%u):Wehavesentoutmaxrequests."
"Totalsent:%u,Totalcompleted:%u,Sent:%u,Received:%u",
conn->conn_id,conn->stats.total_requests_sent,conn->base->stats.total_responses_received,
conn->base->stats.total_requests_sent,conn->stats.total_responses_received);
if(conn->stats.total_requests_sent==
conn->base->stats.total_responses_received){
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_CLOSED);
}
return;
}

/*Checkifwearedonesendingmaxrequestsforthisconn*/
if(conn->base->config->requests_per_connections&&
conn->stats.total_responses_received>=
conn->base->config->requests_per_connections){
/*Don'tsendanymore.*/
zlog_info(log_get_cat_http(),"Conn(%u):Sentmaxrequestsforthisconn",
conn->conn_id);
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_CLOSED);
return;
}

/*Checkifwehavealreadysentmaxrequestsinthissecond.
*IFwehave,donothing.Persecondtimerwillinvoke
*http_connection_re_run()whichwillsendtherequestnextsecond*/
if((conn->base->config->requests_per_second)&&
(conn->base->config->requests_per_second<=
conn->base->stats.requests_sent_in_a_second)){
return;
}

if(conn->ev_req){
http_connection_send_request(conn);
}else{
zlog_warn(log_get_cat_http(),"Conn(%u):EVHTPReqisNULL!GotoError",
conn->conn_id);
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_ERROR);
}
}

/*ClosetheTCPconnectionandremovetheconnfromactivelist*/
void
http_connection_fsm_state_closed(http_connection_t*conn)
{
http_connection_pool_t*pool;

/*Wehavealreadyclosedonce*/
if(conn->done){
return;
}
pool=conn->base->connection_pool;

/*BelowlineisaHack!evhtp_connection_free()resultsinsettingownerto0which
*resultsindoublefree.Doingthiswillhavelibeventfreetheconnection
*forus.*/
if(conn->ev_conn){
conn->ev_conn->owner=0;
/*evhtp_connection_free(conn->ev_conn);*/
}

if(pi_dll_queued(&conn->active_link)){
pi_dll_unlink(&conn->active_link);
}
conn->ev_conn=NULL;

if(conn->initialised){
pool->total_connections_running--;
http_stats_update_base_after_conn_completion(conn);
http_connection_dump_stats(conn);
conn->initialised=0;
}
http_engine_update_connection_pool(conn->base->connection_pool,
pthread_self());
#if0
if(conn->error_msg){
free(conn->error_msg);
conn->error_msg=NULL;
}
#endif
conn->done=1;
}

void
http_connection_fsm_state_error(http_connection_t*conn)
{
http_stats_update_connection_error(conn);
http_connection_fsm_state_change(conn,HTTP_CONN_STATE_CLOSED);
}

void
http_connection_fsm_init()
{
http_connection_fsm_state_handlers[HTTP_CONN_STATE_UNITITIALIZED]=
http_connection_fsm_state_uninitialized;

http_connection_fsm_state_handlers[HTTP_CONN_STATE_INIT]=
http_connection_fsm_state_init;

http_connection_fsm_state_handlers[HTTP_CONN_STATE_RUNNING]=
http_connection_fsm_state_running;

http_connection_fsm_state_handlers[HTTP_CONN_STATE_CLOSED]=
http_connection_fsm_state_closed;

http_connection_fsm_state_handlers[HTTP_CONN_STATE_ERROR]=
http_connection_fsm_state_error;
}
