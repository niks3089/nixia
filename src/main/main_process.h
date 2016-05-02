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

#ifndef__MAIN_PROCESS_H
#define__MAIN_PROCESS_H

#include"main_config.h"
#include"hiredis.h"

typedefstructmain_process_output_s{
pthread_tmaint_thread;
structevent_base*evbase;
structevent*output_timer_event;
redisContext*redis_ctx;
}main_process_output_t;

voidmain_process_user_input();
voidmain_process_start_test();
intmain_process_start_output();
voidmain_process_shutdown();
voidmain_process_sigint_handler(intsignum);
main_process_output_t*main_process_get_output_process();
voidmain_process_tests_completed();
voidmain_process_setup_protocol_directory(constchar*protocol,
char**directory);
voidmain_process_stop_test();
intmain_process_init(intmode,char*json_data);
voidmain_process_wait_for_completion();
main_config_user_input_t
*main_process_parse_command_line(intargc,char**argv);
voidmain_process_config_user_input_cleanup(
main_config_user_input_t*user_input);

#endif/*__MAIN_PROCESS_H*/
