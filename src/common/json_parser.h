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

#ifndef__JSON_PARSER_H
#define__JSON_PARSER_H

#include"cJSON.h"

char*json_parser_parse_file(char*file);
voidjson_parser_get_element_double(cJSON*root,char*table_name,char*element,double*val,
doubleval_default);
voidjson_parser_get_element_str(cJSON*root,char*table_name,char*element,char**val,
char*val_default);
voidjson_parser_get_array_str(cJSON*root,char*table_name,
char*array_name,char**val);
#endif/*__JSON_PARSER_H*/
