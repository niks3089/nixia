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

#include<pthread.h>

#include"csperf_process.h"
#include"csperf_defaults.h"
#include"csperf_config.h"
#include"csperf_client.h"
#include"csperf_server.h"

#include"log.h"
#include"protocol.h"

staticvoid*
csperf_process_worker(void*nothing)
{
csperf_config_t*config=csperf_config_get();

zlog_info(log_get_cat_csperf(),"Startedcsperfworker");

if(config->role==CS_CLIENT){
csperf_client_run(config);
}else{
csperf_server_run(config);
}
protocol_announce_test_is_completed(PROTO_CSPERF);
returnNULL;
}

int
csperf_process_start_worker_thread()
{
pthread_tworker_thread;
interror=0;

error=pthread_create(&worker_thread,NULL,
csperf_process_worker,NULL);

returnerror;
}

void
csperf_process_start(char*directory)
{
interror;
csperf_config_t*config=csperf_config_get();

config->output_directory=directory;

if((error=csperf_process_start_worker_thread())){
}
}

void
csperf_process_display_output()
{
}

void
csperf_process_stop()
{
}

void
csperf_process_shutdown()
{
}
