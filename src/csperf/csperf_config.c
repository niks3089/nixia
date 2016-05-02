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
#include<stdio.h>
#include<assert.h>

#include"csperf_config.h"
#include"csperf_defaults.h"
#include"csperf_network.h"
#include"log.h"
#include"common.h"

staticcsperf_config_t*g_csperf_config=NULL;

csperf_config_t*
csperf_config_get()
{
returng_csperf_config;
}

void
csperf_config_dump()
{
zlog_info(log_get_cat(),"-----------------------------");
zlog_info(log_get_cat(),"CSperfConfiguration");
zlog_info(log_get_cat(),"-----------------------------");
zlog_info(log_get_cat(),"Role:%d",g_csperf_config->role);
zlog_info(log_get_cat(),"Serverip:%s",g_csperf_config->server_ip);
zlog_info(log_get_cat(),"Serverport:%u",g_csperf_config->server_port);
zlog_info(log_get_cat(),"Serverecho:%d",g_csperf_config->server_echo);
zlog_info(log_get_cat(),"Datablocksize:%u",g_csperf_config->data_block_size);
zlog_info(log_get_cat(),"Totaldatablocks:%u",g_csperf_config->total_data_blocks);
zlog_info(log_get_cat(),"Repeatcount:%d",g_csperf_config->repeat_count);
zlog_info(log_get_cat(),"--------------------");
}

void
csperf_config_cleanup(csperf_config_t*config)
{
if(!config){
return;
}
if(config->server_ip){
free(config->server_ip);
}

if(config->output_directory){
free(config->output_directory);
}

if(config->client_output_file){
free(config->client_output_file);
}
if(config->server_output_file){
free(config->server_output_file);
}
free(config);
config=NULL;
}

int
csperf_config_init(cJSON*root)
{
doublenew_val=0;
char*cs_type;

if(!root){
zlog_error(log_get_cat(),"Nojsontoinitconfig");
return-1;
}

if(!g_csperf_config){
g_csperf_config=(csperf_config_t*)calloc(1,sizeof(csperf_config_t));

if(!g_csperf_config){
return-1;
}
}else{
memset(g_csperf_config,0,sizeof(csperf_config_t));
}

/*Getconfigurationfromjsonconfig*/
json_parser_get_element_str(root,"csperf","cs_type",
&cs_type,NULL);

if(strcmp(cs_type,"client")==0){
g_csperf_config->role=CS_CLIENT;
}else{
g_csperf_config->role=CS_SERVER;
}
free(cs_type);
json_parser_get_element_str(root,"csperf","server_ip",
&g_csperf_config->server_ip,NULL);

json_parser_get_element_double(root,"csperf","server_port",
&new_val,CSPERF_DEFAULT_SERVER_PORT);
g_csperf_config->server_port=new_val;

json_parser_get_element_double(root,"csperf","data_block_size",
&new_val,CSPERF_DEFAULT_DATA_BLOCKLEN);
g_csperf_config->data_block_size=new_val;

json_parser_get_element_double(root,"csperf","num_blocks",
&new_val,CSPERF_DEFAULT_DATA_BLOCKS);
g_csperf_config->total_data_blocks=new_val;

json_parser_get_element_double(root,"csperf","repeat_count",
&new_val,1);
g_csperf_config->repeat_count=new_val;

json_parser_get_element_double(root,"csperf","server_echo",
&new_val,0);
g_csperf_config->server_echo=new_val;
csperf_config_dump();
zlog_info(log_get_cat(),"InitializedCSperfconfiguration\n");
return0;
}
