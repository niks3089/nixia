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

#ifndef__PROTOCOL_H
#define__PROTOCOL_H

#include<stdint.h>
#include<stdio.h>

#include"json_parser.h"

/*Alltheprotocolsneedstohavefunctionsoftypeshownbelow.*/
typedefint(*protocol_config_init_t)(cJSON*root);
typedefvoid(*protocol_start_test_t)(char*output_directory);
typedefvoid(*protocol_display_output_t)();
typedefvoid(*protocol_stop_test_t)();
typedefvoid(*protocol_shutdown_t)();

typedefenum{
PROTO_HTTP=0,
PROTO_CSPERF,

/*Addnewprotocolsabovethis*/
PROTO_MAX,
}protocols;

intprotocol_table_init(char*protocol_list,char*json_data);
voidprotocol_start_test();
voidprotocol_stop_test();
voidprotocol_shutdown();
voidprotocol_display_output();
voidprotocol_announce_test_is_completed(int);
char*protocol_get_filename(char*directory,char*filename);
FILE*protocol_create_file(char*directory,char*filename);
voidprotocol_load_main_page();
voidprotocol_helper_display_result(char*output);

#endif/*__PROTOCOL_H*/
