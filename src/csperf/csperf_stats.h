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

#ifndef__CS_PERF_STATS_H
#define__CS_PERF_STATS_H

#include<stdint.h>
typedefstruct{
uint64_ttotal_bytes_sent;
uint64_ttotal_bytes_received;
uint64_ttotal_blocks_sent;
uint64_ttotal_blocks_received;
uint64_ttotal_commands_sent;
uint64_ttotal_commands_received;
uint64_ttime_to_process_data;
charmark_sent_time[100];
charmark_received_time[100];
}csperf_stats_t;

voidansperf_stats_display(csperf_stats_t*stats,FILE*fd);
#endif/*__CS_PERF_STATS_H*/
