/*
 * $Id: locsip_subscribe.c 161 2009-07-08 14:06:01Z aon $
 *  
 * Copyright (C) 2009 FhG Fokus
 *
 * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
 * implementation
 *
 * Open IMS Core is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the Open IMS Core software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact Fraunhofer FOKUS by e-mail at the following
 * addresses:
 *     info@open-ims.org
 *
 * Open IMS Core is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * It has to be noted that this Open Source IMS Core System is not 
 * intended to become or act as a product in a commercial context! Its 
 * sole purpose is to provide an IMS core reference implementation for 
 * IMS technology testing and IMS application prototyping for research 
 * purposes, typically performed in IMS test-beds.
 * 
 * Users of the Open Source IMS Core System have to be aware that IMS
 * technology may be subject of patents and licence terms, as being 
 * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
 * standards. Thus all Open IMS Core users have to take notice of this 
 * fact and have to agree to check out carefully before installing, 
 * using and extending the Open Source IMS Core System, if related 
 * patents and licences may become applicable to the intended usage 
 * context.  
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

/**
 * \file
 * 
 * LRF : subscription to location information interface
 *
 * \author Andreea Ancuta Onofrei andreea.ancuta.onofrei -at- fokus.fraunhofer.de
 */

#include "locsip_subscribe.h"

#include <libxml/xmlschemas.h>
#include <libxml/parser.h>
 

#include "mod.h" 
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../parser/parse_uri.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../dialog/dlg_mod.h"
#include "sip.h"

extern struct tm_binds tmb;   		/**< Structure with pointers to tm funcs 		*/
extern dlg_func_t dialogb;			/**< Structure with pointers to dialog funcs			*/

extern str lrf_name_str;			/**< fixed SIP URI of this P-CSCF 				*/

extern time_t time_now;				/**< current time 								*/

extern int lrf_subscribe_retries;	/**< times to retry subscribe to location on failure */

loc_subscription_hash_slot *subscriptions=0;/**< list of subscriptions					*/
extern int subscriptions_hash_size;	/**< the size of the hash table for subscriptions	*/
time_t time_now;				/**< current time 								*/

/**
 * Computes the hash for a contact.
 * @param aor - the string of the target
 * @returns the hash for the contact
 */
inline unsigned int get_subscription_hash(str uri)
{
#define h_inc h+=v^(v>>3)
   char* p;
   register unsigned v;
   register unsigned h;
   h=0;
   for (p=uri.s; p<=(uri.s+uri.len-4); p+=4){
       v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
       h_inc;
   }
   v=0;
   for (;p<(uri.s+uri.len); p++) {
       v<<=8;
       v+=*p;
   }
   h_inc;
   h=((h)+(h>>11))+((h>>13)+(h>>23));
   return (h)%subscriptions_hash_size;
#undef h_inc 
}

/**
 * Lock a subscription hash slot.
 * @param hash - index to lock
 */
inline void subs_lock(unsigned int hash)
{
//	LOG(L_CRIT,"GET %d\n",hash);
	lock_get(subscriptions[hash].lock);
//	LOG(L_CRIT,"GOT %d\n",hash);
}
/**
 * UnLock a subscriptions hash slot.
 * @param hash - index to unlock
 */
inline void subs_unlock(unsigned int hash)
{
//	LOG(L_CRIT,"REL %d\n",hash);	
	lock_release(subscriptions[hash].lock);
}
/**
 * Initialize the subscription list.
 * @returns 1 if ok, 0 on error
 */
int loc_subscription_init()
{
	int i;
	subscriptions = shm_malloc(sizeof(loc_subscription_hash_slot)*subscriptions_hash_size);
	if (!subscriptions) return 0;
	memset(subscriptions,0,sizeof(loc_subscription_hash_slot)*subscriptions_hash_size);
	for(i=0;i<subscriptions_hash_size;i++){
		subscriptions[i].lock = lock_alloc();
		if (!subscriptions[i].lock) return 0;
		subscriptions[i].lock = lock_init(subscriptions[i].lock);
	}
	return 1;
}

