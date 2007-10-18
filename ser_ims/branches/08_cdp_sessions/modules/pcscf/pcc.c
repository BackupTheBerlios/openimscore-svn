/**
 * $Id: gq.c,v 1.10 2007/03/14 16:18:28 Alberto Exp $
 *   
 * Copyright (C) 2004-2007 FhG Fokus
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
 * P-CSCF Policy and Charging Control interface ops
 *  
 * \author Alberto Diez Albaladejo -at- fokus dot fraunhofer dot de
 */

#include "pcc.h"

#include "pcc_avp.h"
#include "sip.h"
#include "dlg_state.h"
#include "release_call.h" // for the ASR-ASA
#include "../tm/tm_load.h"


/**< Structure with pointers to tm funcs */
extern struct tm_binds tmb;

/**< Structure with pointers to cdp funcs */
extern struct cdp_binds cdpb;

/**< FQDN of PDF, defined in mod.c */
extern str forced_qos_peer; /*its the PCRF in this case*/

extern int pcscf_qos_release7;

inline str pcc_get_destination_realm(str s)
{
	str p;
	p.s = index(s.s, '.')+1;
	p.len = s.len - (p.s - s.s);
	return p;
}

static str s_orig={";orig",5};
static str s_term={";term",5};

/**
 * Modifies the call id by adding at the end orig or term
 * \note results is in shm, needs to be freed!
 * @param call_id - the original call_id
 * @param tag - 0 for orig, 1 for term
 * @returns the new shm string
 */
str pcc_modify_call_id(str call_id, int tag)
{
	str t;
	t.len = call_id.len + (tag?s_orig.len:s_term.len);
	t.s = shm_malloc(t.len);
	if (!t.s) {
		LOG(L_ERR,"ERR:"M_NAME":rx_modify_call_id(): error allocating %d bytes\n",t.len);
		t.len = 0;
		return t;
	}
	t.len=0;
	STR_APPEND(t,call_id);
	if (tag == 0) {
		STR_APPEND(t,s_orig);
	}
	else{ 
		STR_APPEND(t,s_term);
	}
	return t;
}

/**
 * Sends the Authorization Authentication Request.
 * @param req - SIP request  
 * @param res - SIP response
 * @param tag - 0 for originating side, 1 for terminating side
 * 
 * @returns AAA message or NULL on error  
 */
