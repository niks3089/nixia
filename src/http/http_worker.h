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

#ifndef__HTTP_WORKER_H
#define__HTTP_WORKER_H

#include<event2/event.h>

#include"http_connection.h"
#include"http_config.h"

/*TODO:Thisisokaybutmakesurewedividethework*/
typedefhttp_config_thttp_worker_test_config_t;

/*Perthreadconfiguration*/
structhttp_worker_base_t{
structevent_base*evbase;
structevent*per_second_timer_event;
structevent*user_event;
http_worker_test_config_t*config;
http_connection_pool_t*connection_pool;
FILE*worker_summary_file;
FILE*connection_summary_file;

uint32_tworker_id;
intrunning_connections;
charstatistics[MAX_NAME_LENGTH];

/*Statsperthread*/
http_stats_base_tstats;
};

void*http_worker_thread(void*data);
voidhttp_worker_stop_test_if_completed(structhttp_worker_base_t*base);
inthttp_worker_is_test_completed(structhttp_worker_base_t*base);
voidhttp_worker_stop_test(structhttp_worker_base_t*base);
inthttp_worker_finished_sending_requests(structhttp_worker_base_t*base);
#endif/*__HTTP_WORKER_H*/
