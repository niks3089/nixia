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

#include<inttypes.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#defineDIM(x)(sizeof(x)/sizeof(*(x)))

staticconstchar*sizes[]={"TB","GB","MB","KB","B"};
staticconstuint64_texbibytes=1024ULL*1024ULL*1024ULL;

void
csperf_common_calculate_size(char*result,uint64_tsize)
{
uint64_tmultiplier=exbibytes;
inti;

if(!size){
strcpy(result,"0B");
return;
}

for(i=0;i<DIM(sizes);i++,multiplier/=1024)
{
if(size<multiplier)
continue;
if(size%multiplier==0)
sprintf(result,"%"PRIu64"%s",size/multiplier,sizes[i]);
else
sprintf(result,"%.3f%s",(float)size/multiplier,sizes[i]);
return;
}
strcpy(result,"0B");
return;
}
