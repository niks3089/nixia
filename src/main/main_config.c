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

#include"main_config.h"
#include"main_defaults.h"
#include"log.h"
#include"common.h"
#include"json_parser.h"

staticmain_config_t*g_main_config=NULL;

main_config_t*
main_config_get()
{
returng_main_config;
}

void
main_config_cleanup()
{
if(!g_main_config){
return;
}

if(g_main_config->json_config){
free(g_main_config->json_config);
}

if(g_main_config->interface){
free(g_main_config->interface);
}

if(g_main_config->test_name){
free(g_main_config->test_name);
}

if(g_main_config->redis_channel){
free(g_main_config->redis_channel);
}

if(g_main_config->redis_server){
free(g_main_config->redis_server);
}

if(g_main_config->protocols){
free(g_main_config->protocols);
}

if(g_main_config->config_directory){
free(g_main_config->config_directory);
}

free(g_main_config);
g_main_config=NULL;
}

void
main_config_dump()
{
zlog_info(log_get_cat(),"Mainconfiguration:");
zlog_info(log_get_cat(),"total_run_time:%u",g_main_config->total_run_time);
zlog_info(log_get_cat(),"repeat_count:%u",g_main_config->repeat_count);
zlog_info(log_get_cat(),"testname:%s",g_main_config->test_name);
zlog_info(log_get_cat(),"protocols:%s",g_main_config->protocols);
zlog_info(log_get_cat(),"interface:%s",g_main_config->interface);
zlog_info(log_get_cat(),"redischannel:%s",g_main_config->redis_channel);
zlog_info(log_get_cat(),"redisserver:%s",g_main_config->redis_server);
zlog_info(log_get_cat(),"redisport:%u",g_main_config->redis_port);
zlog_info(log_get_cat(),"---------------------");
}

int
main_config_is_cli_mode()
{
returng_main_config?g_main_config->cli_mode:1;
}

/*Initializethemainconfiguration.
*ThiswillbeusedasFYIbyotherprotocols*/
int
main_config_init(intmode,char*json_data)
{
doublenew_val=0;
cJSON*root;

if(!(root=cJSON_Parse(json_data))){
zlog_error(log_get_cat(),"Failedtovalidateconfig:%s",
cJSON_GetErrorPtr());
return-1;
}

g_main_config=(main_config_t*)calloc(1,sizeof(main_config_t));

if(!g_main_config){
zlog_error(log_get_cat(),"Failedtoallocatememoryformainconfig");
return-1;
}
g_main_config->cli_mode=mode;
g_main_config->json_config=json_data;

/*Gettheconfigurationfromjson*/

/*Getstringtypes*/
json_parser_get_element_str(root,"main","name",
&g_main_config->test_name,DEFAULT_TEST_NAME);
json_parser_get_element_str(root,"main","redis_channel",
&g_main_config->redis_channel,NULL);
json_parser_get_element_str(root,"main","redis_server",
&g_main_config->redis_server,NULL);
json_parser_get_element_str(root,"main","interface",
&g_main_config->interface,NULL);

json_parser_get_array_str(root,"main","protocols",
&g_main_config->protocols);

/*Getintegertypes*/
json_parser_get_element_double(root,"main","redis_port",
&new_val,0);
g_main_config->redis_port=new_val;
json_parser_get_element_double(root,"main","total_run_time",
&new_val,MAIN_DEFAULT_TOTAL_RUN_TIME);
g_main_config->total_run_time=new_val;
json_parser_get_element_double(root,"main","repeat_count",
&new_val,MAIN_DEFAULT_REPEAT_COUNT);
g_main_config->repeat_count=new_val;

main_config_dump();
zlog_info(log_get_cat(),"Initializedmainconfiguration");
return0;
}
