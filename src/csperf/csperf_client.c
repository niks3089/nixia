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
#include<errno.h>
#include<arpa/inet.h>
#include<assert.h>
#include<unistd.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>

#include"log.h"
#include"csperf_client.h"
#include"csperf_network.h"
#include"csperf_common.h"

/*Shutdownandclosetheclient*/
staticvoid
csperf_client_shutdown(csperf_client_t*client)
{
inti;

if(!client){
return;
}

zlog_info(log_get_cat_csperf(),"Shutdownclient\n");

/*Printstatsonlyiftheclientisconnectedtotheserver*/
if(client->state>=CLIENT_CONNECTED){
ansperf_stats_display(&client->stats,client->output_file);
}

if(client->buff_event){
bufferevent_free(client->buff_event);
}

if(client->second_timer){
event_free(client->second_timer);
}
csperf_config_cleanup(client->config);

if(client->command_pdu_table){
for(i=0;i<CS_CMD_MAX;i++){
free(client->command_pdu_table[i]);
}
}

if(client->data_pdu){
free(client->data_pdu);
}

if(client->output_file){
fclose(client->output_file);
}
event_base_loopbreak(client->evbase);
event_base_free(client->evbase);
free(client);
}

/*Updatetimer.Itticksevery1second*/
staticint
csperf_client_timer_update(csperf_client_t*client)
{
structtimevaltimeout;
timeout.tv_sec=1;
timeout.tv_usec=0;

evtimer_add(client->second_timer,&timeout);

return0;
}

/*Calledwhenthetimeouthappens.
*Wedon'tdoanythingusefulhere.yet..*/
staticvoid
csperf_client_timer_cb(intfd,shortkind,void*userp)
{
staticuint16_ttimer=0;

csperf_client_t*client=(csperf_client_t*)userp;

/*Checkifweneedtostop*/
if((client->config->client_runtime)&&
(++timer>=client->config->client_runtime)){
zlog_info(log_get_cat_csperf(),"Client:Timeout!\n");
csperf_client_shutdown(client);
return;
}

/*TODO:Displaycurrentstats*/
csperf_client_timer_update(client);
}

/*Initclientsubsystem*/
staticcsperf_client_t*
csperf_client_init(csperf_config_t*config)
{
inti;
csperf_client_t*client;

client=(csperf_client_t*)calloc(1,sizeof(csperf_client_t));
if(!client){
returnNULL;
}

if(!(client->evbase=event_base_new())){
free(client);
returnNULL;
}

/*Createdatapdu.Wedothisonceandkeepsendingthe
*samedataoverandoveragain*/
if(!(client->data_pdu=
csperf_network_create_pdu(CS_MSG_DATA,0,
config->data_block_size))){
free(client);
returnNULL;
}

/*Setupallcommands.Wealsodothisonce.
*Noteverythingwesetupmightbeused*/
for(i=0;i<CS_CMD_MAX;i++){
if(!(client->command_pdu_table[i]=
csperf_network_create_pdu(CS_MSG_COMMAND,i,
CS_COMMAND_PDU_LEN))){
free(client);
returnNULL;
}
}

if(config->client_output_file){
if(!(client->output_file=fopen(config->client_output_file,"w"))){
zlog_warn(log_get_cat_csperf(),"Failedtocreate%sfile\n",config->client_output_file);
free(client);
returnNULL;
}
}
client->repeat_count=1;
client->config=config;
returnclient;
}

/*Sendthiscommandtoasktheservertosendbacktheresponse
*afterprocessingtotal_data_blocks.Wethencalculatethertttomeasure
*thetimetakentoprocesstotal_data_blocks*/
staticint
csperf_client_send_mark_command(csperf_client_t*client,uint8_tflags)
{
asn_command_pdu*command;

command=(asn_command_pdu*)(&client->
command_pdu_table[CS_CMD_MARK]->message);

command->blocks_to_receive=client->config->total_data_blocks;
command->echo_timestamp=csperf_network_get_time(
client->stats.mark_sent_time);

client->transfer_flags=command->flags=flags;
client->stats.total_commands_sent++;

returnbufferevent_write(client->buff_event,
client->command_pdu_table[CS_CMD_MARK],
CS_HEADER_PDU_LEN+CS_COMMAND_PDU_LEN);
}

/*Senddatamessagetotheserver*/
staticint
csperf_client_send_data(csperf_client_t*client)
{
/*Writetosocket*/
interror;

if(client->config->data_size&&
client->stats.total_bytes_sent>=client->config->data_size){
return0;
}

if(client->stats.total_blocks_sent>=
client->config->total_data_blocks){
return0;
}

#if0
error=write(bufferevent_getfd(client->buff_event),
client->data_pdu,client->config->data_block_size+CS_HEADER_PDU_LEN);
zlog_info(log_get_cat_csperf(),"Byteswritten:%d\n",error);
#endif

error=bufferevent_write(client->buff_event,client->data_pdu,
client->config->data_block_size+CS_HEADER_PDU_LEN);

if(error){
assert(0);
returnerror;
}
client->stats.total_bytes_sent+=client->config->data_block_size+
CS_HEADER_PDU_LEN;
client->stats.total_blocks_sent++;
return0;
}

/*Startthetransfer.
*Firstsendmarkcommandfollowedbydata*/
staticvoid
csperf_client_start(csperf_client_t*client)
{
if(csperf_client_send_mark_command(client,
client->config->server_echo)<0){
zlog_warn(log_get_cat_csperf(),"Errorwritingcommand");
}
if(csperf_client_send_data(client)<0){
zlog_warn(log_get_cat_csperf(),"Writeerror\n");
}
}

