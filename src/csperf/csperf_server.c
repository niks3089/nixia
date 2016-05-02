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
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<assert.h>

#include"log.h"
#include"csperf_server.h"
#include"csperf_network.h"

/*Shutdownandclosetheclient*/
staticvoid
csperf_server_shutdown(csperf_server_t*server)
{
inti;

if(!server){
return;
}

zlog_info(log_get_cat_csperf(),"Shutdownserver\n");

if(server->show_stats){
ansperf_stats_display(&server->stats,server->output_file);
}
if(server->buff_event){
bufferevent_free(server->buff_event);
}
if(server->second_timer){
event_free(server->second_timer);
}
csperf_config_cleanup(server->config);

if(server->command_pdu_table){
for(i=0;i<CS_CMD_MAX;i++){
free(server->command_pdu_table[i]);
}
}

if(server->output_file){
fclose(server->output_file);
}
event_base_loopbreak(server->evbase);
event_base_free(server->evbase);
free(server);
}

/*Displaythestatsfirstandthenclearoutthestats*/
staticvoid
csperf_server_reset_stats(csperf_server_t*server)
{
if(server->show_stats){
ansperf_stats_display(&server->stats,server->output_file);

/*Then0itout*/
memset(&server->stats,0,sizeof(server->stats));
server->show_stats=0;
}
}

/*Updatetimer.Itticksevery1second*/
staticint
csperf_server_timer_update(csperf_server_t*server)
{
structtimevaltimeout;
timeout.tv_sec=1;
timeout.tv_usec=0;

evtimer_add(server->second_timer,&timeout);

return0;
}

/*Calledwhenthetimeouthappens*/
staticvoid
csperf_server_timer_cb(intfd,shortkind,void*userp)
{
csperf_server_t*server=(csperf_server_t*)userp;

/*Displaycurrentstats*/
csperf_server_timer_update(server);
}

/*Initserversubsystem*/
staticcsperf_server_t*
csperf_server_init(csperf_config_t*config)
{
inti;
csperf_server_t*server;

server=(csperf_server_t*)calloc(1,sizeof(csperf_server_t));
if(!server){
returnNULL;
}

if(!(server->evbase=event_base_new())){
free(server);
returnNULL;
}

/*Setupallcommands.Wealsodothisonce.
*Noteverythingwesetupmightbeused*/
for(i=0;i<CS_CMD_MAX;i++){
if(!(server->command_pdu_table[i]=
csperf_network_create_pdu(CS_MSG_COMMAND,i,
CS_COMMAND_PDU_LEN))){
free(server);
returnNULL;
}
}

if(config->server_output_file){
if(!(server->output_file=fopen(config->server_output_file,"w"))){
zlog_warn(log_get_cat_csperf(),"Failedtocreate%sfile\n",config->server_output_file);
free(server);
returnNULL;
}
}

server->config=config;
returnserver;
}

/*Calledafterwearedoneprocessingtheclientdata*/
staticint
csperf_server_send_mark_resp_command(csperf_server_t*server,uint8_tflags)
{
asn_command_pdu*command;

command=(asn_command_pdu*)(&server->
command_pdu_table[CS_CMD_MARK_RESP]->message);

command->blocks_to_receive=server->config->total_data_blocks;
command->echo_timestamp=command->echoreply_timestamp=
server->client_last_received_timestamp;
server->transfer_flags=command->flags=flags;
server->stats.total_commands_sent++;

/*Calculatethetimetoprocessthedata*/
server->stats.time_to_process_data=
csperf_network_get_time(server->stats.mark_sent_time)-
server->client_last_received_timestamp;

/*Endof1cycle*/
server->show_stats=1;
csperf_server_reset_stats(server);

returnbufferevent_write(server->buff_event,
server->command_pdu_table[CS_CMD_MARK_RESP],
CS_HEADER_PDU_LEN+CS_COMMAND_PDU_LEN);
}

staticint
csperf_server_process_data(csperf_server_t*server,structevbuffer*buf,
uint32_tlen)
{
server->stats.total_bytes_received+=len;
server->stats.total_blocks_received++;

if(server->transfer_flags==CS_FLAG_DUPLEX){
/*Moveittobufferevent'soutputqueue.
*Basically,wearejustechoingbackthedata*/
evbuffer_remove_buffer
(buf,bufferevent_get_output(server->buff_event),len);
server->stats.total_bytes_sent+=len;
server->stats.total_blocks_sent++;
}else{
/*Silentdraindata*/
evbuffer_drain(buf,len);
}

/*Thatsthedatablockswereceive.Sendmarkrespcommand*/
if(server->stats.total_blocks_received>=
server->config->total_data_blocks){
csperf_server_send_mark_resp_command(server,0);
}
return0;
}

