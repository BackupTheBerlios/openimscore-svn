<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="de.fhg.fokus.hss.util.SecurityPermissions" %>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<link rel="stylesheet" type="text/css"
	href="/hss.web.console/style/fokus_ngni.css">

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title><bean:message key="result.title" /></title>
</head>
<body>
	<html:form action="TP_Search">
		<table align="center" valign="middle" height="80%">
		<tr> 
			<td align=center>
				<br/><br/><h1>Trigger Point - Search </h1>
			</td>
		</tr>
		<tr height=99%>
			<td align=center>
				<table border=0 cellspacing=0 align="center" width=300>
				<tr><td align="right"><b>Enter Search Parameters:</b></td></tr>
	 			<tr>
	 				<td>
					 	<table border="0" cellspacing="1" align="center" width="100%" style="border:1px solid #FF6600;">
		    			<tr bgcolor="#FFCC66">
							<td>ID</td>
							<td><html:text property="id" value="" styleClass="inputtext" size="8"/></td>
						</tr>
		    			<tr bgcolor="#FFCC66">							
		    				<td>Name</td>
							<td><html:text property="name" value="" styleClass="inputtext" size="32"/></td>
						</tr>
						</table>
					</td>		
				</tr>
				<tr>
					<td align="center"><br/><html:submit property="search" value="Search" /></td>
				</tr>				
				</table>
			</td>
		</tr>		
		</table>		
	</html:form>
</body>
</html>
