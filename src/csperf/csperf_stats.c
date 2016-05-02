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
#include<stdarg.h>

#include"csperf_stats.h"
#include"csperf_common.h"

constcharheader[]=
"CycleBytes_sentBytes_ReceivedBlocks_sentBlocks_received"
"Time_to_process(ms)Mark_sent_timeMark_received_time\n";

constcharseperator_line[]=
"------------------------------------------------------------------------"
"------------------------------------------------------------\n";

void
csperf_stats_printf(FILE*fd,constchar*format,...)
{
/*Writetofile*/
va_listargs;

va_start(args,format);
if(fd){
vfprintf(fd,format,args);
}
va_end(args);

/*Writetostdout*/
va_start(args,format);
vfprintf(stdout,format,args);
va_end(args);
}

void
ansperf_stats_display(csperf_stats_t*stats,FILE*fd)
{
staticintheader_displayed=0;
staticintcycle=0;
chartotal_bytes_sent_str[50];
chartotal_bytes_recv_str[50];

if(!stats){
return;
}

if(!header_displayed){
csperf_stats_printf(fd,"%s%s",header,seperator_line);
header_displayed=1;
}

csperf_common_calculate_size(total_bytes_sent_str,
stats->total_bytes_sent);
csperf_common_calculate_size(total_bytes_recv_str,
stats->total_bytes_received);

csperf_stats_printf(fd,"%3d%15s%10s%10zu%10zu%10zu%10s"
"%10s\n\n",++cycle,
total_bytes_sent_str,total_bytes_recv_str,
stats->total_blocks_sent,stats->total_blocks_received,
stats->time_to_process_data,
stats->mark_sent_time,stats->mark_received_time);
}
