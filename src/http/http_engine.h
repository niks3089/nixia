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

#ifndef__HTTP_ENGINE_H
#define__HTTP_ENGINE_H

#include<pthread.h>
#include<event2/event.h>

#include"http_stats.h"
#include"http_connection.h"

#defineMAX_WORKERS16

typedefstructhttp_engine_workers_s{
pthread_tworker;
structevent*user_event;
http_connection_pool_t*connection_pool;
http_stats_base_trunning_stats;
}http_engine_workers_t;

typedefstructhttp_engine_s{
FILE*test_summary_file;
structevent_base*evbase;
structevent*activity_timer_event;
intworkers_running;
inttotal_workers;
pthread_mutex_tlock;
pthread_tactivity_thread;
http_stats_base_ttotal_worker_stats;
http_engine_workers_tworkers[MAX_WORKERS];
}http_engine_t;

inthttp_engine_start(intworkers);
voidhttp_engine_stop();
voidhttp_engine_shutdown();
voidhttp_engine_setup_user_event(structevent*event,pthread_tid);
voidhttp_engine_remove_user_event(structevent*event,pthread_tid);
voidhttp_engine_update_worker_stats(http_stats_base_tstats,pthread_tid);
voidhttp_engine_display_output();
voidhttp_engine_update_connection_pool(http_connection_pool_t*pool,pthread_tid);
voidhttp_engine_copy_connection_pool(http_connection_pool_t*pool,
uint32_tsize,pthread_tid);

http_stats_base_t*http_engine_worker_stats();
#endif/*__HTTP_ENGINE_H*/
