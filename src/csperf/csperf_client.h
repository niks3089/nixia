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

#ifndef__CS_PERF_CLIENT_RUN_H
#define__CS_PERF_CLIENT_RUN_H

#include<event2/listener.h>
#include<event2/bufferevent.h>

#include"csperf_config.h"
#include"csperf_network.h"
#include"csperf_stats.h"

enum{
CLIENT_INIT=0,
CLIENT_CONNECTED,
}csperf_client_states;

typedefstructcsperf_client_s
{
uint8_ttransfer_flags;
intstate;
intrepeat_count;
FILE*output_file;
structevent_base*evbase;
structevent*second_timer;
structbufferevent*buff_event;
asn_message_pdu*data_pdu;
csperf_config_t*config;
asn_message_pdu*command_pdu_table[CS_CMD_MAX];
csperf_stats_tstats;
}csperf_client_t;

intcsperf_client_run();
#endif/*__CS_PERF_CLIENT_RUN_H*/