/*Receivedata.Fornow,justdiscardit*/
staticint
csperf_client_process_data(csperf_client_t*client,structevbuffer*buf,
uint32_tlen)
{
/*Shouldnotreceivedatainhalfduplexmode*/
assert(client->transfer_flags==CS_FLAG_DUPLEX);

/*Silentdraindata*/
client->stats.total_bytes_received+=len;
client->stats.total_blocks_received++;
evbuffer_drain(buf,len);
return0;
}

/*Processthecommand*/
staticint
csperf_client_process_command(csperf_client_t*client,structevbuffer*buf)
{
asn_command_pducommand={0};

/*Removeheader*/
evbuffer_drain(buf,CS_HEADER_PDU_LEN);

evbuffer_remove(buf,&command,CS_COMMAND_PDU_LEN);
client->stats.total_commands_received++;

switch(command.command_type){
caseCS_CMD_MARK_RESP:
/*Calculatethetimetakentoprocessallthedata*/
client->stats.time_to_process_data=
csperf_network_get_time(client->stats.mark_received_time)-
command.echoreply_timestamp;

if(client->config->repeat_count>0&&
client->repeat_count>=client->config->repeat_count){
csperf_client_shutdown(client);
}else{
/*Comesherewhen-roptionisused.Runthetestagain*/
client->repeat_count++;
ansperf_stats_display(&client->stats,client->output_file);
memset(&client->stats,0,sizeof(client->stats));
/*startagain*/
csperf_client_start(client);
}
break;
default:
zlog_warn(log_get_cat_csperf(),"Unexpectedcommand\n");
return-1;
}
return0;
}

/*Invokedwhenwecanwritedatatobufferevent*/
staticvoid
csperf_client_write_cb(structbufferevent*bev,void*ptr)
{
csperf_client_t*client=(csperf_client_t*)ptr;
csperf_client_send_data(client);
}

/*Calledwhenthereisnewstufftoberead*/
staticvoid
csperf_client_readcb(structbufferevent*bev,void*ptr)
{
csperf_client_t*client=(csperf_client_t*)ptr;
structevbuffer*input_buf;
intmessage_type;
uint32_tlen=0;

do{
/*Getbufferfrominputqueue*/
input_buf=bufferevent_get_input(bev);
message_type=csperf_network_get_pdu_type(input_buf,&len);

/*Completepduhasn'tarrivedyet*/
if(!message_type){
break;
}

if(message_type==CS_MSG_DATA){
csperf_client_process_data(client,input_buf,len);
}elseif(message_type==CS_MSG_COMMAND){
/*Wegotacommandfromserver*/
csperf_client_process_command(client,input_buf);
}else{
assert(0);
}
}while(input_buf&&evbuffer_get_length(input_buf));
}

/*Calledwhenthereisaneventonthesocket*/
staticvoid
csperf_client_eventcb(structbufferevent*bev,shortevents,void*ctx)
{
csperf_client_t*client=ctx;
intfinished=0;

/*Connectedtotheserver*/
if(events&BEV_EVENT_CONNECTED){
client->state=CLIENT_CONNECTED;
zlog_info(log_get_cat_csperf(),"Clientconnectedtoserver..Pleasewait\n");

/*Startthetransfer*/
csperf_client_start(client);
}else{
if(events&BEV_EVENT_EOF){
finished=1;
}
if(events&BEV_EVENT_ERROR){
zlog_warn(log_get_cat_csperf(),"Error:%s\n",evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
finished=1;
}
if(finished){
csperf_client_shutdown(client);
}
}
}

/*Setupclient*/
staticint
csperf_client_configure(csperf_client_t*client)
{
/*Createabufferevent*/
client->buff_event=bufferevent_socket_new(client->evbase,-1,
BEV_OPT_CLOSE_ON_FREE);

/*Setreadwriteandeventcallbacksonthebufferevent*/
bufferevent_setcb(client->buff_event,csperf_client_readcb,
csperf_client_write_cb,csperf_client_eventcb,client);
bufferevent_enable(client->buff_event,EV_READ|EV_WRITE);

/*Setthelowwatermarktosizeoftheasnheader.
Thatwaywecanfigureoutifitisdataorcommand*/
bufferevent_setwatermark(client->buff_event,EV_READ,CS_HEADER_PDU_LEN,0);

/*Connecttothehostname*/
if(bufferevent_socket_connect_hostname(client->buff_event,
NULL,AF_INET,client->config->server_ip,
client->config->server_port)){
bufferevent_free(client->buff_event);
return-1;
}
zlog_info(log_get_cat_csperf(),"Client:Connectingto:%s:%d\n",
client->config->server_ip,client->config->server_port);
return0;
}

int
csperf_client_run(csperf_config_t*config)
{
interror=0;
csperf_client_t*client=NULL;

if(!(client=csperf_client_init(config))){
zlog_warn(log_get_cat_csperf(),"Failedtoinitclient\n");
return-1;
}

if((error=csperf_client_configure(client))){
csperf_client_shutdown(client);
returnerror;
}

client->second_timer=evtimer_new(client->evbase,
csperf_client_timer_cb,client);
csperf_client_timer_update(client);

/*Runtheeventloop.Connecttotheserver*/
event_base_dispatch(client->evbase);

return0;
}