AAAMessage *PCC_AAR(struct sip_msg *req, struct sip_msg *res, int tag)
{
	AAAMessage* aar = NULL;
	AAAMessage* aaa = 0;
	p_dialog *dlg;
	AAASession* auth=0;
	t_authdata *generic=0;
	
	str sdpbodyinvite,sdpbody200;
	char *mline;
	int i=0;
	
	if (tag) 
		LOG(L_INFO, "INF:"M_NAME":PCC_AAR: terminating side\n");
	else 
		LOG(L_INFO, "INF:"M_NAME":PCC_AAR: originating side\n");
	
	
	
	/* Check for the existence of an auth session for this dialog */							
	/* if not, create an authorization session */

	
	str call_id = cscf_get_call_id(req, 0);
		
	if (!tag)
		dlg = get_p_dialog_dir(call_id,DLG_MOBILE_ORIGINATING);
	else 
		dlg = get_p_dialog_dir(call_id,DLG_MOBILE_TERMINATING);
	
	
	if (!dlg) goto error;
	
		
	if (!dlg->pcc_session) {
		 /*For the first time*/
		 generic=shm_malloc(sizeof(t_authdata));
		 generic->callid.s=0; generic->callid.len=0;
		 STR_SHM_DUP(generic->callid,call_id,"shm");
		 generic->direction=tag;
		 auth = cdpb.AAACreateAuthSession((void *)generic,1,1);		 
		 dlg->pcc_session = auth;
	}else{ 
		auth = dlg->pcc_session;
	}
	
	d_unlock(dlg->hash);
	
	if (!auth) goto error;												 

	/* Create an AAR prototype */
	if (pcscf_qos_release7)
		aar = cdpb.AAACreateRequest(IMS_Rx, IMS_AAR, Flag_Proxyable, auth);
	else
		aar = cdpb.AAACreateRequest(IMS_Gq, IMS_AAR, Flag_Proxyable, auth);
	
	if (!aar) goto error;
	
	/*---------- 1. Add mandatory AVPs ----------*/
	
	/* Session-Id, Origin-Host, Origin-Realm AVP are added by the stack. */
	
	/* Add Destination-Realm AVP */
	str realm = pcc_get_destination_realm(forced_qos_peer);
	if (!PCC_add_destination_realm(aar, realm)) goto error;
	
	/* Add Auth-Application-Id AVP */
	if (pcscf_qos_release7){
		if (!PCC_add_auth_application_id(aar, IMS_Rx)) goto error;
	}else{
		if (!PCC_add_auth_application_id(aar, IMS_Gq)) goto error;
	} 
	
	/*---------- 2. Create and add Media-Component-Description AVP ----------*/
	
	/*	
	 *  See 3GPP TS29214 V7.1.0:
	 * 
	 *  <Media-Component-Description> = {Media-Component-Number}
	 * 								 	[Media-Sub-Component]
	 * 								 	[AF-Application-Identifier]
	 * 								 	[Media-Type]
	 * 								 	[Max-Requested-Bandwidth-UL]
	 * 									[Max-Requested-Bandwidth-DL]
	 * 									[Flow-Status]
	 * 									[Reservation-Priority] (Not used yet)
	 * 								 	[RS-Bandwidth]
	 * 									[RR-Bandwidth]
	 * 									*[Codec-Data]
	 */

	if(extract_body(req,&sdpbodyinvite)==-1) 
	{
		LOG(L_ERR,"ERROR:"M_NAME":%s: No Body to extract in INVITE\n","rx_aar");
		goto error;
	}
	if(extract_body(res,&sdpbody200)==-1) 
	{
		LOG(L_ERR,"ERROR:"M_NAME":%s: No Body to extract in 200\n","rx_aar");
		goto error;
	}
	/*Create and add 1 media-component-description AVP for each
	 * m= line in the SDP body 
	 */
	
	mline=find_sdp_line(sdpbodyinvite.s,(sdpbodyinvite.s+sdpbodyinvite.len),'m');
	
	if (mline==NULL) goto error;
		
	while(mline!=NULL)
	{
		i++;
		
		if (!PCC_add_media_component_description(aar,sdpbodyinvite,sdpbody200,mline,i,tag))
		{
			LOG(L_ERR,"ERROR: PCC_AAR() : unable to add media component description AVP for line %i\n",i);
			goto error; /* Think about this*/
		}
		
		mline=find_next_sdp_line(mline,(sdpbodyinvite.s+sdpbodyinvite.len),'m',NULL);
	}
	
	/*
	to be added here
		
		Rx_add_specific_action(dia_aar,ACTION);
		Rx_add_subscription_ID(dia_aar,IMSI);
		Rx_add_reservation_priority(dia_aar,TISPANthing);
		Rx_add_Framed_IP_Address(dia_aar,Mariusthing);
		Rx_add_Framed_IPv6_Prefix(dia_aar,Mariusthing);
		Rx_add_Service_URN(dia_aar,emergencysession);
	
	*/
	
	
	/*---------- 3. Send AAR to PCRF ----------*/
	if (forced_qos_peer.len)
		aaa = cdpb.AAASendRecvMessageToPeer(aar,&forced_qos_peer);
	else 
		aaa = cdpb.AAASendRecvMessage(aar);	
	
	//auth->sm_process(auth, AUTH_EV_SEND_REQ, dia_aar, dia_aaa);

	//cdpb.AAAPrintMessage(dia_aaa);

	
	return aaa;
out_of_memory:
	LOG(L_CRIT,"PCC_AAR():out of memory\n");
error:
	if (auth) cdpb.AAADropAuthSession(auth);
	if (generic)
	{
		if (generic->callid.s) shm_free(generic->callid.s);
		shm_free(generic);
	}
	return NULL;
}

int PCC_AAA(AAAMessage *dia_msg)
{
	int rc;
	
	PCC_get_result_code(dia_msg,&rc);
	//cdpb.AAAFreeMessage(&dia_msg);
	return rc;
}



/**
 * Sends and Session Termination Request
 * @param msg - SIP request  
 * @param tag - 0 for originating side, 1 for terminating side
 * 
 * @returns AAA message or NULL on error
 */
