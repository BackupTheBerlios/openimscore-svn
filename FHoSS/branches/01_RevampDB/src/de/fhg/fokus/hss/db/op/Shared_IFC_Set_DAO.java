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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.hibernate.Hibernate;
import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.ApplicationServer;
import de.fhg.fokus.hss.db.model.Shared_IFC_Set;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class Shared_IFC_Set_DAO {
	
	public static void insert(Session session, Shared_IFC_Set shared_ifc_set){
		session.save(shared_ifc_set);
	}
	
	public static void update(Session session, Shared_IFC_Set shared_ifc_set){
		session.saveOrUpdate(shared_ifc_set);
	}
	
	public static List get_all_from_Set(Session session, int id_set){
		Query query = session.createSQLQuery("select * from shared_ifc_set where id_set=?")
			.addEntity(Shared_IFC_Set.class);
		query.setInteger(0, id_set);
		return query.list();
	}

	public static Object[] get_all_from_Set(Session session, int id_set, int firstResult, int maxResults){
		Query query = session.createSQLQuery("select * from shared_ifc_set where id_set=?")
			.addEntity(Shared_IFC_Set.class);
		query.setInteger(0, id_set);
		
		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		
		return result;
	}
	
	public static List get_all_Sets(Session session){
		Query query = session.createSQLQuery("select distinct id_set, name from shared_ifc_set")
			.addScalar("id_set", Hibernate.INTEGER)
			.addScalar("name", Hibernate.STRING);
		
		List result = new ArrayList();
		if (query.list() != null){
			Iterator it = query.list().iterator();
			Object [] row = null;
			while (it.hasNext()){
				row = (Object[]) it.next();
				Shared_IFC_Set shared_IFC = null;
				if (row != null){
					shared_IFC = new Shared_IFC_Set();
					shared_IFC.setId_set((Integer)row[0]);
					shared_IFC.setName((String)row[1]);
				}
				result.add(shared_IFC);
			}
		} 

		return result;
	}

	public static Shared_IFC_Set get_by_ID(Session session, int id){
		Query query;
		query = session.createSQLQuery("select * from shared_ifc_set where id=?")
			.addEntity(Shared_IFC_Set.class);
		query.setInteger(0, id);

		return (Shared_IFC_Set) query.uniqueResult();
	}
	
	public static List get_all_by_IFC_ID(Session session, int id_ifc){
		Query query;
		query = session.createSQLQuery("select * from shared_ifc_set where id_ifc=?")
			.addEntity(Shared_IFC_Set.class);
		query.setInteger(0, id_ifc);

		return query.list();
	}
	
	
	public static Object[] get_by_Wildcarded_Name(Session session, String name, 
			int firstResult, int maxResults){
		
		Query query;
		query = session.createSQLQuery("select * from shared_ifc_set where name like ?")
			.addEntity(Shared_IFC_Set.class);
		query.setString(0, "%" + name + "%");

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	
	public static Object[] get_all(Session session, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from shared_ifc_set")
			.addEntity(Shared_IFC_Set.class);
		
		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	
	public static Object[] get_by_Wildcarded_IFC_Name(Session session, String name_ifc, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from shared_ifc_set, ifc where shared_ifc_set.id_ifc=ifc.id and ifc.name like ?")
			.addEntity(Shared_IFC_Set.class);
		query.setString(0, "%" + name_ifc + "%");
		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static int delete_Set_by_ID(Session session, int id){
		Query query = session.createSQLQuery("delete from shared_ifc_set where id_set=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
	
	
	public static int delete_IFC_from_Set(Session session, int id_set, int id_ifc){
		//Query query = session.createSQLQuery("delete from shared_ifc_set where id_set=? and id_ifc=?");
		//query.setInteger(0, id_set);
		//query.setInteger(1, id_ifc);
		//return query.executeUpdate();
		return 1;
	}

	public static int delete_by_ID(Session session, int id){
		Query query = session.createSQLQuery("delete from shared_ifc_set where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
	
}
