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

	for (i in Pages.pages)
	{
		if (Pages.pages[i].name == name)
			return Pages.pages[i].ID;
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
function getPopupIndex(name)
{
	var i;

	for (i in Popups.pages)
	{
		if (Popups.pages[i].name == name)
			return i;
	}

	return -1;
}
function getPageIndex(name)
{
	var i;

	for (i in Pages.pages)
	{
		if (Pages.pages[i].name == name)
			return i;
	}

	return -1;
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

	for (i in basePages)
	{
		if (Pages.pages[i].name == name)
			return Pages.pages[i].active;
	}

	return false;
}
function getActivePage()
{
	var name;

	for (i in Pages.pages)
	{
		if (Pages.pages[i].active == true)
			return Pages.pages[i].ID;
	}

	return 0;
}
function getActivePageName()
{
	var name;
	
	for (i in Pages.pages)
	{
		if (Pages.pages[i].active)
			return Pages.pages[i].name;
	}

	return "";
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
	var idx;

	pID = findPopupNumber(name);
	group = findPageGroup(name);
	pname = "Page_"+pID;
	hideGroup(group);

	try
	{
		document.getElementById(pname).style.display = 'inline-block';
		idx = getPopupIndex(name);

		if (idx >= 0)
		{
			Popups.pages[idx].active = true;
			Popups.pages[idx].lnpage = getActivePageName();
		}
	}
	catch(e)
	{
		console.log('showPopup: Error on name <'+name+'> and page '+pname+': '+e);
	}
}
function showPopupOnPage(name, pg)
{
	var pname;
	var pID;
	var group;
	var idx;

	pID = findPopupNumber(name);
	group = findPageGroup(name);
	pname = "Page_"+pID;
	hideGroup(group);

	try
	{
		document.getElementById(pname).style.display = 'inline-block';
		idx = getPopupIndex(name);

		if (idx >= 0)
		{
			Popups.pages[idx].active = true;
			Popups.pages[idx].lnpage = getActivePageName();
		}
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
	var idx;

	pID = findPopupNumber(name);
	pname = "Page_"+pID;

	try
{
		document.getElementById(pname).style.display = 'none';
		idx = getPopupIndex(name);

		if (idx >= 0)
			Popups.pages[idx].active = false;
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

	if (pID > 0)
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
		
		for (i in Popups.pages)
		{
			pname = "Page_"+Popups.pages[i].ID;

			if (Popups.pages[i].active && Popups.pages[i].lnpage != name)
				document.getElementById(pname).style.display = 'none';
			else if (Popups.pages[i].active)
				document.getElementById(pname).style.display = 'inline-block';
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

		for (i in Popups.pages)
		{
			pname = "Page_"+Popups.pages[i].ID;

			if (Popups.pages[i].active && Popups.pages[i].lnpage != name)
				document.getElementById(pname).style.display = 'none';
		}
	}
	catch(e)
	{
		console.log('hidePage: Error on name <'+name+'> and page '+pname+': '+e);
	}
}
function switchDisplay(name1, name2, dStat, cport, cnum)
{
	try
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
	catch(e)
	{
		console.log('switchDisplay: Error: name1='+name1+', name2='+name2+', dStat='+dStat+', cport='+cport+', cnum='+cnum);
	}
}
function doAPG(msg)
{
	var pos;
	var pg;
	var name;
	var i;
	var have;
	var group;

	pos = msg.indexOf(";");

	if (pos > 0)
	{
		name = msg.substr(5, pos - 1);
		pg = msg.substr(pos+1);

		for (i in popupGroups)
		{
			if (popupGroups[i] == name)
			{
				have = false;
				group = popupGroups[i];

				for (j in group)
				{
					if (group[j] == pg)
					{
						have = true;
						break;
					}
				}

				if (!have)
					popupGroups[i] += name;
			}
		}
	}
}
function doPPN(msg)
{
	var pos;
	var pg;
	var name;

	pos = msg.indexOf(";");			// Do we have a page name?
									// FIXME: Page names are not supported currently!
	if (pos < 0)
		pos = msg.length;
	else
		pg = msg.substr(pos+1);

	name = msg.substr(5, pos);		// Extract the popup name

	if (pg.length > 0)
		showPopupOnPage(name, pg);
	else
		showPopup(name);
}
function doPPF(msg)
{
	var pos;
	var name;

	pos = msg.indexOf(";");			// Do we have a page name?
									// FIXME: Page names are not supported currently!
	if (pos < 0)
		pos = msg.length;

	name = msg.substr(5, pos);		// Extract the popup name
	hidePopup(name);
}
function doPPG(msg)
{
	var pos;
	var name;

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
function doPPK(msg)
{
	var name;
	var group;

	name = msg.substr(5);			// Extract the popup name
	group = findPageGroup(name);

	if (group.length > 0)
		hideGroup(group);
	else
		hidePopup(name);
}
function doPPX(msg)
{
	var name;
	var ID;

	for (i in Popups.pages)
	{
		ID = Popups.pages[i].ID;
		name = "Page_"+ID;
		hidePopup(name);
	}
}
function doPAGE(msg)
{
	var name;

	name = msg.substr(5);
	showPage(name);
}
function doCPG(msg)
{
	var i;
	var group;
	var name;

	name = msg.substr(5);
	group = popupGroups[name];

	if (group.length == 0)
		return;

	for (i in group)
		delete group[i];
}
function parseMessage(msg)
{
	var name;
	var pg;
	var pos;
	var idx;

	pg = "";

	if (msg.startsWidth("@APG-"))		// Add a popup to a popup group
		doAPG(msg);
	else if (msg.startsWidth("@CPG-"))	// Clear all popups from a group
		doCPG(msg);
	else if (msg.startsWidth("@PPN-") || msg.startsWidth("PPON-"))	// Popup on
		doPPN(msg);
	else if (msg.startsWidth("@PPF-") || msg.startsWidth("PPOF-"))	// Popup off
		doPPF(msg);
	else if (msg.startsWidth("@PPG-") || msg.startsWidth("PPOG-"))	// Toggle a popup
		doPPG(msg);
	else if (msg.startsWidth("@PPK-"))	// Close popup on all pages
		doPPK(msg);
	else if (msg == "@PPX")				// close all popups on all pages
		doPPX(msg);
	else if (msg.startsWidth("PAGE-"))	// Flip to page
		doPAGE(msg);
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
		console.log("WARNING: Socket not ready!");
		return;
	}
	wsocket.send(msg);
}
function checkTime(i)
{
	if (i < 10) {i = "0" + i};
	return i;
}

