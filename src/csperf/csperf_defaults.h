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

#ifndef__CS_PERF_CSPERF_DEFAULTS_H
#define__CS_PERF_CSPERF_DEFAULTS_H
enumasn_role{
CS_CLIENT,
CS_SERVER
};

/*Generalconstants*/
#defineCSPERF_DEFAULT_DATA_BLOCKLEN(1024)
#defineCSPERF_DEFAULT_SERVER_PORT5001
#defineCSPERF_DEFAULT_DATA_BLOCKS1000
#defineCSPERF_DEFAULT_CLIENT_OUTPUT_FILE"csperf_client_out.txt"
#defineCSPERF_DEFAULT_SERVER_OUTPUT_FILE"csperf_server_out.txt"

/*Maxconstants*/
#defineMAX_CLIENT_RUNTIME6000

#endif/*__CS_PERF_CSPERF_DEFAULTS_H*/
