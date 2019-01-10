function findPageNumber(name)
{
	var i;

	for (i in Popups.pages)
	{
		if (Popups.pages[i].name == name)
			return Popups.pages[i].ID;
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
		pg = findPageNumber(group[i]);
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

	pID = findPageNumber(name);
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

	pID = findPageNumber(name);
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
		document.getElementById(pname).style.display = 'block';
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
function switchDisplay(name1, name2, dStat, bid)
{
	var bname;
	var url;
	if (dStat == 1)
	{
		document.getElementById(name1).style.display = "none";
		document.getElementById(name2).style.display = "inline";
		bname = pageName+":button_"+bid;
		writeText("PUSH:"+bname+":1;");
	}
	else
	{
		document.getElementById(name1).style.display = "inline";
		document.getElementById(name2).style.display = "none";
		bname = pageName+":button_"+bid;
		writeText("PUSH:"+bname+":0;");
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

