function findPopupNumber(name)
{
	var i;

	for (i in Popups.pages)
	{
		if (Popups.pages[i].name == name)
			return Popups.pages[i].ID;
	}

	return -1;
}
function findPageNumber(name)
{
	var i;

	for (i in basePages.pages)
	{
		if (basePages.pages[i].name == name)
			return basePages.pages[i].ID;
	}

	return -1;
}
function findPageGroup(name)
{
	var i;

	for (i in Popups.pages)
	{
		if (Popups.pages[i].name == name)
			return Popups.pages[i].group;
	}

	return ;
}
function hideGroup(name)
{
	var nm;
	var group;
	var i;
	group = popupGroups[name];

	if (name == "")
		return;

	for (i in group)
	{
		var pg;
		pg = findPopupNumber(group[i]);
		nm = 'Page_'+pg;

		try
		{
			document.getElementById(nm).style.display = 'none';
		}
		catch(e)
		{
			console.log('hideGroup: Error on name <'+name+'> and page '+nm+': '+e);
		}
	}
}
function showPopup(name)
{
	var pname;
	var pID;
	var group;

	pID = findPopupNumber(name);
	group = findPageGroup(name);
	pname = "Page_"+pID;
	hideGroup(group);

	try
	{
		document.getElementById(pname).style.display = 'inline-block';
	}
	catch(e)
	{
		console.log('showPopup: Error on name <'+name+'> and page '+pname+': '+e);
	}
}
function hidePopup(name)
{
	var pname;
	var pID;

	pID = findPopupNumber(name);
	pname = "Page_"+pID;

	try
{
		document.getElementById(pname).style.display = 'none';
	}
	catch(e)
	{
		console.log('hidePopup: Error on name <'+name+'> and page '+pname+': '+e);
	}
}
function showPage(name)
{
	var pname;
	var pID;

	pID = findPageNumber(name);

	if (pID >= 0)
		pname = "Page_"+pID;
	else
		pname = name;

	try
	{
		var ID;
		ID = getActivePage();

		if (ID > 0)
			document.getElementById("Page_"+ID).style.display = 'none';

		document.getElementById(pname).style.display = 'block';
		
		for (i in pageNames)
		{
			if (pageNames.pages[i].active && pageNames.pages[i].lnpage != name)
				document.getElementById("Page_"+pageNames.pages[i].ID).style.display = 'none';
			else if (pageNames.pages[i].active)
				document.getElementById("Page_"+pageNames.pages[i].ID).style.display = 'inline-block';
		}
	}
	catch(e)
	{
		console.log('showPage: Error on name <'+name+'> and page '+pname+': '+e);
	}
}
function hidePage(name)
{
	var pname;
	var pID;

	pID = findPageNumber(name);

	if (pID >= 0)
		pname = "Page_"+pID;
	else
		pname = name;

	try
	{
		document.getElementById(pname).style.display = 'none';
	}
	catch(e)
	{
		console.log('hidePage: Error on name <'+name+'> and page '+pname+': '+e);
	}
}
function getPopupStatus(name)	// return true if popup is shown
{
	var pID;
	var pname;
	var stat;

	pID = findPopupNumber(name);

	if (pID > 0)
		pname = "Page_"+pID;

	try
	{
		stat = document.getElementById(pname).style.display;
		
		if (stat == "none")
			return false;
		else
			return true;
	}
	catch(e)
	{
		console.log('getPopupStatus: Error on name <'+name+'> and page '+pname+': '+e);
	}
}
function getPageStatus(name)	// return true if popup is shown
{
	var pID;
	var pname;
	var stat;

	pID = findPageNumber(name);

	if (pID > 0)
		pname = "Page_"+pID;

	try
	{
		stat = document.getElementById(pname).style.display;
		
		if (stat == "none")
			return false;
		else
			return true;
	}
	catch(e)
	{
		console.log('getPopupStatus: Error on name <'+name+'> and page '+pname+': '+e);
	}
}
function getActivePage()
{
	var name;

	for (i in basePages)
	{
		name = "Page_"+basePages.pages[i].ID;

		if (document.getElementById(name).style.display != 'none')
			return basePages.pages[i].ID;
	}

	return 0;
}
function switchDisplay(name1, name2, dStat, cport, cnum)
{
	if (dStat == 1)
	{
		document.getElementById(name1).style.display = "none";
		document.getElementById(name2).style.display = "inline";
		writeText("PUSH:"+cport+":"+cnum+":1;");
	}
	else
	{
		document.getElementById(name1).style.display = "inline";
		document.getElementById(name2).style.display = "none";
		writeText("PUSH:"+cport+":"+cnum+":0;");
	}
}
function parseMessage(msg)
{
	var name;
	var pos;

	if (msg.startsWidth("@PPN-"))		// Popup on
	{
		pos = msg.indexOf(";");			// Do we have a page name?
										// FIXME: Page names are not supported currently!
		if (pos < 0)
			pos = msg.length;

		name = msg.substr(5, pos);		// Extract the popup name
		showPopup(name);
	}
	else if (msg.startsWidth("@PPF-"))	// Popup off
	{
		pos = msg.indexOf(";");			// Do we have a page name?
										// FIXME: Page names are not supported currently!
		if (pos < 0)
			pos = msg.length;

		name = msg.substr(5, pos);		// Extract the popup name
		hidePopup(name);
	}
	else if (msg.startsWidth("@PPG-"))	// Toggle a popup
	{
		pos = msg.indexOf(";");			// Do we have a page name?
										// FIXME: Page names are not supported currently!
		if (pos < 0)
			pos = msg.length;

		name = msg.substr(5, pos);		// Extract the popup name

		if (getPopupStatus(name))		// Is popup visible?
			hidePopup(name);
		else
			showPopup(name);
	}
	else if (msg.startsWidth("@PPK-"))	// Close popup on all pages
	{
		var group;

		name = msg.substr(5);			// Extract the popup name
		group = findPageGroup(name);

		if (group.length > 0)
			hideGroup(group);
		else
			hidePopup(name);
	}
	else if (msg == "@PPX")				// close all popups on all pages
	{
		var ID;

		for (i in Popups.pages)
		{
			ID = Popups.pages[i].ID;
			name = "Page_"+ID;
			hidePopup(name);
		}
	}
	else if (msg.startsWidth("PAGE-"))	// Flip to page
	{
		name = msg.substr(5);
		showPage(name);
	}
}
function connect()
{
	try
	{
		wsocket = new WebSocket("wss://www.theosys.at:11012/");
		wsocket.onopen = function() { wsocket.send('READY;'); }
		wsocket.onerror = function(error) { console.log(`WebSocket error: ${error}`); }
		wsocket.onmessage = function(e) { parseMessage(e.data); }
		wsocket.onclose = function() { console.log('WebSocket is closed!'); }
	}
	catch (exception)
	{
		console.error("Error initializing: "+exception);
	}
}

function writeText(msg)
{
	if (wsocket.readyState != WebSocket.OPEN)
	{
		alert("Socket not ready!");
		return;
	}
	wsocket.send(msg);
}
function checkTime(i)
{
	if (i < 10) {i = "0" + i};
	return i;
}

