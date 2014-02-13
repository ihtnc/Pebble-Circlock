var enable_logging = true;
var invert = 0;
var bt = 0;
var show = 0;

Pebble.addEventListener("ready", function() {
	var temp_mode = localStorage.getItem("InvertMode");
	if (temp_mode) { 
		invert = temp_mode; 
		
		if(enable_logging) console.log("Pebble.ready: InvertMode=" + invert);
	}
	else {
		if(enable_logging) console.log("Pebble.ready: default InvertMode=" + invert);
	}
	
	var temp_show = localStorage.getItem("ShowMode");
	if (temp_show) { 
		show = temp_show; 
		
		if(enable_logging) console.log("Pebble.ready: ShowMode=" + show);
	}
	else {
		if(enable_logging) console.log("Pebble.ready: default ShowMode=" + show);
	}
	
	var temp_bt = localStorage.getItem("BTNotification");
	if (temp_bt) { 
		bt = temp_bt; 
		
		if(enable_logging) console.log("Pebble.ready: BTNotification=" + bt);
	}
	else {
		if(enable_logging) console.log("Pebble.ready: default BTNotification=" + bt);
	}
});

Pebble.addEventListener("showConfiguration", function(e) {
	var json = 
		"[" +
			"{" +
				"'caption':'Invert Screen'," +
				"'key':'InvertMode'," +
				"'type':'checkbox'," +
				"'initialValue':'" + invert + "'," +		
			"}," +
			"{" +
				"'caption':'Display Info'," +
				"'key':'ShowMode'," +
				"'initialValue':'" + show + "'," +
				"'type':'radiobutton'," +
				"'values':" +
					"[" + 
						"{" + 
							"'text':'None'," +
							"'value':'0'" +
						"}," +
						"{" +
							"'text':'Date'," +
							"'value':'1'" +
						"}," +
						"{" +
							"'text':'Status'," +
							"'value':'2'" +
						"}," +
						"{" +
							"'text':'Date + Status'," +
							"'value':'3'" +
						"}," +
					"]" +
				"}," +
			"{" +
				"'caption':'BT Notification'," + 
				"'key':'BTNotification'," +
				"'type':'checkbox'," +
				"'initialValue':'" + bt + "'" +
			"}" +
		"]";
	var url = "http://ihtnc-pebble-config.azurewebsites.net/?";
	var title = "&title=Circlock+Configuration";
	var fields = "&fields=" + encodeURIComponent(json);
	
	if(enable_logging) console.log("Pebble.showConfiguration: url=" + url + title + fields);
	Pebble.openURL(url + title + fields);
});

Pebble.addEventListener("webviewclosed", function(e) {
	if(!e.response && enable_logging) {
		console.log("Pebble.webviewclosed: no response received");
		return;
	}
	else {
		console.log("Pebble.webviewclosed: response=" + e.response);
	}
	
	var configuration = JSON.parse(e.response);
	if(configuration.action == "cancel") {
		if(enable_logging) console.log("Pebble.webviewclosed: action=cancel");
		return;
	}
	
	if(enable_logging) console.log("Pebble.webviewclosed: action=save");
	
	if(configuration.InvertMode == null) invert = "0";
	else invert = configuration.InvertMode;
	
	localStorage.setItem("InvertMode", invert);
	if(enable_logging) console.log("Pebble.webviewclosed: mode=" + invert);
	
	if(configuration.ShowMode == null) show = "0";
	else show = configuration.ShowMode;
	
	localStorage.setItem("ShowMode", show);
	if(enable_logging) console.log("Pebble.webviewclosed: show=" + show);
	
	
	if(configuration.BTNotification == null) bt = "0";
	else bt = configuration.BTNotification;
	
	localStorage.setItem("BTNotification", bt);
	if(enable_logging) console.log("Pebble.webviewclosed: bt=" + bt);        
	
	//since thinCFG returne everything as string, convert the values we retrieved to int before sending to the pebble watchface
	configuration.InvertMode = parseInt(invert);
	configuration.ShowMode = parseInt(show);
	configuration.BTNotification = parseInt(bt);
	Pebble.sendAppMessage(configuration);
	
	if(enable_logging) console.log("Pebble.sendAppMessage: done");
});
