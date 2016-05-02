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

#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<unistd.h>
#include<assert.h>

#include"http_output.h"
#include"http_worker.h"
#include"http_connection.h"
#include"common.h"
#include"log.h"
#include"cJSON.h"
#include"main_config.h"

char*
http_output_worker_summary(http_stats_base_t*stats)
{

cJSON*root,*connection_summary,*request_summary,
*transfer_summary,*header;
root=cJSON_CreateObject();

if(!main_config_is_cli_mode()){
cJSON_AddItemToObject(root,"header",
header=cJSON_CreateObject());
cJSON_AddStringToObject(header,"protocol","http");
}

cJSON_AddItemToObject(root,"connectionsummary",
connection_summary=cJSON_CreateObject());
cJSON_AddItemToObject(root,"requestsummary",
request_summary=cJSON_CreateObject());
cJSON_AddItemToObject(root,"transfersummary",
transfer_summary=cJSON_CreateObject());

cJSON_AddNumberToObject(connection_summary,"TotalRunningconnections",
stats->total_running_connections);
cJSON_AddNumberToObject(connection_summary,"Totalcompletedconnections",
stats->total_completed_connections);
cJSON_AddNumberToObject(connection_summary,"Totalsuccessfulconnects",
stats->total_successful_connects);
cJSON_AddNumberToObject(connection_summary,"Totalfailedconnects",
stats->total_failed_connects);

cJSON_AddNumberToObject(request_summary,"Totalrequestssent",
stats->total_requests_sent);
cJSON_AddNumberToObject(request_summary,"Totalresponsesreceived",
stats->total_responses_received);
cJSON_AddNumberToObject(request_summary,"Totalerrors",
stats->total_errors);
cJSON_AddNumberToObject(request_summary,"Total1xxresponses",
stats->total_http_code_1xx);
cJSON_AddNumberToObject(request_summary,"Total2xxresponses",
stats->total_http_code_2xx);
cJSON_AddNumberToObject(request_summary,"Total3xxresponses",
stats->total_http_code_3xx);
cJSON_AddNumberToObject(request_summary,"Total4xxresponses",
stats->total_http_code_4xx);
cJSON_AddNumberToObject(request_summary,"Total5xxresponses",
stats->total_http_code_5xx);

cJSON_AddNumberToObject(transfer_summary,"Totalruntime",
stats->total_run_time);
cJSON_AddNumberToObject(transfer_summary,"Totaluploadedbytes",
stats->total_uploaded_bytes);
cJSON_AddNumberToObject(transfer_summary,"Totaldownloadedbytes",
stats->total_downloaded_bytes);
cJSON_AddNumberToObject(transfer_summary,"Averagetransfertime",
stats->average_transfer_time);
cJSON_AddNumberToObject(transfer_summary,"Averagerequestssentinasecond",
stats->average_requests_sent_in_a_second);
cJSON_AddNumberToObject(transfer_summary,"Averagerequestsreceivedinasecond",
stats->average_requests_sent_in_a_second);
cJSON_AddNumberToObject(transfer_summary,"95percentileofTotaltransfertime",
stats->request_percentile_95);

zlog_info(log_get_cat_http(),"Jsondumpis:%s",cJSON_Print(root));
returncJSON_Print(root);
}

char*
http_output_connection_summary(http_connection_pool_t*pool)
{
http_connection_t*conn;
inti;

cJSON*root,*connection_summary;
root=cJSON_CreateObject();

if(!pool){
returnNULL;
}

for(i=0;i<pool->total_connections;i++){
conn=&pool->connection_table[i];
cJSON_AddItemToObject(root,"connectiondetail",
connection_summary=cJSON_CreateObject());
cJSON_AddNumberToObject(connection_summary,"Connectionid",
conn->conn_id);
cJSON_AddStringToObject(connection_summary,"Connectionstate",
http_connection_fsm_state_name(conn));
cJSON_AddNumberToObject(connection_summary,"Requestssent",
conn->stats.total_requests_sent);
cJSON_AddNumberToObject(connection_summary,"Responsesreceived",
conn->stats.total_responses_received);
cJSON_AddNumberToObject(connection_summary,"Runtime",
conn->stats.total_connection_time);
}
returncJSON_Print(root);
}

#if0
void
http_output_test_config(char**writer,int32_t*allocated_mem,
int32_t*mem_left,myprintf_tmyprintf)
{
http_config_t*config=http_config_get();

http_output_write(writer,allocated_mem,mem_left,myprintf,
"TestConfig\n");

http_output_write(writer,allocated_mem,mem_left,myprintf,
"--------------------------------------------------\n");

http_output_write(writer,allocated_mem,mem_left,myprintf,
"Totalconnections:%u\n",config->total_connections);
http_output_write(writer,allocated_mem,mem_left,myprintf,
"Concurrency:%u\n",config->concurrency);
http_output_write(writer,allocated_mem,mem_left,myprintf,
"Connectionspersecond:%u\n",config->connections_per_second);

http_output_write(writer,allocated_mem,mem_left,myprintf,
"TotalRequests:%u\n",config->total_requests);
http_output_write(writer,allocated_mem,mem_left,myprintf,
"Requestspersecond:%u\n",config->requests_per_second);
http_output_write(writer,allocated_mem,mem_left,myprintf,
"Pipelinerequests:%s\n",(config->pipeline_requests)?
"YES":"NO");
http_output_write(writer,allocated_mem,mem_left,myprintf,
"Dividerequests:%s\n",(config->divide_requests_equally)?
"YES":"NO");
}
#endif

void
http_output_worker_summary_to_file(http_stats_base_t*stats,FILE*fp)
{
char*output;
output=http_output_worker_summary(stats);
fprintf(fp,output);
}

char*
http_output_worker_summary_to_string(http_stats_base_t*stats)
{
returnhttp_output_worker_summary(stats);
}

void
http_output_connection_summary_to_file(structhttp_worker_base_t*base)
{
char*output;
output=http_output_connection_summary(base->connection_pool);
fprintf(base->connection_summary_file,output);
}

char*
http_output_connection_summary_to_string(http_connection_pool_t*pool)
{
returnhttp_output_connection_summary(pool);
}

void
http_output_test_config_to_string(char**output_str)
{
#if0
int32_tallocated_mem,mem_left;

*output_str=(char*)calloc(1,MAX_MSG_LENGTH);

if(!*output_str){
return;
}

allocated_mem=mem_left=MAX_MSG_LENGTH;

http_output_test_config(output_str,
&allocated_mem,&mem_left,http_output_write_to_string);
#endif
}
