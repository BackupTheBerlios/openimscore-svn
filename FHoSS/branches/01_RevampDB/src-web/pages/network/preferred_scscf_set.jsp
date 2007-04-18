<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="de.fhg.fokus.hss.util.SecurityPermissions" %>
<html>
<head>

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> Charging Info Sets </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<%
	String action=request.getParameter("action");
	int id = Integer.parseInt(request.getParameter("id"));
%>

<script type="text/javascript" language="JavaScript">
function add_action_for_form(a) {
	switch(a){
		case 1:
			document.CS_Form.nextAction.value="save";
			document.CS_Form.submit();
			break;
		case 2:
			document.CS_Form.nextAction.value="refresh";
			document.CS_Form.submit();			
			break;
		case 3:
			document.CS_Form.nextAction.value="reset";
			document.CS_Form.reset();
			break;
		case 4:
			document.CS_Form.nextAction.value="delete";
			document.CS_Form.submit();
			break;
	}
}
</script>
</head>

<body>
	<table align=center valign=middle height=100%>
		<!-- Print errors, if any -->
		<tr>
			<td>
				<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
			</td>
		</tr>
		
		<html:form action="/CS_Submit">
			<html:hidden property="nextAction" value=""/>
			<tr>
				<td align="center"><h1>Charging Info Sets </h1></td>
			</tr>
			<tr>
				<td>
			 		<table border="0" align="center" width="100%" >						
			 		<tr>
			 				<td>
						 		<table border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">						
								<tr bgcolor="#FFCC66">
									<td> ID </td>
									<td><html:text property="id" readonly="true" styleClass="inputtext_readonly"/> </td>
								</tr>
								<tr bgcolor="#FFCC66">
									<td>Name </td>
									<td><html:text property="name" styleClass="inputtext"/> </td>
								</tr>

								<tr bgcolor="#FFCC66">
									<td>Primary CCF </td>
									<td><html:text property="pri_ccf" styleClass="inputtext"/> </td>
								</tr>
								<tr bgcolor="#FFCC66">
									<td>Secondary CCF  </td>
									<td><html:text property="sec_ccf" styleClass="inputtext"/> </td>
								</tr>
								<tr bgcolor="#FFCC66">
									<td>Primary ECF </td>
									<td><html:text property="pri_ecf" styleClass="inputtext"/> </td>
								</tr>
								<tr bgcolor="#FFCC66">
									<td>Secondary ECF  </td>
									<td><html:text property="sec_ecf" styleClass="inputtext"/> </td>
								</tr>
								</table>		
							</td>
					</tr>
					<tr>
						<td>
							<table align="center">
								<tr>
									<td align=center> 
										<html:button property="save_button" value="Save" onclick="add_action_for_form(1, -1);"/>				
										<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(3, -1);"/> 
										<% if (id == -1){ %>
											<html:button property="reset_button" value="Reset" onclick="add_action_for_form(2, -1);"/> 
										<%}%>
						
										<% if (id != -1){ %>
										<html:button property="delete_button" value="Delete" onclick="add_action_for_form(4, -1);" 
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
		</table>			
	</html:form>
</body>
</html>