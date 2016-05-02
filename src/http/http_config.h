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

#ifndef__HTTP_CONFIG_H
#define__HTTP_CONFIG_H

#include"common.h"
#include"json_parser.h"

/*TODO:Weshouldmakeallofthemconst*/
typedefstructhttp_config_s{
char*output_directory;
char*url;
char*url_path;
char*http_version;
uint32_ttotal_connections;
uint32_tconcurrency;
uint32_tconnections_per_second;
uint32_ttotal_requests;
uint32_trequests_per_second;
uint32_trequests_per_connections;
uint8_tis_https;
uint8_tpipeline_requests;
uint8_tdivide_requests_equally;
}http_config_t;

inthttp_config_init(cJSON*root);
voidhttp_config_cleanup();
http_config_t*http_config_get();
voidhttp_config_dump();
inthttp_config_strict_validate(char*message);

#endif/*__HTTP_CONFIG_H*/
