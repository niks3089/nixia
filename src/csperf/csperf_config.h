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

#ifndef__CS_PERF_CONFIG_H
#define__CS_PERF_CONFIG_H

#include<stdint.h>
#include"json_parser.h"

typedefstructcsperf_config_s{
uint8_trole;/*Clientorserver*/
uint8_tserver_echo;
uint16_tserver_port;
uint16_tmark_interval;
uint32_tdata_block_size;/*Blocksizeofeachdatasegment*/
uint32_ttotal_data_blocks;/*Totalblockstobesent*/
uint16_tclient_runtime;/*Totaldurationofthetest.-t*/
uint64_tdata_size;/*Totalsizeofdatatosend*/
intrepeat_count;
char*server_ip;
char*output_directory;
char*client_output_file;
char*server_output_file;
}csperf_config_t;

intcsperf_config_init(cJSON*root);
voidcsperf_config_cleanup(csperf_config_t*config);
csperf_config_t*csperf_config_get();
#endif/*__CS_PERF_CONFIG_H*/
#
