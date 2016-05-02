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

#ifndef__MAIN_CONFIG_H
#define__MAIN_CONFIG_H

#include"common.h"

typedefstructmain_config_user_input_s{
char*config_file;
intgui_mode;
}main_config_user_input_t;

typedefstructmain_config_s{
char*test_name;
char*redis_channel;
char*redis_server;
char*interface;
char*protocols;
char*config_directory;
char*json_config;
uint16_tredis_port;
uint16_ttotal_run_time;
uint16_trepeat_count;
uint8_tcli_mode;
}main_config_t;

main_config_t*main_config_get();
intmain_config_init(intmode,char*json_data);
voidmain_config_cleanup();
intmain_config_is_cli_mode();

#endif/*__MAIN_CONFIG_H*/
