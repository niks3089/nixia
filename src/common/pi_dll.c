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

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/types.h>

#include"log.h"
#include"common.h"
#include"pi_dll.h"


void
pi_dll_init(pi_dll_t*list)
{
list->dll_head=list;
list->dll_tail=list;
}


/*Nexttwofunctionsarerequiredtopreventthecompilerfrom
optimizingawayorreorderingcertainlistconsistencychecks.*/

pi_dll_t
*pi_dll_next(pi_dll_t*entry)
{
return(entry->dll_next);
}


pi_dll_t
*pi_dll_prev(pi_dll_t*entry)
{
return(entry->dll_prev);
}


int
pi_dll_idle(pi_dll_t*entry)
{
return(entry->dll_next==entry&&entry->dll_prev==entry);
}


int
pi_dll_queued(pi_dll_t*entry)
{
return(entry->dll_next!=entry||entry->dll_prev!=entry);
}


void
pi_dll_insert_head(pi_dll_t*list,pi_dll_t*entry)
{
if(!pi_dll_idle(entry)){
zlog_error(log_get_cat(),"%s:entry%pisnotidle.\n",__FUNCTION__,entry);
*((char*)5)=0;
}

entry->dll_next=list->dll_head;
entry->dll_prev=list;
entry->dll_next->dll_prev=entry;
list->dll_head=entry;
}


void
pi_dll_insert_tail(pi_dll_t*list,pi_dll_t*entry)
{
if(!pi_dll_idle(entry)){
zlog_error(log_get_cat(),"%s:entry%pisnotidle.\n",__FUNCTION__,entry);
*((char*)5)=0;
}

entry->dll_next=list;
entry->dll_prev=list->dll_tail;
entry->dll_prev->dll_next=entry;
list->dll_tail=entry;
}


void
pi_dll_insert_following(pi_dll_t*ref,pi_dll_t*entry)
{
if(!pi_dll_idle(entry)){
zlog_error(log_get_cat(),"%s:entry%pisnotidle.\n",__FUNCTION__,entry);
*((char*)5)=0;
}

/*sameaspi_dll_insert_head()*/
entry->dll_next=ref->dll_next;
entry->dll_prev=ref;
entry->dll_next->dll_prev=entry;
ref->dll_next=entry;
}


void
pi_dll_insert_preceeding(pi_dll_t*ref,pi_dll_t*entry)
{
if(!pi_dll_idle(entry)){
zlog_error(log_get_cat(),"%s:entry%pisnotidle.\n",__FUNCTION__,entry);
*((char*)5)=0;
}

/*sameaspi_dll_insert_tail()*/
entry->dll_next=ref;
entry->dll_prev=ref->dll_prev;
entry->dll_prev->dll_next=entry;
ref->dll_prev=entry;
}


void
pi_dll_move_list(pi_dll_t*dst,pi_dll_t*src)
{
if(!pi_dll_idle(dst)){
zlog_error(log_get_cat(),"%s:entry%pisnotidle.\n",__FUNCTION__,dst);
*((char*)5)=0;
}

dst->dll_next=src->dll_next;
dst->dll_next->dll_prev=dst;
dst->dll_prev=src->dll_prev;
dst->dll_prev->dll_next=dst;

src->dll_next=src;
src->dll_prev=src;
}


void
pi_dll_unlink_ex(pi_dll_t*entry,char*caller)
{

if(pi_dll_idle(entry)){
zlog_error(log_get_cat(),"%s:entry%pisidle.\n",caller,entry);
return;
}
if(entry->dll_next==NULL&&entry->dll_prev==NULL){
zlog_error(log_get_cat(),"%s:entry%pisallNULL.\n",caller,entry);
return;
}
if(entry->dll_next==NULL||entry->dll_prev==NULL){
zlog_error(log_get_cat(),"%s:entry%pispartiallyNULL.\n",caller,entry);
return;
}

entry->dll_next->dll_prev=entry->dll_prev;
entry->dll_prev->dll_next=entry->dll_next;
entry->dll_next=entry->dll_prev=entry;
}


void
pi_dll_unlink(pi_dll_t*entry)
{
pi_dll_unlink_ex(entry,"pi_dll_unlink");
}

pi_dll_t*
pi_dll_dequeue_tail(pi_dll_t*list)
{
pi_dll_t*entry=list->dll_tail;

if(entry==list){
entry=NULL;
}

if(entry){
pi_dll_unlink_ex(entry,"pi_dll_dequeue_tail");
}

return(entry);
}

pi_dll_t*
pi_dll_dequeue_head(pi_dll_t*list)
{
pi_dll_t*entry=list->dll_head;

if(entry==list){
entry=NULL;
}

if(entry){
pi_dll_unlink_ex(entry,"pi_dll_dequeue_head");
}

return(entry);
}
