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
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<pthread.h>
#include<event2/event.h>
#include<event2/thread.h>
#include<ncurses/ncurses.h>

#include"log.h"
#include"common.h"

#include"main_process.h"
#include"main_config.h"
#include"protocol.h"

main_process_output_t*g_main_output_process=NULL;
staticintg_main_running=0;

main_process_output_t*
main_process_get_output_process()
{
returng_main_output_process;
}

void
main_process_setup_protocol_directory(constchar*protocol,char**directory)
{
main_config_t*config=main_config_get();
chardir_name[MAX_NAME_LENGTH]={0};
structstatst={0};
interror=0;

if(!protocol||!config||!directory){
return;
}

if((strlen(config->test_name)+strlen(protocol))>=
MAX_NAME_LENGTH){
return;
}

/*TODO:Fixthesekindofbugsthatcanleadtobufferoverflow*/
strncpy(dir_name,config->test_name,strlen(config->test_name));
strcat(dir_name,"/");
strncat(dir_name,protocol,strlen(protocol));

if(stat(dir_name,&st)==-1){
zlog_info(log_get_cat(),"Creatingdirectory:%s",dir_name);
error=mkdir(dir_name,0777);
}

if(error){
zlog_error(log_get_cat(),"Failedtocreate"
"subdirectories:%serr:%d",
dir_name,error);
//common_gui_exit_with_error("Failedtocreatesubdirectories");
}

*directory=strdup(dir_name);
}

void
main_process_set_up_test_directory()
{
interror=0;
main_config_t*config=main_config_get();
structstatst={0};
charconfig_dir_name[MAX_NAME_LENGTH]={0};

if(!config){
zlog_error(log_get_cat(),"Failedtosetupdirectory");
return;
}

if(stat(config->test_name,&st)==-1){
error=mkdir(config->test_name,0777);
}

if(error){
//common_gui_exit_with_error("Failedtocreatedirectories");
}

strncpy(config_dir_name,config->test_name,strlen(config->test_name));
strcat(config_dir_name,"/config");

if(stat(config_dir_name,&st)==-1){
zlog_info(log_get_cat(),"Creatingdirectory:%s",config_dir_name);
error=mkdir(config_dir_name,0777);
}

if(error){
//common_gui_exit_with_error("Failedtoconfigdirectory");
}

config->config_directory=strdup(config_dir_name);

zlog_info(log_get_cat(),"Createddirectory:%s",config->config_directory);
}

void
main_process_output_stuff()
{
main_config_t*config=main_config_get();
charfilename[MAX_NAME_LENGTH]={0};

if(!config||!config->config_directory){
return;
}

strncpy(filename,config->config_directory,
strlen(config->config_directory));
strcat(filename,"/config.xml");

//xml_writer_write_to_file(filename,config->config_xml);
}

int
main_process_connect_with_redis()
{
main_config_t*config=main_config_get();
main_process_output_t*output=main_process_get_output_process();

if(!config->redis_channel||!config->redis_server||
!config->redis_port){
zlog_error(log_get_cat(),"DonothavetheconfigtorunGUImode");
return-1;
}

/*Thisisablockingconnectonpurpose...*/
zlog_error(log_get_cat(),"Connectingtoredison%s:%u",
config->redis_server,config->redis_port);
output->redis_ctx=redisConnect(config->redis_server,config->redis_port);

if(!output->redis_ctx||output->redis_ctx->err){
if(output->redis_ctx){
zlog_warn(log_get_cat(),"Connectionerror:%s\n",
output->redis_ctx->errstr);
}else{
zlog_error(log_get_cat(),"Connectionerror:"
"can'tallocaterediscontext\n");
}
return-1;
}

/*Connectedsuccessfuly*/
zlog_info(log_get_cat(),"Connectedsuccessfullytoredis"
"server,%s:%u",config->redis_server,config->redis_port);
return0;
}

void
main_process_start_test()
{
zlog_info(log_get_cat(),"Startingtest");

main_process_set_up_test_directory();
protocol_start_test();
}

void
main_process_stop_test()
{
zlog_info(log_get_cat(),"Stoppingtest");

/*TODO:Savefilesandstuffhere*/
main_process_output_stuff();

protocol_stop_test();
}

void
main_process_output_cleanup()
{
if(g_main_output_process){
zlog_info(log_get_cat(),"Stoppingmaintthread");
event_base_loopbreak(g_main_output_process->evbase);
event_base_free(g_main_output_process->evbase);
if(g_main_output_process->redis_ctx){
redisFree(g_main_output_process->redis_ctx);
}
free(g_main_output_process);
}
}

void
main_process_shutdown()
{
intcli_mode=0;
zlog_info(log_get_cat(),"Shuttingdown");

if(main_config_is_cli_mode()){
cli_mode=1;
printw("\nTestscompleted.Resultsin'%s'directory\n",
main_config_get()->test_name);
}
zlog_info(log_get_cat(),"Testscompleted.Resultsin'%s'directory\n",
main_config_get()->test_name);

main_process_stop_test();

protocol_shutdown();

/*Othercleanups*/
main_config_cleanup();

/*Dopropercleanup*/
main_process_output_cleanup();

zlog_info(log_get_cat(),"Byebye");
zlog_fini();
if(cli_mode){
printw("Pressanycharactertoexit\n");
getch();
endwin();
}
}

