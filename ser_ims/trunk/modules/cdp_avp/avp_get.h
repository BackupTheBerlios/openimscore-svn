/**
 * $Id$
 *   
 * Copyright (C) 2009-2010 FhG Fokus
 *
 * This file is part of Open EPC - an implementation of EPS core components
 * 
 * The Open EPC project  is a prototype implementation of the 3GPP 
 * Release 8 and later Evolved Packet Core (EPC) that will allow academic and 
 * industrial researchers and engineers around the world to obtain a 
 * practical look and feel of the capabilities of the Evolved Packet Core.
 * For more information on the Open EPC project, please visit www.openepc.net
 * or please contact Fraunhofer FOKUS by e-mail at the following addresses:
 *     info@openepc.net
 *
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
 * CDiameter AVP Operations modules - AVP retrieval
 * 
 * 
 *  \author Dragos Vingarzan dragos dot vingarzan -at- fokus dot fraunhofer dot de
 * 
 */ 

#ifndef __AVP_GET_H
#define __AVP_GET_H

#ifndef CDP_AVP_REFERENCE

	#include "../cdp/cdp_load.h"

	AAA_AVP* cdp_avp_get_next_from_list(AAA_AVP_LIST list,int avp_code,int avp_vendor_id,AAA_AVP *start_avp);
	typedef AAA_AVP* (*cdp_avp_get_next_from_list_f)(AAA_AVP_LIST list,int avp_code,int avp_vendor_id,AAA_AVP *start_avp);
	
	AAA_AVP* cdp_avp_get_next_from_msg(AAAMessage *msg,int avp_code,int avp_vendor_id,AAA_AVP *start_avp);
	typedef AAA_AVP* (*cdp_avp_get_next_from_msg_f)(AAAMessage *msg,int avp_code,int avp_vendor_id,AAA_AVP *start_avp);
	
	AAA_AVP* cdp_avp_get_from_list(AAA_AVP_LIST list,int avp_code,int avp_vendor_id);
	typedef AAA_AVP* (*cdp_avp_get_from_list_f)(AAA_AVP_LIST list,int avp_code,int avp_vendor_id);
	
	AAA_AVP* cdp_avp_get_from_msg(AAAMessage *msg,int avp_code,int avp_vendor_id);
	typedef AAA_AVP* (*cdp_avp_get_from_msg_f)(AAAMessage *msg,int avp_code,int avp_vendor_id);

#else

	AAA_AVP* basic.get_next_from_list(AAA_AVP_LIST list,int avp_code,int avp_vendor_id,AAA_AVP *start_avp);

	AAA_AVP* basic.get_next_from_msg(AAAMessage *msg,int avp_code,int avp_vendor_id,AAA_AVP *start_avp);

	AAA_AVP* basic.get_from_list(AAA_AVP_LIST list,int avp_code,int avp_vendor_id);

	AAA_AVP* basic.get_from_msg(AAAMessage *msg,int avp_code,int avp_vendor_id);

#endif



#endif /* __AVP_NEW_H */
