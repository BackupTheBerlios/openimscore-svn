<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
	
<%@ page import="java.util.*, de.fhg.fokus.hss.db.model.*, de.fhg.fokus.hss.db.op.*, de.fhg.fokus.hss.web.util.*,
	de.fhg.fokus.hss.db.hibernate.*, org.hibernate.Session " %>

<html>
<head>

<jsp:useBean id="scscf_list" type="java.util.List" scope="request"></jsp:useBean>

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> Preferred S-CSCF Set </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<%
	int id_set = Integer.parseInt(request.getParameter("id_set"));
%>

<script type="text/javascript" language="JavaScript">

function add_action_for_form(action, associated_ID) {
	switch(action){
		case 1:
			document.PrefS_Form.nextAction.value="save";
			document.PrefS_Form.submit();
			break;
		case 2:
			document.PrefS_Form.nextAction.value="refresh";
			document.PrefS_Form.submit();			
			break;
		case 3:
			document.PrefS_Form.nextAction.value="reset";
			document.PrefS_Form.reset();
			break;
		case 4:
			document.PrefS_Form.nextAction.value="delete";
			document.PrefS_Form.submit();			
			break;
		case 5:
			document.PrefS_Form.nextAction.value="delete_scscf";
			document.PrefS_Form.associated_ID.value = associated_ID;
			document.PrefS_Form.submit();
			break;
				
		case 12:
			document.PrefS_Form.nextAction.value="add_scscf";
			document.PrefS_Form.submit();
			break;
	}
}
</script>
</head>

<body>
	<html:form action="/PrefS_Submit">
		<html:hidden property="nextAction" value=""/>
		<html:hidden property="associated_ID" value=""/>
		
		<table align=center valign=middle height=100%>
		
		<tr>
			<!-- Print errors, if any -->
			<td>
				<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
			</td>
		</tr>	
		<tr>
			<td align="center"><h1>  Preferred S-CSCF Set </h1></td>
		</tr>
		<tr>
			<td>
	 			<table border="0" align="center" width="450" >						
	 			<tr>
 					<td>
						<table border="0" cellspacing="1" align="center" width="70%" style="border:2px solid #FF6600;">						
						<tr bgcolor="#FFCC66">
							<td> ID-Set </td>
							<td><html:text property="id_set" readonly="true" styleClass="inputtext_readonly"/> </td>
						</tr>
						<tr bgcolor="#FFCC66">
							<td>Name </td>
							<td><html:text property="name" styleClass="inputtext"/> </td>
						</tr>
						
						
			<%			if (id_set == -1){
						// we can create a preferred_scscf_set only if we have an initial S-CSCF Name
			%>			
						<tr bgcolor="#FFCC66">
							<td> S-CSCF </td>							
							<td>
								<html:text property="scscf_name" styleClass="inputtext" value="" /> </td>
							</td>
						</tr>
						
						<tr bgcolor="#FFCC66">
							<td>
								Priority	
							</td>
							<td>	
								<html:text property="priority" styleClass="inputtext"/>
							</td>
						</tr>
			<%
						}
			%>			
						</table>
					</td>
				</tr>
	 			<tr>
 					<td>
						<table align="center">			
						<tr>
							<td align=center> <br/>
								<html:button property="save_button" value="Save" onclick="add_action_for_form(1);"/>				
								<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(2);"/> 
								<% if (id_set == -1){ %>
									<html:button property="reset_button" value="Reset" onclick="add_action_for_form(3);"/> 
								<%}%>
								<% if (id_set != -1){ %>
									<html:button property="delete_button" value="Delete" onclick="add_action_for_form(4);" 
										disabled="<%=Boolean.parseBoolean((String)request.getAttribute("deleteDeactivation")) %>"/>				
								<%}%>									
							</td>
						</tr>
						</table>
					</td>
				</tr>							
				</table>		
			</td>
		</tr>
		<tr>
		
			<%
					if (id_set != -1){		
			%>
				<td>	
					<table width="400">
					<tr>
						<td>	
							<b>Add S-CSCF </b>
						</td>
					</tr>			
							
					<tr>
						<td>
							<html:text property="scscf_name" styleClass="inputtext" value=""/> </td>
						</td>

						<td>
							Priority	
						</td>
						<td>	
							<html:text property="priority" styleClass="inputtext"/>
						</td>
						
						<td>
							<html:button property="scscf_add_button" value="Add" onclick="add_action_for_form(12, -1);"/>
							<br />
						</td>
					</tr>	
					</table>
					
					<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
					<tr class="header">
						<td class="header"> S-CSCF Name </td>
						<td class="header"> Priority </td>
						<td class="header"> Delete </td>
					</tr>
					<%
						if (scscf_list != null){
							Iterator it = scscf_list.iterator();
							Preferred_SCSCF_Set preferred_scscf_set = null;
							int idx = 0;
							while (it.hasNext()){
								preferred_scscf_set = (Preferred_SCSCF_Set) it.next();
					%>
							<tr class="<%= idx % 2 == 0 ? "even" : "odd" %>">																			
								<td>  
									<%= preferred_scscf_set.getScscf_name() %>
								</td>

								<td>  
									<%=preferred_scscf_set.getPriority()%>
								</td>
								
								<td> 
									
									<%
										if (((String)request.getAttribute("deleteSCSCFDeactivation")).equals("true")){
									%>
											<input type="button" name="delete_scscf" "value="Delete" onclick="add_action_for_form(5, <%= preferred_scscf_set.getId() %>);" disabled/>	
									<%
										}
										else{
									%>
											<input type="button" name="delete_scscf" "value="Delete" onclick="add_action_for_form(5, <%= preferred_scscf_set.getId() %>);" />										
									<%
										}
									%>												
								</td>
							</tr>											
					<%			
								idx++;												
								}
							}
					%>
					</table>
					
				</td>	
			<%
				}
			%>	
		</tr>
		</table>		
	</html:form>
</body>
</html>
