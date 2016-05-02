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

#ifndef__COMMON_H
#define__COMMON_H

#include<stdint.h>

/*Donotincludeanyofourheaderfileshere!*/

#defineLOG_CONFIG_FILE"/etc/nixia/log.conf"
#defineLOG_CATEGORY"my_cat"
#defineHTTP_LOG_CATEGORY"http_cat"
#defineCSPERF_LOG_CATEGORY"csperf_cat"

/*XSDfilesafterinstallation*/
#defineCONFIG_XSD_FILE"/etc/nixia/config.xsd"

/*Somelengthstouse.*/
#defineMESSAGE_ERR_LENGTH1500
#defineMAX_MSG_LENGTH1500
#defineMAX_NAME_LENGTH100

/*Defaulttestnameanddirectory*/
#defineDEFAULT_TEST_NAME"output_test"
#endif/*__COMMON_H*/
