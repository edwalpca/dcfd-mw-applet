<%@ page contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<html>
<head>
<title>eID Applet Signature Test</title>
</head>
<body>
<h1>eID Applet Signature Test</h1>
<p>This test will simulate untrusted eID signing certificates to
test the error reporting.</p>
<script src="https://www.java.com/js/deployJava.js"></script>
<script>
	var attributes = {
		code :'be.fedict.eid.applet.Applet.class',
		archive :'eid-applet-package-${pom.version}.jar',
		width :600,
		height :300
	};
	var parameters = {
		TargetPage :'sign-result.jsp',
		AppletService :'applet-service-sign-untrusted'
	};
	var version = '1.6';
	deployJava.runApplet(attributes, parameters, version);
</script>
</body>
</html>