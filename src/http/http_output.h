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

#include"http_worker.h"
#include"http_stats.h"
#include"http_connection.h"
voidhttp_output_worker_summary_to_file(http_stats_base_t*stats,FILE*fp);
char*http_output_worker_summary_to_string(http_stats_base_t*stats);

voidhttp_output_connection_summary_to_file(structhttp_worker_base_t*base);
char*http_output_connection_summary_to_string(http_connection_pool_t*pool);
voidhttp_output_test_config_to_string(char**output_str);
