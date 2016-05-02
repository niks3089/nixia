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
#include<signal.h>
#include<unistd.h>
#include<hiredis.h>
#include<ncurses/ncurses.h>

#include"log.h"
#include"common.h"
#include"json_parser.h"
#include"main_process.h"
#include"main_config.h"

/*Placewheremagicstarts.
*InitializemodulesandfireupthemainUI*/
void
main_init(intargc,char**argv)
{
interror=0;
char*json_data=NULL;
main_config_user_input_t*user_input=NULL;

/*Initalizeourloggingmodule.*/
if((error=log_init())){
fprintf(stderr,"Failedtoinitloggingmodule.error:%d\n",error);
exit(1);
}
zlog_info(log_get_cat(),"Initializedloggingmodule");

signal(SIGINT,main_process_sigint_handler);

/*Parsethearguments*/
user_input=main_process_parse_command_line(argc,argv);

if(!user_input){
exit(1);
}

if(access(user_input->config_file,F_OK)<0){
zlog_error(log_get_cat(),"Configfiledoesn'texist.");
fprintf(stderr,"Configfiledoesn'texist.\n");
exit(1);
}

zlog_info(log_get_cat(),"Parsingfile:%s",user_input->config_file);
json_data=json_parser_parse_file(user_input->config_file);

if(!json_data){
zlog_error(log_get_cat(),"Failedtoloadconfiguration");
if(!user_input->gui_mode){
fprintf(stdout,"Failedtoloadconfiguration.\n");
}
exit(1);
}elseif(!cJSON_Parse(json_data)){
zlog_error(log_get_cat(),"Failedtovalidateconfiguration."
"Error:%s",cJSON_GetErrorPtr());
if(!user_input->gui_mode){
fprintf(stdout,"Failedtovalidateconfiguration.Error:%s\n",
cJSON_GetErrorPtr());
}
exit(1);
}

/*Initialmainprocessandprotocolconfiguration*/
if((error=main_process_init(!user_input->gui_mode,json_data))){
zlog_error(log_get_cat(),"Failedtoinitmainprocess");
main_process_config_user_input_cleanup(user_input);
main_process_shutdown();
exit(1);
}

if((error=main_process_start_output())){
zlog_error(log_get_cat(),"Failedtostartthemaintthread");
main_process_config_user_input_cleanup(user_input);
main_process_shutdown();
exit(1);
}

if(!user_input->gui_mode){
initscr();
mvprintw(0,0,"Runningtest.Pleasewait...\n");
refresh();
}

/*Cleanupuserinput.Wedon'tneeditanymore*/
main_process_config_user_input_cleanup(user_input);

/*Startthetest*/
main_process_start_test();

/*Testsarecompleted*/
main_process_wait_for_completion();
zlog_error(log_get_cat(),"Testscompleted");

/*TODO:wemightneedtomovewhereprogram
*actuallycloses*/
//zlog_fini();
}

intmain(intargc,char**argv)
{
main_init(argc,argv);
return0;
}
