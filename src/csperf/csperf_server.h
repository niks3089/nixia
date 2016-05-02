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

#ifndef__CS_PERF_SERVER_H
#define__CS_PERF_SERVER_H

#include<event2/listener.h>
#include<event2/bufferevent.h>

#include"csperf_config.h"
#include"csperf_network.h"
#include"csperf_stats.h"

typedefstructcsperf_server_s{
uint8_ttransfer_flags;
uint8_tshow_stats;
FILE*output_file;
structevent_base*evbase;
structbufferevent*buff_event;
structevent*second_timer;
csperf_config_t*config;
asn_message_pdu*command_pdu_table[CS_CMD_MAX];
uint64_tclient_last_received_timestamp;
csperf_stats_tstats;
}csperf_server_t;

intcsperf_server_run(csperf_config_t*config);
#endif/*__CS_PERF_SERVER_H*/