AAAMessage* PCC_STR(struct sip_msg* msg, int tag)
{
	AAAMessage* dia_str = NULL;
	AAAMessage* dia_sta = NULL;
	
	p_dialog *dlg;
	t_authdata *g;
	
	
/** get Diameter session based on sip call_id */
	str call_id = cscf_get_call_id(msg, 0);
	
	
	
	if (tag)
		dlg = get_p_dialog_dir(call_id,DLG_MOBILE_ORIGINATING);
	else 
		dlg = get_p_dialog_dir(call_id,DLG_MOBILE_TERMINATING);
		
	if (!dlg)	goto end;
	if (!(dlg->pcc_session)) goto error_dlg;
		

	if (pcscf_qos_release7)
		dia_str = cdpb.AAACreateRequest(IMS_Rx, IMS_STR, Flag_Proxyable, dlg->pcc_session);
	else
		dia_str = cdpb.AAACreateRequest(IMS_Gq, IMS_STR, Flag_Proxyable, dlg->pcc_session);
	
	if (!dia_str) goto error;
	


	str realm = pcc_get_destination_realm(forced_qos_peer);
	if (!PCC_add_destination_realm(dia_str, realm)) goto error;


	if (pcscf_qos_release7){
		if (!PCC_add_auth_application_id(dia_str, IMS_Rx)) goto error;
	}else{
		if (!PCC_add_auth_application_id(dia_str, IMS_Gq)) goto error;
	} 
	
	/*Termination-Cause*/
	
	
	
	if (forced_qos_peer.len)
		dia_sta = cdpb.AAASendRecvMessageToPeer(dia_str,&forced_qos_peer);
	else 
		dia_sta = cdpb.AAASendRecvMessage(dia_str);	
	
	LOG(L_DBG,"PCC_STR successful STR-STA exchange\n");
	
	/*Before dropping the session remember the authdata!!*/
	g=(t_authdata *)dlg->pcc_session->u.auth.generic_data;
	if (g)
	{
		if (g->callid.s) shm_free(g->callid.s);
		shm_free(g);
		dlg->pcc_session->u.auth.generic_data=0;
	}
	
	cdpb.AAADropAuthSession(dlg->pcc_session);
	return dia_sta;
error:
	 cdpb.AAADropAuthSession(dlg->pcc_session);
error_dlg:
	d_unlock(dlg->hash);
end:
	return NULL;
}

str reason_terminate_dialog_s={"Session terminated ordered by the PCRF",38};
/*
 * Called upon reciept of an ASR terminates the user session and returns the ASA
 * Terminates the corresponding dialog
 * @param request - the received request
 * @returns the ASA with the result code
 * 
*/
AAAMessage* PCC_ASA(AAAMessage *request)
{
	AAAMessage *asa;
	t_authdata *data;
	p_dialog *p;
	char x[4];
	AAA_AVP *rc=0;
	
	data = (t_authdata *) request->session->u.auth.generic_data; //casting
		
	if (data->callid.s)
	{
		p=get_p_dialog_dir(data->callid,data->direction);
		release_call_p(p,503,reason_terminate_dialog_s);
		/*of course it would be nice to first have a look on the Abort-Cause AVP*/
		d_unlock(p->hash);
	}
	asa=cdpb.AAACreateResponse(request);
	if (!asa) return 0;
	set_4bytes(x,AAA_SUCCESS);
	
	
	rc=cdpb.AAACreateAVP(AVP_Result_Code,AAA_AVP_FLAG_MANDATORY,0,x,4,AVP_DUPLICATE_DATA);
	if (rc) cdpb.AAAAddAVPToMessage(asa,rc,NULL);
	
	// Should i drop the auth session here??	

	return asa;
}


/**
 * Handler for incoming Diameter requests.
 * @param request - the received request
 * @param param - generic pointer
 * @returns the answer to this request
 */
AAAMessage* PCCRequestHandler(AAAMessage *request,void *param)
{
	if (is_req(request)){		
		LOG(L_INFO,"INFO:"M_NAME":PCCRequestHandler(): We have received a request\n");
		#ifdef WITH_IMS_PM
			ims_pm_diameter_request(request);
		#endif		
		switch(request->applicationId){
        	case IMS_Rx:
        	case IMS_Gq: // Its almost the same!
				switch(request->commandCode){				
					case IMS_RAR:
						LOG(L_INFO,"INFO:"M_NAME":PCCRequestHandler():- Received an IMS_RAR \n");
						return 0; //return PCC_RAR(request);
						break;
					case IMS_ASR:
						LOG(L_INFO,"INFO:"M_NAME":PCCRequestHandler(): - Received an IMS_ASR \n");
						return PCC_ASA(request);
						break;
					default :
						LOG(L_ERR,"ERR:"M_NAME":PCCRequestHandler(): - Received unknown request for Rx  or Gq command %d\n",request->commandCode);
						break;	
				}
				break;
			default:
				LOG(L_ERR,"ERR:"M_NAME":PCCRequestHandler(): - Received unknown request for app %d command %d\n",
					request->applicationId,
					request->commandCode);
				break;				
		}					
	}
	return 0;		
}