/**
 * Destroys the subscription list
 */
void loc_subscription_destroy()
{
	int i;
	loc_subscription *s,*ns;
	for(i=0;i<subscriptions_hash_size;i++){
		subs_lock(i);
		s = subscriptions[i].head;
		while(s){
			ns = s->next;
			//TODO send out unSUBSCRIBE
			free_loc_subscription(s);
			s = ns;
		}
		lock_destroy(subscriptions[i].lock);
		lock_dealloc(subscriptions[i].lock);
	}	
	shm_free(subscriptions);
}



/**
 * Creates a subcription and starts the timer resubscription for the given contact.
 * @param uri - the contact to subscribe to (actually to its default public id)
 * @param duration - SUBCRIBE expires
 * @returns 1 on success, 0 on failure
 */
loc_subscription* loc_subscribe(str uri,int duration)
{
	loc_subscription *s;
	/* first we try to update. if not found, add it */
	LOG(L_DBG,"DBG:"M_NAME":loc_subscription: begin\n");
	s = get_loc_subscription(uri);	
	if (s){
		s->duration = duration;
		s->attempts_left=lrf_subscribe_retries;
		subs_unlock(s->hash);
	}else{			
		s = new_loc_subscription(uri,duration);
		if (!s){
			LOG(L_ERR,"ERR:"M_NAME":loc_subscribe: Error creating new subscription\n");
			return 0;
		}
		add_loc_subscription(s);
		s->attempts_left=lrf_subscribe_retries;
	}
		
	return s;
}


static str method={"SUBSCRIBE",9};
static str event_hdr={"Event: location\r\n",17};
static str accept_hdr={"Accept: application/pidf+xml\r\n",30};
static str content_len_hdr={"Content-Length: 0\r\n",19};
static str max_fwds_hdr={"Max-Forwards: 10\r\n",18};
static str expires_s={"Expires: ",9};
static str expires_e={"\r\n",2};
static str contact_s={"Contact: <",10};
static str contact_e={">\r\n",3};
static str p_asserted_identity_s={"P-Asserted-Identity: <",22};
static str p_asserted_identity_e={">\r\n",3};
/**
 * Send a subscription
 * @param s - the subsription to send for
 * @param duration - expires time
 * @returns true if OK, false if not, error on failure
 * \todo store the dialog and reSubscribe on the same dialog
 */
int loc_send_subscribe(loc_subscription *s, str route, int duration)
{
	str h={0,0};

	LOG(L_DBG,"DBG:"M_NAME":loc_send_subscribe: SUBSCRIBE to <%.*s>\n",
		s->req_uri.len,s->req_uri.s);
	
	h.len = event_hdr.len+accept_hdr.len+content_len_hdr.len+max_fwds_hdr.len;
	h.len += expires_s.len + 12 + expires_e.len;

	h.len += contact_s.len + lrf_name_str.len + contact_e.len;

	h.s = pkg_malloc(h.len);
	if (!h.s){
		LOG(L_ERR,"ERR:"M_NAME":loc_send_subscribe: Error allocating %d bytes\n",h.len);
		h.len = 0;
		return 0;
	}

	h.len = 0;
	STR_APPEND(h,event_hdr);
	STR_APPEND(h,accept_hdr);
	STR_APPEND(h,content_len_hdr);
	STR_APPEND(h,max_fwds_hdr);

	STR_APPEND(h,expires_s);
	sprintf(h.s+h.len,"%d",duration);
	h.len += strlen(h.s+h.len);
	STR_APPEND(h,expires_e);

	STR_APPEND(h,contact_s);
	STR_APPEND(h,lrf_name_str);
	STR_APPEND(h,contact_e);
	
	if (!s->dialog){
		/* this is the first request in the dialog */
		if (tmb.new_dlg_uac(0,0,1,&lrf_name_str,&s->req_uri,&s->dialog)<0){
			LOG(L_ERR,"ERR:"M_NAME":loc_send_subscribe: Error creating a dialog for SUBSCRIBE\n");
			goto error;
		}
		set_dialog_route_set(s->dialog, route);
		LOG(L_DBG, "DBG:"M_NAME":loc_send_subsribe: next hop is %.*s dst_uri is %.*s\n",
				s->dialog->hooks.next_hop->len,
				s->dialog->hooks.next_hop->s,
				s->dialog->dst_uri.len,
				s->dialog->dst_uri.s);
		if (dialogb.request_outside(&method, &h, 0, s->dialog, loc_subscribe_response,  0) < 0){
			LOG(L_ERR,"ERR:"M_NAME":loc_send_subscribe: Error sending initial request in a SUBSCRIBE dialog\n");
			goto error;
		}		
	}else{
		/* this is a subsequent subscribe */
		if (dialogb.request_inside(&method, &h, 0, s->dialog, loc_subscribe_response,  0) < 0){
			LOG(L_ERR,"ERR:"M_NAME":loc_send_subscribe: Error sending subsequent request in a SUBSCRIBE dialog\n");
			goto error;
		}				
	}

	if (h.s) pkg_free(h.s);
	return 1;

error:
	if (h.s) pkg_free(h.s);
	return 0;
}

