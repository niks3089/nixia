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

/*
*Description:ConatinsAPIsformainandapplicationstocommunicatewith
*eachother.Allowsnewapplicationstoregisterhere
*/

#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<ncurses/ncurses.h>
#include<event2/event.h>

#include"common.h"
#include"protocol.h"
#include"main_process.h"
#include"log.h"
#include"json_parser.h"
#include"hiredis.h"

#include"http_process.h"
#include"http_config.h"

#include"csperf_process.h"
#include"csperf_config.h"

/*
*Protocolregisterationtable.
*Newprotocolsmustregisterhere.
*protocol_name:Nameoftheprotousedforlogging.
*protocol_set:Setwhenuserclicksonthecheckbox.
*protocol_test_completed_event:Eventwhichallowstoinformmainguiwhenthe
*testiscompleted.
*well
*protocol_config_init:Callbacktoinitializeconfig
*protocol_start_test:Callbacktostartthetestforthatprotocol
*protocol_display_output:CallbacktodisplaystatstoGUI
*protocol_stop_test:Callbacktostopthetestsandtraffic
*protocol_shutdown:Callbacktocleanup
*/
staticstructprotocol_table_t{
constchar*protocol_name;
uint8_tprotocol_set;
structevent*protocol_test_completed_event;

/*Callbackfunctions*/
protocol_config_init_tprotocol_config_init;
protocol_start_test_tprotocol_start_test;
protocol_display_output_tprotocol_display_output;
protocol_stop_test_tprotocol_stop_test;
protocol_shutdown_tprotocol_shutdown;
}protocol_table[]={
[PROTO_HTTP]={"http",0,NULL,
http_config_init,
http_process_start,
http_process_display_output,
http_process_stop,
http_process_shutdown},

[PROTO_CSPERF]={"csperf",0,NULL,
csperf_config_init,
csperf_process_start,
csperf_process_display_output,
csperf_process_stop,
csperf_process_shutdown},
};

int
protocol_table_init(char*protocol_list,char*json_config)
{
inti=0,error=0;
cJSON*root=cJSON_Parse(json_config);

/*Foralltheprotocolsintheprotocollist,setprotocol_set
*inprotocoltablewhenwillallowustoknowwhat
*protocoltoloadandwhen*/
for(i=0;i<PROTO_MAX;i++){
if(strstr(protocol_list,protocol_table[i].protocol_name)!=NULL){
zlog_debug(log_get_cat(),"Settingprotocol:%s",
protocol_table[i].protocol_name);
protocol_table[i].protocol_set=1;

if(protocol_table[i].protocol_config_init){
if((error=protocol_table[i].protocol_config_init(root))){
return-1;
}
}else{
zlog_error(log_get_cat(),"Nocallbacktosetupconfig");
return-1;
}
}
}
return0;
}

/*Calledbythemainactivitythreadeverysecond.Itcalls
*callbacksofotherprotocolstodisplaystats*/
void
protocol_display_output()
{
inti;

for(i=0;i<PROTO_MAX;i++){
if(protocol_table[i].protocol_set){
if(protocol_table[i].protocol_display_output){
protocol_table[i].protocol_display_output();
}
}
}
}

/*Informmainguithatthistestiscompleted.
*Thiswillresultincalltoprotocol_handle_test_completion()*/
void
protocol_announce_test_is_completed(intprotocol)
{
if(protocol_table[protocol].protocol_set){
event_active(protocol_table[protocol].protocol_test_completed_event,
EV_READ|EV_WRITE,1);
}
}

