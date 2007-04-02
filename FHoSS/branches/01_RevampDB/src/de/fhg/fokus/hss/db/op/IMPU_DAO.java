/*
  *  Copyright (C) 2004-2007 FhG Fokus
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
  * patents and licenses may become applicable to the intended usage
  * context. 
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  
  * 
  */

package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.SP;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IMPU_DAO {
	
	public static void insert(Session session, IMPU impu){
		session.save(impu);
	}
	
	public static void update(Session session, IMPU impu){
		session.saveOrUpdate(impu);
	}	
	
	public static IMPU update(Session session, int id, short user_state){
		IMPU impu = (IMPU) session.load(IMPU.class, id);
		if (impu != null){
			impu.setUser_state(user_state);
		}
		return impu;
	}
	
	
	public static IMPU get_by_ID(Session session, int id){
		Query query;
		query = session.createSQLQuery("select * from impu where id=?")
			.addEntity(IMPU.class);
		query.setInteger(0, id);

		return (IMPU) query.uniqueResult();
	}
	
	public static IMPU get_by_Identity(Session session, String identity){
		Query query;
		query = session.createSQLQuery("select * from impu where identity like ?")
			.addEntity(IMPU.class);
		query.setString(0, identity);
		return (IMPU) query.uniqueResult();
	}
	
	public static Object[] get_by_Wildcarded_Identity(Session session, String identity, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from impu where identity like ?")
			.addEntity(IMPU.class);
		query.setString(0, "%" + identity + "%");

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static Object[] get_all(Session session, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from impu")
			.addEntity(IMPU.class);
		
		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static int delete_by_ID(Session session, int id){
		Query query = session.createSQLQuery("delete from impu where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
	
	public static List get_all_from_set(Session session, int id_set){
		Query query;
		query = session.createSQLQuery("select * from impu where id_impu_implicitset=?")
			.addEntity(IMPU.class);
		query.setInteger(0, id_set);
		return query.list();
	}
	
	public static Object[] get_all_from_set(Session session, int id_set, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from impu where id_impu_implicitset=?")
		.addEntity(IMPU.class);
		query.setInteger(0, id_set);

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static List get_others_from_set(Session session, int id, int id_set){
		Query query;
		query = session.createSQLQuery("select * from impu where id != ? and id_impu_implicitset=?")
			.addEntity(IMPU.class);
		query.setInteger(0, id);
		query.setInteger(1, id_set);
		return query.list();
	}
	
	public static List get_all_sp_for_set(Session session, int id_implicit_set){
		Query query;
		query = session.createSQLQuery(
				"select {SP.*}, {IMPU.*} from sp SP" +
				"	inner join impu IMPU on IMPU.id_sp=SP.id" +
				"		where IMPU.id_impu_implicitset=? order by IMPU.id_sp")
					.addEntity(SP.class)
					.addEntity(IMPU.class);
		query.setInteger(0, id_implicit_set);
		return query.list();
	}
	
	
	public static List get_by_Charging_Info_ID(Session session, int id_charging_info){
		Query query;
		query = session.createSQLQuery("select * from impu where id_charging_info = ?")
			.addEntity(IMPU.class);
		query.setInteger(0, id_charging_info);
		return query.list();
	}
}