/**
 * Response callback for subscribe
 */
void loc_subscribe_response(struct cell *t,int type,struct tmcb_params *ps)
{
	str req_uri;
	int expires;
	loc_subscription *s=0;
	LOG(L_DBG,"DBG:"M_NAME":loc_subscribe_response: code %d\n",ps->code);
	if (!ps->rpl || ps->rpl==(void*) -1) {
		LOG(L_ERR,"INF:"M_NAME":loc_subscribe_response: No reply\n");
		return;	
	}
	if (!cscf_get_to_uri(ps->rpl,&req_uri)){
		LOG(L_ERR,"INF:"M_NAME":loc_subscribe_response: Error extracting the original Req-URI from To header\n");
		return;
	} 		
	s = get_loc_subscription(req_uri);
	if (!s){
		LOG(L_ERR,"INF:"M_NAME":loc_subscribe_response: received a SUBSCRIBE response but no subscription for <%.*s>\n",
			req_uri.len,req_uri.s);
		return;
	}
	if (ps->code>=200 && ps->code<300){
		expires = cscf_get_expires_hdr(ps->rpl);
		update_loc_subscription(s,expires);
		tmb.dlg_response_uac(s->dialog, ps->rpl, IS_TARGET_REFRESH);
	}else
		if (ps->code==404){
			update_loc_subscription(s,0);			
			//tmb.dlg_response_uac(s->dialog, ps->rpl, IS_TARGET_REFRESH);
		}else{
			LOG(L_INFO,"INF:"M_NAME":loc_subscribe_response: SUBSCRIRE response code %d ignored\n",ps->code);				
		}	
	if (s) subs_unlock(s->hash);		
}


#ifdef WITH_IMS_PM
	static str zero={0,0};
#endif

/**
 * Alters the saved dlg_t routes for the dialog 
 * required because the location subscription is not necesarily 
 * sent to the same domain as the user URI
 * @param d - the dialog to modify the Record-Routes
 * @param dir - the new route
 */
void set_dialog_route_set(dlg_t *d, str route)
{
	rr_t *r,*r_new;	
		
	for(r=d->route_set;r!=NULL;r=r->next) {
		r_new=r->next;
		r->next=NULL;
		shm_free_rr(&r);
		d->route_set = r_new;
  	}	
	/*LOG(L_DBG, "DBG:"M_NAME":set_dialog_route_set: settings to %.*s\n", route.len, route.s);
	char * p = (char*)shm_malloc(sizeof(rr_t)+(route.len-2)*sizeof(char));
	memset(p, sizeof(p), 0);
	r_new = (rr_t*)p;
	r_new->len = route.len;
	r_new->nameaddr.uri.s= p+sizeof(rr_t);
	memcpy(r_new->nameaddr.uri.s, route.s+1, sizeof(char)*route.len-2);
	r_new->nameaddr.uri.len= route.len-2;
	r_new->nameaddr.len = route.len;*/
	if(d->dst_uri.s)
		shm_free(d->dst_uri.s);
	d->dst_uri.s = 0;
	d->dst_uri.len = 0;
	tmb.calculate_hooks(d);

	STR_SHM_DUP(d->dst_uri, route, "alter_dialog_route_set");
out_of_memory:
	return;

}

