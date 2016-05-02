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
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>

#include"log.h"
#include"main_config.h"
#include"http_worker.h"
#include"http_stats.h"

#defineACTIVE_LINK_TO_CONN_ENT(_h)(http_connection_t*)((char*)_h-offsetof(http_connection_t,active_link))

/*Checkifwehavestartedalltheconnections*/
int
http_connections_loaded_max_connections(structhttp_worker_base_t*base)
{
return(base->connection_pool->next_connection_to_run>=
base->config->total_connections);
}

/*CheckifwehavealreadysentourquotaofHTTPrequests*/
int
http_connection_should_send_next_request(structhttp_worker_base_t*base)
{
if(base->stats.total_requests_sent>=
base->config->total_requests){
zlog_info(log_get_cat_http(),"Wehavealreadysent"
"maxrequests.Sent:%u,Max:%u",
base->stats.total_requests_sent,base->config->total_requests);
return0;
}
if(base->config->requests_per_second&&
base->stats.requests_sent_in_a_second>=
base->config->requests_per_second){
return0;
}
return1;
}

int
http_connection_should_run_next_connection(structhttp_worker_base_t*base)
{
/*Noneedifthetestiscompleted*/
if(http_worker_is_test_completed(base)){
return0;
}

/*Noneedtorunaconnectionifwedon'thavetosend
*anyrequests*/
if(!http_connection_should_send_next_request(base)){
return0;
}

/*Onesecondtimerisgoingtoruntheconnections.
*Weshouldn'tdoithere*/
if(base->config->connections_per_second){
return0;
}

/*Wehaveloadedmaxconnections*/
if(http_connections_loaded_max_connections(base)){
return0;
}

/*Noneedifwearerunningatmax*/
if(base->config->concurrency){
if(base->connection_pool->total_connections_running>=
base->config->concurrency){
zlog_info(log_get_cat_http(),"Alreadyrunning"
"maxconcurrentconnections");
return0;
}
}
return1;
}

/*SendHTTPrequest*/
void
http_connection_send_request(http_connection_t*conn)
{
if(conn&&conn->ev_conn&&conn->ev_req){
http_stats_increment_requests_sent(conn);
http_stats_start_time(&conn->stats.request_transfer_time);
evhtp_make_request(conn->ev_conn,conn->ev_req,htp_method_GET,
http_config_get()->url_path);
zlog_info(log_get_cat_http(),"Conn(%u):Sendingrequest.Totalsent:%u"
"Connectionsent:%urequestpath:%s",
conn->conn_id,conn->base->stats.total_requests_sent,
conn->stats.total_requests_sent,http_config_get()->url_path);
}else{
assert(0);
}
}


/*Calledwhenrunningrps*/
void
http_connection_re_run(structhttp_worker_base_t*base)
{
pi_dll_t*list,*dll;
http_connection_t*conn=NULL;

list=&base->connection_pool->active_list;

/*Checkifthereareanyactiveconnections*/
for(dll=list->dll_next;dll!=list&&dll;
dll=dll->dll_next){

if(!(http_connection_should_send_next_request(base))){
break;
}
conn=(http_connection_t*)dll;

assert(conn&&conn->ev_req&&conn->ev_conn);
http_connection_send_request(conn);
}
/*Runnextconnectioniftherearenoactiveconnectionstorun*/
if(!conn&&
base->stats.total_requests_sent>=base->config->total_requests){
http_connection_run_next_connection(base);
return;
}
}

/*Freeconnectionpool*/
void
http_connection_pool_cleanup(structhttp_worker_base_t*base,
http_connection_pool_t*pool)
{
/*TODO:ConnFSMcleanuptoDe-queueactivelinksand
*closeTCPconnections*/
free(pool);
pool=NULL;
}

