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

#include<stdint.h>
#include<string.h>

#include"protocol.h"
#include"common.h"
#include"log.h"

#include"http_defaults.h"
#include"http_process.h"
#include"http_engine.h"
#include"http_config.h"

int
http_process_get_worker_threads(http_config_t*config)
{
return1;
}

void
http_process_stop()
{
/*Stopthethreads*/
/*Collectstatsandinformation*/
http_engine_stop();
}

void
http_process_shutdown()
{
/*Releasetheresources*/
zlog_info(log_get_cat_http(),"ShuttingdownHTTP");
http_engine_shutdown();
http_config_cleanup();
}

void
http_process_start(char*directory)
{
intworkers,error;
http_config_t*config=http_config_get();

if(!config||!directory){
return;
}

config->output_directory=directory;

workers=http_process_get_worker_threads(http_config_get());

/*Startourengine*/
if((error=http_engine_start(workers))){
/*TODO:Thisexitwillnotworkifotherprotocolsarerunning*/
//common_gui_exit_with_error("FailedtostartHTTPengine");
}
}

void
http_process_display_output()
{
http_engine_display_output();
}

void
http_process_test_completed()
{
protocol_announce_test_is_completed(PROTO_HTTP);
}