extern str locsip_server_route;
/**
 * The Subscription timer looks for almost expired subscriptions and subscribes again.
 * @param ticks - the current time
 * @param param - the generic parameter
 */
void subscription_timer(unsigned int ticks, void* param)
{
	loc_subscription *s,*ns;
	int i;
	#ifdef WITH_IMS_PM
		int subs_cnt=0;
	#endif
	for(i=0;i<subscriptions_hash_size;i++){
		subs_lock(i);
		s = subscriptions[i].head;
//		loc_act_time();
		while(s){
			ns = s->next;			
			if (s->attempts_left > 0 ){
				/* attempt to send a subscribe */
				if (!loc_send_subscribe(s,locsip_server_route, s->duration)){
					LOG(L_ERR,"ERR:"M_NAME":subscription_timer: Error on SUBSCRIBE (%d times)... droping\n",
						lrf_subscribe_retries);
					del_loc_subscription_nolock(s);
				}else{
					s->attempts_left--;
					#ifdef WITH_IMS_PM
						subs_cnt++;
					#endif
				}
			}else if (s->attempts_left==0) {
				/* we failed to many times, drop the subscription */
				LOG(L_ERR,"ERR:"M_NAME":subscription_timer: Error on SUBSCRIBE for %d times... aborting\n",lrf_subscribe_retries);
				del_loc_subscription_nolock(s);										
			}else{
				/* we are subscribed already */
				/* if expired, drop it */
				if (s->expires<time_now) 
					del_loc_subscription_nolock(s);
				#ifdef WITH_IMS_PM
					else subs_cnt++;
				#endif
					
				/* if not expired, check for renewal */
//		Commented as the S-CSCF should adjust the subscription time accordingly				
//				if ((s->duration<1200 && s->expires-time_now<s->duration/2)||
//					(s->duration>=1200 && s->expires-time_now<600))
//				{
//					/* if we need a resubscribe, we mark it as such and try to subscribe again */					
//					s->attempts_left = lrf_subscribe_retries;
//					ns = s;
//				}
			}
			s = ns;
		}	
		subs_unlock(i);
	}
	print_subs(L_INFO);
	#ifdef WITH_IMS_PM
		IMS_PM_LOG01(RD_NbrSubs,subs_cnt);
	#endif	
}





/**
 * Creates a subscription based on the given parameters.
 * @param req_uri - the AOR of the user to subcribe to
 * @param duration - expires time in seconds
 * @returns the loc_notification or NULL on error
 */
loc_subscription* new_loc_subscription(str req_uri,int duration)
{
	loc_subscription *s=0;
	
	LOG(L_ERR,"ERR:"M_NAME":new_loc_subscription: begin\n");
	s = shm_malloc(sizeof(loc_subscription));
	if (!s){
		LOG(L_ERR,"ERR:"M_NAME":new_loc_subscription: Error allocating %d bytes\n",
			sizeof(loc_subscription));
		goto error;
	}
	memset(s,0,sizeof(loc_subscription));
	
	STR_SHM_DUP(s->req_uri,req_uri,"new_loc_subscription");
	LOG(L_DBG,"DBG:"M_NAME":new_loc_subscription: initial req uri %.*s, final %.*s\n",
			req_uri.len, req_uri.s, s->req_uri.len, s->req_uri.s);

	if (!s->req_uri.s) goto error;
		
	s->duration = duration;
	s->expires = 0;
	
	return s;
error:
out_of_memory:
	if (s->req_uri.s) shm_free(s->req_uri.s);
	if (s) shm_free(s);	
	return 0;
}