/*CalledwhenaHTTPrespisreceived.*/
void
http_connection_check_completed_transfer(evhtp_request_t*req,void*arg)
{
http_connection_t*conn=(http_connection_t*)arg;
structhttp_worker_base_t*base=conn->base;

/*Updateconnectionstats*/
if(req){
conn->status=evhtp_request_status(req);
}
assert(conn->state==HTTP_CONN_STATE_RUNNING);

/*Runconnectionfsmhere*/
http_stats_update_response_stats(conn);
http_connection_fsm_process(conn);

zlog_info(log_get_cat_http(),"Conn(%u):Requestcompleted.Responsestatus:%u"
"totalreqcompl:%uconnreqcompletd:%uTotalreqsent:%u,"
"Totalconnectonsrunning:%u,Connectionscompleted:%u",
conn->conn_id,conn->status,base->stats.total_responses_received,
conn->stats.total_responses_received,base->stats.total_requests_sent,
conn->base->connection_pool->total_connections_running,
base->stats.total_completed_connections);

if(!http_worker_is_test_completed(base)){
/*Checkifweneedtorunnewconnection*/
if(http_connection_should_run_next_connection(base)){
http_connection_run_next_connection(base);
}
}else{
/*Testiscompleted.Collectstats*/
zlog_info(log_get_cat_http(),"Testscompleted.Totalreqsent:%u"
"Totalreqcompleted:%u,Connectionscompleted:%u",
base->stats.total_requests_sent,
base->stats.total_responses_received,base->stats.total_completed_connections);
http_worker_stop_test_if_completed(base);
}
}

/*Setupourconnectionobject*/
void
http_connection_setup(http_connection_t*conn,uint32_tid,
structhttp_worker_base_t*base)
{
/*Setupconnection*/
conn->conn_id=id;
conn->base=base;

/*SettoInitstate*/
http_connection_fsm_process(conn);
}

/*Inittheconnectionpoolandalltheconnectionobjects*/
http_connection_pool_t*
http_connection_init_pool(void*data)
{
inti;
http_connection_pool_t*pool;
structhttp_worker_base_t*base=(structhttp_worker_base_t*)data;
http_worker_test_config_t*config=base->config;
uint16_ttotal_connections=config->total_connections;

pool=(http_connection_pool_t*)calloc(1,sizeof(http_connection_pool_t)+
(sizeof(http_connection_t)*total_connections));

if(!pool){
returnNULL;
}

pool->total_connections=total_connections;
pool->total_pending_connections=0;

pool->base=base;
pi_dll_init(&pool->active_list);

for(i=0;i<total_connections;i++){
http_connection_setup(&pool->connection_table[i],i,base);
}
returnpool;
}

int
http_connection_run_next_connection(structhttp_worker_base_t*base)
{
http_connection_pool_t*pool=base->connection_pool;
http_worker_test_config_t*config=base->config;
http_connection_t*conn=NULL;

if(pool->next_connection_to_run<config->total_connections){
conn=&pool->connection_table[pool->next_connection_to_run++];
zlog_info(log_get_cat_http(),"Conn(%u):Loadingconnection",
conn->conn_id);

/*ConnectionshouldbeinINITstate.Connecttotheserver
*andbegintransfer*/
http_connection_fsm_process(conn);
return0;
}
return-1;
}

void
http_connection_run_connections(structhttp_worker_base_t*base,uint32_tcount)
{
uint32_ti;

zlog_info(log_get_cat_http(),"Loading%uconnections."
"nextcontorun:%u",count,
base->connection_pool->next_connection_to_run);

for(i=0;i<count;i++){
http_connection_run_next_connection(base);
}
}

/*Initially,howmanyconnectionstorun?*/
void
http_connection_kickstart(structhttp_worker_base_t*base)
{
http_worker_test_config_t*config=base->config;

if(config->concurrency){
http_connection_run_connections(base,config->concurrency);
}elseif(config->connections_per_second){
http_connection_run_connections(base,config->connections_per_second);
}elseif(config->requests_per_second){
http_connection_run_connections(base,config->requests_per_second);
}else{
http_connection_run_connections(base,1);
}
}

void
http_connection_dump_stats(http_connection_t*conn)
{
if(!conn){
return;
}

zlog_info(log_get_cat_http(),"Connectionstatsforconn%u",conn->conn_id);
zlog_info(log_get_cat_http(),"-----------------");
zlog_info(log_get_cat_http(),"total_transfer_time:%f",conn->stats.total_transfer_time);
zlog_info(log_get_cat_http(),"uploaded_bytes:%f",conn->stats.uploaded_bytes);
zlog_info(log_get_cat_http(),"upload_speed:%f",conn->stats.upload_speed);
zlog_info(log_get_cat_http(),"downloaded_bytes:%f",conn->stats.downloaded_bytes);
zlog_info(log_get_cat_http(),"download_speed:%f",conn->stats.download_speed);
zlog_info(log_get_cat_http(),"total_requests:%u",conn->stats.total_requests_sent);

zlog_info(log_get_cat_http(),"--------------------------------------------");
}
