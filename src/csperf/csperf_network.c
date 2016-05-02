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

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<assert.h>
#include<sys/time.h>
#include<time.h>

#include"csperf_network.h"

/*Createasnmessagewithheader.
*message_infoargumentisusedifthe
*messagetypeiscommand*/
asn_message_pdu*
csperf_network_create_pdu(uint8_tmessage_type,
uint8_tmessage_info,uint32_tmessage_len)
{
inti;
asn_message_pdu*header=NULL;
asn_command_pducommand;

/*Ifthemessage_lenishuge(>1GB),wemightneed
*tousemmap*/
header=calloc(1,message_len+CS_HEADER_PDU_LEN);

if(!header){
returnNULL;
}

header->total_len=CS_HEADER_PDU_LEN+message_len;
header->magic=CS_MAGIC;
header->message_type=message_type;

if(message_type==CS_MSG_DATA){
/*Randomizethedata*/
for(i=0;i<message_len;i++){
header->message[i]=rand();
}
}else{
memset(&command,0,sizeof(asn_command_pdu));
command.command_type=message_info;

/*Copythecommandasthemessagepayload*/
memcpy(header->message,&command,message_len);
}
returnheader;
}

/*Usuallycalledwhenthepduisreadfromthesocket.
Returnswhetherpduiscommandordata.
Ifthecompletemessageisnotreceived,returns0*/
int
csperf_network_get_pdu_type(structevbuffer*buf,uint32_t*len)
{
asn_message_pduheader;
uint32_ttotal_len;
size_tbuffer_len=evbuffer_get_length(buf);

if(buffer_len<CS_HEADER_PDU_LEN){
/*Thesizefieldhasn'tarrived.*/
return0;
}

/*Weuseevbuffer_copyoutheresothatmessgaewillstay
inthebufferfornow.*/
evbuffer_copyout(buf,&header,CS_HEADER_PDU_LEN);
total_len=header.total_len;

if(buffer_len<total_len){
/*Thepduhasn'tarrived*/
return0;
}
*len=header.total_len;

/*Checkifitisdataorcommand*/
if(header.message_type==CS_MSG_DATA){
returnCS_MSG_DATA;
}

if(header.message_type==CS_MSG_COMMAND){
returnCS_MSG_COMMAND;
}

/*Error!*/
return-1;
}

/*Gettimeinmillisecond*/
uint64_t
csperf_network_get_time(char*buf)
{
charfmt[64];
structtm*tm;
structtimespectv;
uint64_ts;

clock_gettime(CLOCK_MONOTONIC,&tv);
if(buf){
if((tm=localtime(&tv.tv_sec))!=NULL){
strftime(fmt,sizeof(fmt),"%Y-%m-%d%H:%M:%S.%03u",tm);
snprintf(buf,sizeof(fmt),fmt,tv.tv_nsec/1000LL);
}
}
s=tv.tv_sec*1000LL;
return(s+tv.tv_nsec/1000000LL);
}
