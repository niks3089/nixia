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

#ifndef__HTTP_CONNECTION_H
#define__HTTP_CONNECTION_H

#include<event2/event.h>
#include"http_stats.h"
#include"pi_dll.h"
#include<evhtp.h>

structhttp_worker_base_t;
structhttp_request_handle_t;

typedefenumhttp_connection_state_s
{
HTTP_CONN_STATE_UNITITIALIZED=0,
HTTP_CONN_STATE_INIT,
HTTP_CONN_STATE_RUNNING,
HTTP_CONN_STATE_ERROR,
HTTP_CONN_STATE_CLOSING,
HTTP_CONN_STATE_CLOSED,

/*Addnewstatesabove*/
HTTP_CONN_MAX_STATES,

}http_connection_fsm_state;

typedefenumhttp_connection_error_code_s
{
HTTP_CONN_ERR_NO_MEM=1,
HTTP_CONN_ERR_EVHTP,
HTTP_CONN_ERR_TRANSFER,

}http_connection_error_code;

typedefstructhttp_connection_s{

/*Attachittotheactivelistwhenthe
*connectionissetup*/
pi_dll_tactive_link;

/*EvHTPConnectionobject*/
evhtp_connection_t*ev_conn;

/*EvHTPRequestobject*/
evhtp_request_t*ev_req;

/*EvHTPresponsestatus*/
evhtp_resstatus;
structhttp_worker_base_t*base;
uint32_tconn_id;
uint8_tis_https:1;
uint8_tdone:1;
uint8_tinitialised:1;
uint8_treserved:6;
interror;
char*error_msg;
http_connection_fsm_statestate;
http_stats_connection_tstats;
}http_connection_t;

typedefstructhttp_connection_pool_s{
pi_dll_tactive_list;
uint32_ttotal_connections;
uint32_tnext_connection_to_run;
uint32_ttotal_pending_connections;
uint32_ttotal_connections_running;
structhttp_worker_base_t*base;
http_connection_tconnection_table[1];
}http_connection_pool_t;

/*FSMstuff*/
voidhttp_connection_fsm_process(http_connection_t*conn);
voidhttp_connection_fsm_init();
voidhttp_connection_fsm_process_all(structhttp_worker_base_t*base);
voidhttp_connection_fsm_close_all(structhttp_worker_base_t*base);
voidhttp_connection_fsm_close(http_connection_t*conn);
char*http_connection_fsm_state_name(http_connection_t*conn);
voidhttp_stats_update_connection_error(http_connection_t*conn);

/*Connectionstuff*/
http_connection_pool_t*http_connection_init_pool(void*config);
voidhttp_connection_check_completed_transfer(evhtp_request_t*req,void*arg);
voidhttp_connection_dump_stats(http_connection_t*conn);
voidhttp_connection_kickstart(structhttp_worker_base_t*base);
inthttp_connection_run_next_connection(structhttp_worker_base_t*base);
voidhttp_connection_run_connections(structhttp_worker_base_t*base,uint32_tcount);
voidhttp_stats_update_base(http_connection_t*conn);
voidhttp_stats_update_base_after_conn_completion(
http_connection_t*conn);
voidhttp_connection_send_request(http_connection_t*conn);
voidhttp_connection_pool_cleanup(structhttp_worker_base_t*base,
http_connection_pool_t*pool);
voidhttp_connection_get_connection_summary(http_connection_pool_t*pool,
char**output_str,int32_t*allocated_mem,int32_t*mem_left);
voidhttp_connection_re_run(structhttp_worker_base_t*base);
voidhttp_connection_write_summary_to_file(structhttp_worker_base_t*base);
voidhttp_stats_update_for_connection(http_connection_t*conn);
voidhttp_stats_increment_requests_sent(http_connection_t*conn);
voidhttp_stats_increment_responses_received(http_connection_t*conn);
voidhttp_stats_reset_per_second_stats(structhttp_worker_base_t*base);
voidhttp_stats_update_request_error(http_connection_t*conn);
voidhttp_stats_update_base_run_time(structhttp_worker_base_t*base);
voidhttp_stats_update_response_stats(http_connection_t*conn);
voidhttp_stats_start_time(structtimespec*now);
#endif/*__HTTP_CONNECTION_H*/
