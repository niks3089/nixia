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

#ifndef__CSPERF_PROCESS_H
#define__CSPERF_PROCESS_H

#include<pthread.h>
#include<event2/event.h>

typedefstructcsperf_process_s{
FILE*test_summary_file;
structevent_base*evbase;
structevent*activity_timer_event;
pthread_mutex_tlock;
pthread_tactivity_thread;
//csperf_stats_base_ttotal_worker_stats;
}csperf_process_t;

voidcsperf_process_start(char*directory);
voidcsperf_process_display_output();
voidcsperf_process_stop();
voidcsperf_process_shutdown();
#endif/*__CSPERF_PROCESS_H*/