/**
 * Adds a subscription to the list of subscriptions at the end (FIFO).
 * @param s - the subscription to be added
 */
void add_loc_subscription(loc_subscription *s)
{
	if (!s) return;
	s->hash = get_subscription_hash(s->req_uri);
	subs_lock(s->hash);
		s->next = 0;
		s->prev = subscriptions[s->hash].tail;
		if (subscriptions[s->hash].tail) subscriptions[s->hash].tail->next = s;			
		subscriptions[s->hash].tail = s;
		if (!subscriptions[s->hash].head) subscriptions[s->hash].head = s;		
	subs_unlock(s->hash);
}

/**
 * Updates the expiration time of a subscription.
 * \todo Maybe we should use a hash here to index it as this is called for every notification
 * @param aor - aor to look for
 * @param expires - new expiration time
 * @returns 1 if found, 0 if not
 */
int update_loc_subscription(loc_subscription *s,int expires)
{
	LOG(L_DBG,"DBG:"M_NAME":update_loc_subscription: refreshing subscription for <%.*s> [%d]\n",
		s->req_uri.len,s->req_uri.s,expires);
	s->attempts_left = -1;
	if (expires == 0) del_loc_subscription_nolock(s);
	else s->expires = expires+time_now;;
	subs_unlock(s->hash);	
	return 1;
}

/**
 * Returns a subscription if it exists
 * \note - this returns with a lock on the subscriptions[s->hash] if found. Don't forget to unlock when done!!!
 * @param aor - AOR to look for
 * @returns 1 if found, 0 if not
 */
loc_subscription* get_loc_subscription(str aor)
{
	loc_subscription *s;
	unsigned int hash = get_subscription_hash(aor);
	subs_lock(hash);
		s = subscriptions[hash].head;
		while(s){
			if (s->req_uri.len == aor.len &&
				strncasecmp(s->req_uri.s,aor.s,aor.len)==0)
			{
				return s;
			}
			s = s->next;
		}
	subs_unlock(hash);	
	return 0;
}

/**
 * Finds out if a subscription exists
 * @param aor - AOR to look for
 * @returns 1 if found, 0 if not
 */
int is_loc_subscription(str aor)
{
	loc_subscription *s;
	unsigned int hash = get_subscription_hash(aor);
	subs_lock(hash);
		s = subscriptions[hash].head;
		while(s){
			if (s->req_uri.len == aor.len &&
				strncasecmp(s->req_uri.s,aor.s,aor.len)==0)
			{
				subs_unlock(hash);	
				return 1;
			}
			s = s->next;
		}
	subs_unlock(hash);	
	return 0;
}

/**
 * Deletes a subscription from the list of subscriptions 
 * @param s - the subscription to be deleted
 */
void del_loc_subscription(loc_subscription *s)
{
	if (!s) return;
	subs_lock(s->hash);
		if (subscriptions[s->hash].head == s) subscriptions[s->hash].head = s->next;
		else s->prev->next = s->next;
		if (subscriptions[s->hash].tail == s) subscriptions[s->hash].tail = s->prev;
		else s->next->prev = s->prev;
	subs_unlock(s->hash);
	free_loc_subscription(s);
}

/**
 * Deletes a subscription from the list of subscriptions.
 * \note Must have the lock to do this
 * @param s - the subscription to be deleted
 */
void del_loc_subscription_nolock(loc_subscription *s)
{
	if (!s) return;
	if (subscriptions[s->hash].head == s) subscriptions[s->hash].head = s->next;
	else s->prev->next = s->next;
	if (subscriptions[s->hash].tail == s) subscriptions[s->hash].tail = s->prev;
	else s->next->prev = s->prev;
	free_loc_subscription(s);
}

/**
 * Frees up space taken by a subscription
 * @param s - the subscription to free
 */