/*Protocolisdonewithitstest.*/
staticvoid
protocol_handle_test_completion(evutil_socket_tfd,shortevents,void*user_data)
{
inti;
/*Informmainaboutthisandifallaredone.
*Dowhateverisnecessary*/
structprotocol_table_t*protocol_info=
(structprotocol_table_t*)user_data;

assert(protocol_info);

if(protocol_info->protocol_set){
assert(protocol_info->protocol_test_completed_event);
event_del(protocol_info->protocol_test_completed_event);
protocol_info->protocol_test_completed_event=NULL;
}

/*Checkifallprotocolsaredone.Ifnot,return*/
for(i=0;i<PROTO_MAX;i++){
if(protocol_table[i].protocol_set){
if(protocol_table[i].protocol_test_completed_event){
return;
}
}
}

/*Alltestsarecompleted*/
protocol_display_output();
main_process_tests_completed();
}

void
protocol_setup_test_completion_event(intprotocol)
{
if(protocol_table[protocol].protocol_set){
protocol_table[protocol].protocol_test_completed_event=
event_new(main_process_get_output_process()->evbase,-1,EV_READ|EV_WRITE,
protocol_handle_test_completion,&protocol_table[protocol]);
event_add(protocol_table[protocol].protocol_test_completed_event,NULL);
}
}

void
protocol_start_test()
{
inti;
char*directory=NULL;

for(i=0;i<PROTO_MAX;i++){
if(protocol_table[i].protocol_set){
if(protocol_table[i].protocol_start_test){
protocol_setup_test_completion_event(i);

/*Beforerunningtest,createoutputdirectories*/
main_process_setup_protocol_directory(
protocol_table[i].protocol_name,&directory);

/*Starttest*/
protocol_table[i].protocol_start_test(directory);
}else{
zlog_error(log_get_cat(),"Oops!Nostarttesthandlerforprotocol:%s",
protocol_table[i].protocol_name);
}
}
}
}

void
protocol_stop_test()
{
inti;

for(i=0;i<PROTO_MAX;i++){
if(protocol_table[i].protocol_set){
if(protocol_table[i].protocol_stop_test){
protocol_table[i].protocol_stop_test();
}else{
//common_gui_exit_with_error("Failedtostopthetest");
zlog_error(log_get_cat(),"Oops!Nostop_testhandlerforprotocol:%s",
protocol_table[i].protocol_name);
}
}
}
}

void
protocol_shutdown()
{
inti;

for(i=0;i<PROTO_MAX;i++){
if(protocol_table[i].protocol_set){
if(protocol_table[i].protocol_shutdown){
protocol_table[i].protocol_shutdown();
}else{
zlog_error(log_get_cat(),"Oops!Nostarttesthandlerforprotocol:%s",
protocol_table[i].protocol_name);
}
}
}
}

/*TODO:Movethistocommon.c*/

/*Whoeverusesthisfunctionhastheresponsibility
*tofreefilename*/
char*
protocol_get_filename(char*directory,char*filename)
{
chartemp_name[MAX_NAME_LENGTH]={0};

if(!directory||!filename){
returnNULL;
}

if((strlen(directory)+strlen(filename))>=MAX_NAME_LENGTH){
returnNULL;
}

strncpy(temp_name,directory,strlen(directory));
strcat(temp_name,"/");
strncat(temp_name,filename,strlen(filename));

return(strdup(temp_name));
}

FILE*
protocol_create_file(char*directory,char*filename)
{
char*filepath;
FILE*new_file=NULL;

if(!directory||!filename){
returnNULL;
}

if(!(filepath=protocol_get_filename(directory,filename))){
zlog_warn(log_get_cat_http(),"Failedtogetthefilepathfor%s",
filename);
returnNULL;
}
new_file=fopen(filepath,"w");
free(filepath);
returnnew_file;
}

void
protocol_helper_display_result(char*output)
{
redisReply*reply;

if(main_config_is_cli_mode()){
mvprintw(0,0,"%s",output);
refresh();
}else{
reply=redisCommand(main_process_get_output_process()->redis_ctx,
"PUBLISH%s%s",
main_config_get()->redis_channel,output);

if(reply){
zlog_info(log_get_cat(),"replycommand:%d:%s\n",reply->type,reply->str);
}
}
}
