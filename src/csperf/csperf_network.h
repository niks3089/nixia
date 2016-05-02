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

#ifndef__CS_PERF_NETWORK_H
#define__CS_PERF_NETWORK_H

#include<stdint.h>
#include<event2/buffer.h>

#defineCS_MAGIC0xaa
#defineCS_HEADER_PDU_LEN(sizeof(asn_message_pdu))
#defineCS_COMMAND_PDU_LEN(sizeof(asn_command_pdu))

/*Flags*/
#defineCS_FLAG_DUPLEX0x01
#defineCS_FLAG_HALF_DUPLEX0x02

/*Eachpacketthatgetssentoutcanbeeither
*adataoracommand*/
enumasn_message_type{
CS_MSG_COMMAND=1,
CS_MSG_DATA
};

/*Mark:Sentfromclient->server.
Askstheservertorespondbackafterspecifedamount
ofbytes.

Markresponse:Sentfromserver->client.
Aftertheserverhasdoneprocessingdata
*/
enumasn_command_type{
CS_CMD_MARK,
CS_CMD_MARK_RESP,

/*Placenewcommandsabovethis*/
CS_CMD_MAX,
};

typedefstruct
{
/*Header*/
uint32_ttotal_len;
uint8_tmagic;
uint8_tmessage_type;
uint8_treserved[2];

/*Payload*/
uint8_tmessage[];
}asn_message_pdu;

typedefstruct
{
uint64_techo_timestamp;
uint64_techoreply_timestamp;
uint32_tblocks_to_receive;
uint8_tcommand_type;
uint8_tflags;
uint8_tresvrd[2];
}asn_command_pdu;

intcsperf_network_get_pdu_type(structevbuffer*buf,uint32_t*len);
asn_message_pdu*csperf_network_create_pdu(uint8_tmessage_type,
uint8_tmessage_info,uint32_tmessage_len);
uint64_tcsperf_network_get_time(char*buf);
#endif/*__CS_PERF_NETWORK_H*/