/*Processthecommand*/
staticint
csperf_server_process_command(csperf_server_t*server,structevbuffer*buf)
{
asn_command_pducommand={0};

/*Removeheader*/
evbuffer_drain(buf,CS_HEADER_PDU_LEN);

evbuffer_remove(buf,&command,CS_COMMAND_PDU_LEN);

switch(command.command_type){
caseCS_CMD_MARK:
assert(command.blocks_to_receive);
server->transfer_flags=command.flags;
server->config->total_data_blocks=command.blocks_to_receive;
server->client_last_received_timestamp=command.echo_timestamp;
csperf_network_get_time(server->stats.mark_received_time);
break;
default:
zlog_warn(log_get_cat_csperf(),"Unexpectedcommand\n");
return-1;
}
server->stats.total_commands_received++;
return0;
}

staticvoid
csperf_accept_error(structevconnlistener*listener,void*ctx)
{
interr=EVUTIL_SOCKET_ERROR();

zlog_warn(log_get_cat_csperf(),"Server:Gotanerror%d(%s)onthelistener."
"Shuttingdown.\n",err,evutil_socket_error_to_string(err));

csperf_server_shutdown((csperf_server_t*)ctx);
}

/*Calledwhenthereisnewstufftoberead*/
void
csperf_server_readcb(structbufferevent*bev,void*ptr)
{
structevbuffer*input_buf;
intmessage_type;
csperf_server_t*server=(csperf_server_t*)ptr;
uint32_tlen=0;

/*Getbufferfrominputqueue*/
do{
input_buf=bufferevent_get_input(bev);
message_type=csperf_network_get_pdu_type(input_buf,&len);

/*Completepduhasn'tarrivedyet*/
if(!message_type){
break;
}

if(message_type==CS_MSG_DATA){
csperf_server_process_data(server,input_buf,len);
}elseif(message_type==CS_MSG_COMMAND){
/*Wegotacommandfromserver*/
csperf_server_process_command(server,input_buf);
}else{
assert(0);
}
}while(input_buf&&evbuffer_get_length(input_buf));
}

/*Handleeventsthatwegetonaconnection*/
void
csperf_server_eventcb(structbufferevent*bev,shortevents,void*ctx)
{
csperf_server_t*server=ctx;
intfinished=0;

if(events&BEV_EVENT_ERROR){
zlog_warn(log_get_cat_csperf(),"Error:%s\n",evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
finished=1;
}

if(events&BEV_EVENT_EOF){
finished=1;
}

if(finished){
/*Displaystats*/
csperf_server_reset_stats(server);
}
}

staticvoid
csperf_server_accept(structevconnlistener*listener,
evutil_socket_tfd,structsockaddr*address,intsocklen,
void*ctx)
{
csperf_server_t*server=(csperf_server_t*)ctx;
structevent_base*base=evconnlistener_get_base(listener);

/*Currentlywecanhandlejustoneclient.Createanarray
ofbuffereventswhenweneedtosupportmore*/
server->buff_event=bufferevent_socket_new(base,fd,
BEV_OPT_CLOSE_ON_FREE);

/*Wegotanewconnection!Setupabuffereventforit.*/
/*Setcallbacks*/
bufferevent_setcb(server->buff_event,csperf_server_readcb,
NULL,csperf_server_eventcb,server);
bufferevent_enable(server->buff_event,EV_READ|EV_WRITE);
bufferevent_setwatermark(server->buff_event,EV_READ,CS_HEADER_PDU_LEN,0);
server->show_stats=1;
}

int
csperf_server_configure(csperf_server_t*server)
{
structsockaddr_insin;
structevconnlistener*listener;

memset(&sin,0,sizeof(sin));
sin.sin_family=AF_INET;
sin.sin_addr.s_addr=htonl(0);
sin.sin_port=htons(server->config->server_port);

listener=evconnlistener_new_bind(server->evbase,
csperf_server_accept,server,
LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,-1,
(structsockaddr*)&sin,sizeof(sin));

if(!listener){
return-1;
}
evconnlistener_set_error_cb(listener,csperf_accept_error);
zlog_info(log_get_cat_csperf(),"Server:Listeningon%s:%d\n",
inet_ntoa(sin.sin_addr),server->config->server_port);
return0;
}

int
csperf_server_run(csperf_config_t*config)
{
interror=0;
csperf_server_t*server=NULL;

if(!(server=csperf_server_init(config))){
zlog_warn(log_get_cat_csperf(),"Failedtoinitserver\n");
return-1;
}

if((error=csperf_server_configure(server))){
zlog_warn(log_get_cat_csperf(),"Failedtoconfigureserver\n");
csperf_server_shutdown(server);
returnerror;
}

server->second_timer=evtimer_new(server->evbase,
csperf_server_timer_cb,server);
csperf_server_timer_update(server);

/*Runtheeventloop.Listenforconnection*/
event_base_dispatch(server->evbase);
return0;
}
