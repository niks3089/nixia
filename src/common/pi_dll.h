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

#ifndef__PI_DLL_H
#define__PI_DLL_H

/*doublylinkedlist*/

typedefstructpi_dll_spi_dll_t;

structpi_dll_s{
pi_dll_t*dll_next;
pi_dll_t*dll_prev;
};

#definedll_headdll_next
#definedll_taildll_prev

#definepi_dll_empty(_dll)((_dll)->dll_next==(_dll))
#definepi_dll_queued_m(_dll)((_dll)->dll_next!=(_dll))

voidpi_dll_init(pi_dll_t*list);
pi_dll_t*pi_dll_next(pi_dll_t*entry);
pi_dll_t*pi_dll_prev(pi_dll_t*entry);
voidpi_dll_insert_head(pi_dll_t*list,pi_dll_t*entry);
voidpi_dll_insert_tail(pi_dll_t*list,pi_dll_t*entry);
voidpi_dll_insert_following(pi_dll_t*ref,pi_dll_t*entry);
voidpi_dll_insert_preceeding(pi_dll_t*ref,pi_dll_t*entry);
voidpi_dll_move_list(pi_dll_t*dst,pi_dll_t*src);
voidpi_dll_unlink(pi_dll_t*entry);
intpi_dll_queued(pi_dll_t*entry);
pi_dll_t*pi_dll_dequeue_tail(pi_dll_t*list);
pi_dll_t*pi_dll_dequeue_head(pi_dll_t*list);

#definePI_IMH_BUF_SIZE64
#definePI_IP_HDR_BUF_SIZE64
#definePI_TCP_HDR_BUF_SIZE64


/*genericlinkagewithbackreference*/

typedefstructpi_brlink_s{
pi_dll_tbrl_dll;
void*brl_ref;
}pi_brlink_t;

#endif/*__PI_DLL_H*/