void
main_process_sigint_handler(intsignum)
{
zlog_info(log_get_cat(),"Gotsigint");
if(main_config_is_cli_mode()){
endwin();
fprintf(stdout,"Stoppingtest..Pleasewait\n");
fflush(stdout);
}
g_main_running=0;
}

void
main_process_wait_for_completion()
{
pthread_join(g_main_output_process->maint_thread,NULL);
zlog_info(log_get_cat_http(),"Mainteancethreadstopped");
}

staticint
main_process_output_timer_update()
{
structtimevaltimeout;
timeout.tv_sec=1;
timeout.tv_usec=0;

evtimer_add(g_main_output_process->output_timer_event,&timeout);

return0;
}

staticvoid
main_process_run_output_thread(intfd,shortkind,void*userp)
{
if(g_main_running){
zlog_info(log_get_cat(),"Runningmainthread");

/*SendAlivecommand*/
if(!main_config_is_cli_mode()&&main_process_get_output_process()
&&main_process_get_output_process()->redis_ctx){
zlog_info(log_get_cat(),"SendingAlivecommand");
redisCommand(main_process_get_output_process()->redis_ctx,
"PUBLISH%s%s",
main_config_get()->redis_channel,"KEEPALIVE");
}

protocol_display_output();
main_process_output_timer_update();
}else{
if(!main_config_is_cli_mode()&&main_process_get_output_process()
&&main_process_get_output_process()->redis_ctx){
zlog_info(log_get_cat(),"SendingCompletedcommand");
redisCommand(main_process_get_output_process()->redis_ctx,
"PUBLISH%s%s",
main_config_get()->redis_channel,"DONE");
}
zlog_info(log_get_cat(),"Shuttingdown");
main_process_shutdown();
}
}

void*
main_process_output_thread(void*nothing)
{
main_process_run_output_thread(0,0,NULL);
event_base_dispatch(g_main_output_process->evbase);
returnNULL;
}

/*Calledwhenallthetestsaredone*/
void
main_process_tests_completed()
{
g_main_running=0;
}

int
main_process_start_output()
{
interror=0;

g_main_output_process=(main_process_output_t*)calloc(1,sizeof(
main_process_output_t));

if(!g_main_output_process){
return-1;
}

if(!main_config_is_cli_mode()){
error=main_process_connect_with_redis();
if(error){
return-1;
}
}

g_main_output_process->evbase=event_base_new();

evthread_use_pthreads();
if(evthread_make_base_notifiable(g_main_output_process->evbase)<0){
//common_gui_exit_with_error("Failedtostartoutputthread");
}

g_main_running=1;
g_main_output_process->output_timer_event=evtimer_new(
g_main_output_process->evbase,
main_process_run_output_thread,NULL);

error=pthread_create(&g_main_output_process->maint_thread,NULL,
main_process_output_thread,NULL);
returnerror;
}

void
main_process_display_help()
{

fprintf(stderr,"Usage:nixia-f<configurationfilename>\n");
fprintf(stderr,"Note,torunyourtest,"
"createyourconfiguration.jsonfile.\n");
fprintf(stderr,"\n");
exit(1);
}

void
main_process_config_user_input_cleanup(main_config_user_input_t*user_input)
{
if(!user_input){
return;
}

if(user_input->config_file){
free(user_input->config_file);
}
free(user_input);
}

main_config_user_input_t*
main_process_parse_command_line(intargc,char**argv)
{
intrget_opt=0;
main_config_user_input_t*user_input=NULL;

if(argc<2){
main_process_display_help();
/*Notneeded*/
returnNULL;
}

if(!(user_input=(main_config_user_input_t*)calloc(1,
sizeof(main_config_user_input_t)))){
returnNULL;
}

while((rget_opt=getopt(argc,argv,"f:g"))!=-1){
switch(rget_opt){
case'f':
if(optarg){
user_input->config_file=strdup(optarg);
}else{
main_process_config_user_input_cleanup(user_input);
main_process_display_help();
}
break;
case'g':
user_input->gui_mode=1;
break;
default:
main_process_config_user_input_cleanup(user_input);
main_process_display_help();
}
}

if(!user_input->config_file){
main_process_config_user_input_cleanup(user_input);
main_process_display_help();
}

returnuser_input;
}

int
main_process_init(intmode,char*json_data)
{
interror=0;

/*Initializemainconfiguration*/
if((error=main_config_init(mode,json_data))){
returnerror;
}

/*Initializeprotocoltableandprotocolconfiguration*/
if((error=protocol_table_init(main_config_get()->protocols,
json_data))){
/*Unrecoverableerror.Cleanupeverything*/
/*TODO:Cleanupsincethiswillexitbyleakingmemory*/
g_main_running=0;
return-1;
}

return0;
}