void free_loc_subscription(loc_subscription *s)
{
	if (s){
		if (s->req_uri.s) shm_free(s->req_uri.s);
		if (s->dialog) tmb.free_dlg(s->dialog);
		shm_free(s);
	}
}

void print_subs(int log_level)
{
	loc_subscription *s;
	int i;
	if (debug<log_level) return; /* to avoid useless calls when nothing will be printed */	
	LOG(log_level,ANSI_GREEN"INF:"M_NAME":----------  Subscription list begin ---------\n");
	for(i=0;i<subscriptions_hash_size;i++){
		subs_lock(i);
		s = subscriptions[i].head;
//		r_act_time();
		while(s){
			LOG(log_level,ANSI_GREEN"INF:"M_NAME":[%4u]\tP: <"ANSI_BLUE"%.*s"ANSI_GREEN"> D:["ANSI_CYAN"%5d"ANSI_GREEN"] E:["ANSI_MAGENTA"%5d"ANSI_GREEN"] Att:[%2d]\n",
				s->hash,s->req_uri.len,s->req_uri.s,s->duration,(int)(s->expires-time_now),s->attempts_left);
			s = s->next;			
		}
		subs_unlock(i);
	}
	LOG(log_level,ANSI_GREEN"INF:"M_NAME":----------  Subscription list end -----------\n");	
}








static xmlDtdPtr	dtd=0;	/**< DTD file */
static xmlValidCtxt	cvp;	/**< XML Validating context */

/**
 * Initializes the libxml parser.
 * @param dtd_filename - path to the DTD file
 * @returns 1 if OK, 0 on error
 */
int parser_init(char *dtd_filename)
{
	dtd = xmlParseDTD(NULL,(unsigned char*)dtd_filename);
	if (!dtd){
		LOG(L_ERR,"ERR:"M_NAME":parser_init: unsuccesful DTD parsing from file <%s>\n",
			dtd_filename);
		return 0;
	}
	cvp.userData = (void*)stderr;
	cvp.error = (xmlValidityErrorFunc) fprintf;
	cvp.warning = (xmlValidityWarningFunc) fprintf;
	return 1;
}

/**
 * Destroys the parser. 
 */
void parser_destroy()
{
	xmlCleanupParser();
}



/**
 * Trims spaces and duplicate content into pkg.
 * @param dest - destination
 * @param src - source
 */
static inline void space_trim_dup(str *dest, char *src)
{
	int i;
	dest->s=0;
	dest->len=0;
	if (!src) return;
	dest->len = strlen(src);
	i = dest->len-1;
	while((src[i]==' '||src[i]=='\t') && i>0) 
		i--;
	i=0;
	while((src[i]==' '||src[i]=='\t') && i<dest->len)
		i++;
	dest->len -= i;
	dest->s = pkg_malloc(dest->len);
	if (!dest->s) {
		LOG(L_ERR,"ERR:"M_NAME":space_trim_dup: Out of memory allocating %d bytes\n",dest->len);
		dest->len=0;
		return;
	}
	memcpy(dest->s,src+i,dest->len);
}

/**
 * Parses a notification and creates the loc_notification object
 * @param xml - the XML data
 * @returns the new loc_notification* or NULL on error
 */
loc_notification* loc_notification_parse(str xml)
{
	
	return 0;
}



/**
 * Processes a notification and updates the registrar info.
 * @param n - the notification
 * @param expires - the Subscription-Status expires parameter
 * @returns 1 on success, 0 on error
 */
int loc_notification_process(loc_notification *n,int expires)
{

	
	loc_notification_print(n);	
	if (!n) return 0;
	
//	lrf_act_time();
	
	return 1;
}

/** 
 * Prints the content of a notification
 * @param n - the notification to print
 */
void loc_notification_print(loc_notification *n)
{
	if (!n) return;
	LOG(L_DBG,"DBG:"M_NAME":loc_notification_print: State %d\n",n->state);
			
}

/**
 * Frees up the space taken by a notification
 * @param n - the notification to free
 */
void loc_notification_free(loc_notification *n)
{

}






