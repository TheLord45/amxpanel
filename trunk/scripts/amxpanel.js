var curPort;		// The port number the currently processed command depends on
var curCommand;		// The currently command stripped from the port number
var z_index = 0;
var cmdArray =
	{
		"commands": [
			{"cmd":"@WLD-","call":unsupported},
			{"cmd":"@AFP-","call":unsupported},
			{"cmd":"@GCE-","call":unsupported},
			{"cmd":"@APG-","call":doAPG},			// Add a popup to a popup group
			{"cmd":"@CPG-","call":doCPG},			// Clear all popups from a group
			{"cmd":"@DPG-","call":unsupported},
			{"cmd":"@PDR-","call":unsupported},
			{"cmd":"@PHE-","call":unsupported},
			{"cmd":"@PHP-","call":unsupported},
			{"cmd":"@PHT-","call":unsupported},
			{"cmd":"@PPA-","call":unsupported},
			{"cmd":"@PPF-","call":doPPF},			// Popup off
			{"cmd":"@PPG-","call":doPPG},			// Toggle a popup
			{"cmd":"@PPK-","call":doPPK},			// Close popup on all pages
			{"cmd":"@PPM-","call":unsupported},
			{"cmd":"@PPN-","call":doPPN},			// Popup on
			{"cmd":"@PPT-","call":unsupported},
			{"cmd":"@PPX","call":doPPX},			// close all popups on all pages
			{"cmd":"@PSE-","call":unsupported},
			{"cmd":"@PSP-","call":unsupported},
			{"cmd":"@PST-","call":unsupported},
			{"cmd":"PAGE-","call":doPAGE},			// Flip to page
			{"cmd":"PPOF-","call":doPPF},			// Popup off
			{"cmd":"PPOG-","call":unsupported},
			{"cmd":"PPON-","call":doPPN},			// Popup on
			{"cmd":"^ANI-","call":unsupported},
			{"cmd":"^APF-","call":doAPF},			// Add page flip action to button
			{"cmd":"^BAT-","call":doBAT},			// Append non-unicode text.
			{"cmd":"^BAU-","call":unsupported},		// Append unicode text
			{"cmd":"^BCB-","call":unsupported},		// Set the border color to the specified color.
			{"cmd":"^BCF-","call":unsupported},		// Set the fill color to the specified color.
			{"cmd":"^BCT-","call":unsupported},		// Set the text color to the specified color.
			{"cmd":"^BDO-","call":unsupported},		// Set the button draw order.
			{"cmd":"^BFB-","call":unsupported},		// Set the feedback type of the button.
			{"cmd":"^BIM-","call":unsupported},		// Set the input mask for the specified address.
			{"cmd":"^BLN-","call":unsupported},		// Set the number of lines removed equally from the top and bottom of a composite video signal.
			{"cmd":"^BMC-","call":unsupported},		// Button copy command.
			{"cmd":"^BMF-","call":unsupported},		// Set any/all button parameters by sending embedded codes and data.
			{"cmd":"^BMI-","call":unsupported},		// Set thje button mask image.
			{"cmd":"^BML-","call":unsupported},		// Set the maximum length of the text area button.
			{"cmd":"^BMP-","call":doBMP},			// Assign a picture to those buttons with a defined addressrange.
			{"cmd":"^BNC-","call":unsupported},
			{"cmd":"^BNN-","call":unsupported},
			{"cmd":"^BNT-","call":unsupported},
			{"cmd":"^BOP-","call":unsupported},
			{"cmd":"^BOR-","call":unsupported},
			{"cmd":"^BOS-","call":unsupported},
			{"cmd":"^BPP-","call":unsupported},
			{"cmd":"^BRD-","call":unsupported},
			{"cmd":"^BSF-","call":unsupported},
			{"cmd":"^BSP-","call":doBSP},			// Set the button size and position.
			{"cmd":"^BSM-","call":unsupported},
			{"cmd":"^BSO-","call":unsupported},
			{"cmd":"^BVL-","call":unsupported},
			{"cmd":"^BVN-","call":unsupported},
			{"cmd":"^BVP-","call":unsupported},
			{"cmd":"^BVT-","call":unsupported},
			{"cmd":"^BWW-","call":unsupported},
			{"cmd":"^CPF-","call":doCPF},			// Clear all page flips from a button.
			{"cmd":"^DLD-","call":unsupported},
			{"cmd":"^DPF-","call":unsupported},
			{"cmd":"^ENA-","call":unsupported},		// Enable or disable buttons with a set variable text range.
			{"cmd":"^FON-","call":unsupported},
			{"cmd":"^GDI-","call":unsupported},
			{"cmd":"^GIV-","call":unsupported},
			{"cmd":"^GLH-","call":unsupported},
			{"cmd":"^GLL-","call":unsupported},
			{"cmd":"^GRD-","call":unsupported},
			{"cmd":"^GRU-","call":unsupported},
			{"cmd":"^GSC-","call":unsupported},
			{"cmd":"^GSN-","call":unsupported},
			{"cmd":"^ICO-","call":doICO},			// Set the icon to a button.
			{"cmd":"^IRM-","call":unsupported},
			{"cmd":"^JSB-","call":unsupported},
			{"cmd":"^JSI-","call":unsupported},
			{"cmd":"^JST-","call":unsupported},
			{"cmd":"^MBT-","call":unsupported},
			{"cmd":"^MDC-","call":unsupported},
			{"cmd":"^SHO-","call":unsupported},
			{"cmd":"^TEC-","call":unsupported},
			{"cmd":"^TEF-","call":unsupported},
			{"cmd":"^TOP-","call":unsupported},
			{"cmd":"^TXT-","call":unsupported},
			{"cmd":"^UNI-","call":unsupported},
			{"cmd":"^LPC-","call":unsupported},
			{"cmd":"^LPR-","call":unsupported},
			{"cmd":"^LPS-","call":unsupported},
			{"cmd":"?BCP-","call":unsupported},
			{"cmd":"?BCF-","call":unsupported},
			{"cmd":"?BCT-","call":unsupported},
			{"cmd":"?BMP-","call":unsupported},
			{"cmd":"?BOP-","call":unsupported},
			{"cmd":"?BRD-","call":unsupported},
			{"cmd":"?BWW-","call":unsupported},
			{"cmd":"?FON-","call":unsupported},
			{"cmd":"?ICO-","call":unsupported},
			{"cmd":"?JSB-","call":unsupported},
			{"cmd":"?JSI-","call":unsupported},
			{"cmd":"?JST-","call":unsupported},
			{"cmd":"?TEC-","call":unsupported},
			{"cmd":"?TEF-","call":unsupported},
			{"cmd":"?TXT-","call":unsupported},
			{"cmd":"ABEEP","call":unsupported},
			{"cmd":"ADBEEP","call":unsupported},
			{"cmd":"@AKB-","call":unsupported},
			{"cmd":"AKEYB-","call":unsupported},
			{"cmd":"AKEYP-","call":unsupported},
			{"cmd":"AKEYR-","call":unsupported},
			{"cmd":"@AKP-","call":unsupported},
			{"cmd":"@AKR","call":unsupported},
			{"cmd":"BEEP","call":unsupported},
			{"cmd":"BRIT-","call":unsupported},
			{"cmd":"@BRT-","call":unsupported},
			{"cmd":"DBEEP","call":unsupported},
			{"cmd":"@EKP-","call":unsupported},
			{"cmd":"PKEYP-","call":unsupported},
			{"cmd":"@PKP-","call":unsupported},
			{"cmd":"SETUP","call":unsupported},
			{"cmd":"SHUTDOWN","call":unsupported},
			{"cmd":"SLEEP","call":unsupported},
			{"cmd":"@SOU-","call":unsupported},
			{"cmd":"@TKP-","call":unsupported},
			{"cmd":"TPAGEON","call":unsupported},
			{"cmd":"TPAGEOFF","call":unsupported},
			{"cmd":"@VKB","call":unsupported},
			{"cmd":"WAKE","call":unsupported},
			{"cmd":"^CAL","call":unsupported},
			{"cmd":"^KPS-","call":unsupported},
			{"cmd":"^VKS-","call":unsupported},
			{"cmd":"@PWD-","call":unsupported},
			{"cmd":"^PWD-","call":unsupported},
			{"cmd":"^BBR-","call":unsupported},
			{"cmd":"^RAF-","call":unsupported},
			{"cmd":"^RFR-","call":unsupported},
			{"cmd":"^RMF-","call":unsupported},
			{"cmd":"^RSR-","call":unsupported},
			{"cmd":"^MODEL?","call":unsupported},
			{"cmd":"^ICS-","call":unsupported},
			{"cmd":"^ICE-","call":unsupported},
			{"cmd":"^ICM-","call":unsupported},
			{"cmd":"^PHN-","call":unsupported},
			{"cmd":"?PHN-","call":unsupported}
		]
	};

function unsupported(msg)
{
	var pos;
	var bef;

	pos = msg.search('-');

	if (pos >= 0)
		bef = msg.substr(0, pos);
	else
		bef = msg;

	console.log("Command "+bef+" is currently not supported!");
}
function newZIndex()
{
	z_index = z_index + 1;
	console.log("+zIndex="+z_index);
	return z_index;
}
function freeZIndex()
{
	if (z_index > 0)
		z_index = z_index - 1;
	console.log("-zIndex="+z_index);
}
function splittCmd(msg)
{
	var pos;

	pos = msg.indexOf('|');
	
	if (pos > 0)
	{
		curPort = parseInt(msg.substr(0, pos));
		curCommand = msg.substr(pos+1);
	}
	else
	{
		curPort = 0;
		curCommand = msg;
	}
}
function getField(msg, field, sep)
{
	var flds;
	var rest;
	var pos;

	pos = msg.indexOf('-');			// Check for the command part preceding the parameters

	if (pos >= 0)
		rest = msg.substr(pos+1);	// Cut off the command part
	else
		rest = msg;					// No command part, so take the whole string

	flds = rest.split(sep);			// Split in fields

	if (flds.length < field)
		return "";

	return flds[field];
}
function getRange(sr)
{
	var narr;
	var sp1;
	var sp2;
	var i;
	var a;

	narr = [];

	if (sr.indexOf('&') < 0 && sr.indexOf('.') < 0)
	{
		narr.push(parseInt(sr));
		return narr;
	}

	if (sr.indexOf('&') >= 0)
	{
		sp1 = sr.split('&');

		for (i = 0; i < sp1.length; i++)
		{
			if (sp1[i].indexOf('.') >= 0)
			{
				sp2 = sp1[i].split('.');

				for (a = parseInt(sp2[0]); a < parseInt(sp2[1]); a++)
					narr.push(a);
			}
			else
				narr.push(parseInt(sp1[i]));
		}
	}
	else
	{
		sp1 = sr.split('.');

		for (a = parseInt(sp1[0]); a < parseInt(sp1[1]); a++)
			narr.push(a);
	}

	return narr;
}
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

	return -1;
}
function findButton(num)
{
	var bt;
	var btArray;
	var i;

	btArray = [];

	for (i in buttonArray.buttons)
	{
		bt = buttonArray.buttons[i];

		if (bt.cp == curPort && bt.ch == num)
			btArray.push(buttonArray.buttons[i]);
	}

	return btArray;
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
function getIconFile(id)
{
	var i;

	for (i in iconArray.icons)
	{
		if (iconArray.icons[i].id == id)
			return iconArray.icons[i].file;
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
			var idx = getPopupIndex(group[i]);

			if (idx >= 0)
			{
				if (Popups.pages[idx].active == true)
					freeZIndex();

				Popups.pages[idx].active = false;
				Popups.pages[idx].lnpage = "";
			}
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
		document.getElementById(pname).style.zIndex = newZIndex();
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
		document.getElementById(pname).style.zIndex = newZIndex();
		idx = getPopupIndex(name);

		if (idx >= 0)
		{
			Popups.pages[idx].active = true;
			Popups.pages[idx].lnpage = getActivePageName();
		}
	}
	catch(e)
	{
		console.log('showPopupOnPage: Error on name <'+name+'> and page '+pname+': '+e);
	}
}
function hidePopupOnPage(name, pg)
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
		document.getElementById(pname).style.display = 'none';
		idx = getPopupIndex(name);

		if (idx >= 0)
		{
			if (Popups.pages[idx].active)
				freeZIndex();

			Popups.pages[idx].active = false;
			Popups.pages[idx].lnpage = getActivePageName();
		}
	}
	catch(e)
	{
		console.log('hidePopupOnPage: Error on name <'+name+'> and page '+pname+': '+e);
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
		{
			if (Popups.pages[idx].active)
				freeZIndex();

			Popups.pages[idx].active = false;
			Popups.pages[idx].lnpage = getActivePageName();
		}
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
			{
				document.getElementById(pname).style.display = 'none';
				freeZIndex();
			}
			else if (Popups.pages[i].active)
			{
				document.getElementById(pname).style.display = 'inline-block';
				document.getElementById(pname).style.zIndex = newZIndex();
			}
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
			{
				document.getElementById(pname).style.display = 'none';
				freeZIndex();
			}
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
function readText(port, channel)
{
	var bt;
	var name;

	curPort = port;

	if ((bt = findButton(channel)) === -1)
	{
		console.log('readText: Error button '+channel+' not found!');
		return;
	}

	name = 'Page'+bt.pnum+'_b1_button_'+bt.bi+'_font';

	try
	{
		return document.getElementById(name).innerHTML;
	}
	catch(e)
	{
		console.log("readText: No element of name "+name+" found!");
	}

	return "";
}
function readTextInst(port, channel, inst)
{
	var bt;
	var i;
	var inum;
	var name;

	curPort = port;

	if ((bt = findButton(channel)) === -1)
	{
		console.log('readText: Error button '+channel+' not found!');
		return;
	}

	for (i = 1; i <= bt.instances; i++)
	{
		if (i == inst)
		{
			name = 'Page'+bt.pnum+'_b'+i+'_button_'+bt.bi+'_font';

			try
			{
				return document.getElementById(name).innerHTML;
			}
			catch(e)
			{
				console.log("readText: No element of name "+name+" found!");
				break;
			}
		}
	}

	return "";
}
function writeText(port, channel, text)
{
	var bt;
	var i;
	var name;

	curPort = port;

	if ((bt = findButton(channel)) === -1)
	{
		console.log('writeText: Error button '+channel+' not found!');
		return;
	}

	for (i = 1; i <= bt.instances; i++)
	{
		name = 'Page'+bt.pnum+'_b'+i+'_button_'+bt.bi+'_font';

		try
		{
			document.getElementById(name).innerHTML = text;
		}
		catch(e)
		{
			console.log("writeText: No element of name "+name+" found!");
		}
	}

	return "";
}
function writeTextInst(port, channel, inst, text)
{
	var bt;
	var i;
	var name;

	curPort = port;

	if ((bt = findButton(channel)) === -1)
	{
		console.log('writeText: Error button '+channel+' not found!');
		return;
	}

	for (i = 1; i <= bt.instances; i++)
	{
		if (i == inst)
		{
			name = 'Page'+bt.pnum+'_b'+i+'_button_'+bt.bi+'_font';

			try
			{
				document.getElementById(name).innerHTML = text;
			}
			catch(e)
			{
				console.log("writeText: No element of name "+name+" found!");
			}
		}
	}

	return "";
}
function doAPG(msg)
{
	var pg;
	var name;
	var i;
	var have;
	var group;

	pg = getField(msg, 0, ";");
	name = getField(msg, 1, ";");

	if (pg.length > 0 && group.length > 0)
	{
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

	pg = "";
	pos = msg.indexOf(";");			// Do we have a page name?
									// FIXME: Page names are not supported currently!
	if (pos < 0)
		pos = msg.length;
	else
	{
		pg = msg.substr(pos+1);
		pos = msg.length - pos - 5;
	}

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
	var pg;

	pg = "";
	pos = msg.indexOf(";");			// Do we have a page name?
									// FIXME: Page names are not supported currently!
	if (pos < 0)
		pos = msg.length - 5;
	else
	{
		pg = msg.substr(pos+1);
		pos = msg.length - pos - 5;
	}

	name = msg.substr(5, pos);		// Extract the popup name

	if (pg.length > 0)
		hidePopupOnPage(name, pg);
	else
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
function doAPF(msg)
{
	var name;
	var cmd;
	var addr;
	var addrRange;
	var bt;
	var i;
	var j;
	var b;
	var num;

	addr = getField(msg, 0, ',');
	addrRange = getRange(addr);
	cmd = getField(msg, 1, ',');
	name = getField(msg, 2,',');

	for (i = 0; i < addrRange.length; i++)
	{
		bt = findButton(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doAPF: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (b = 0; b < bt.length; b++)
		{
			if (cmd.search('Show') >= 0)
			{
				num = bt[b].instances;

				for (j = 0; j < num; j++)
					document.getElementById('Page'+bt[b].pnum+'Button_'+bt[b].bi).addEventListener("click", showPopup(name));
			}
			else if (cmd.search('Hide') >= 0)
			{
				num = bt[b].instances;

				for (j = 0; j < num; j++)
					document.getElementById('Page'+bt[b].pnum+'Button_'+bt[b].bi).addEventListener("click", hidePopup(name));
			}
			// FIXME: There are more commands!
		}
	}
}
function doBAT(msg)
{
	var bt;
	var addr;
	var bts;
	var addrRange;
	var btRange;
	var text;
	var bt;
	var i;
	var j;
	var z;
	var b;
	var name;
	var elem;

	addr = getField(msg, 0, ',');
	bts = getField(msg, 1, ',');
	text = getField(msg, 2, ',');

	addrRange = getRange(addr);
	btRange = getRange(bts);

	for (i = 0; i < addrRange.length; i++)
	{
		bt = findButton(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doBAT: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (b = 0; b < bt.length; b++)
		{
			if (btRange.length == 1 && btRange[0] == 0)
			{
				elem = readText(curPort, addrRange[i]);
				writeText(curPort, addrRange[i], elem+text);
				break;
			}

			for (z = 1; z <= bt[b].instances; z++)
			{
				for (j = 0; j < btRange.length; j++)
				{
					if (btRange[j] == z)
					{
						name = 'Page'+bt[b].pnum+'_b'+z+'_Button_'+bt[b].bi+'_font';

						try
						{
							elem = document.getElementById(name).innerHTML;
							elem = elem + text;
							document.getElementById(name).innerHTML = elem;
						}
						catch(e)
						{
							console.log("doBAT: No element of name "+name+" found!");
						}
					}
				}
			}
		}
	}
}
function doBMP(msg)
{
	var bt;
	var addr;
	var bts;
	var addrRange;
	var btRange;
	var img;
	var bt;
	var i;
	var j;
	var z;
	var b;
	var name;

	addr = getField(msg, 0, ',');
	bts = getField(msg, 1, ',');
	img = getField(msg, 2, ',');

	addrRange = getRange(addr);
	btRange = getRange(bts);

	for (i = 0; i < addrRange.length; i++)
	{
		bt = findButton(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doBMI: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (i = 0; i < bt.length; b++)
		{
			for (z = 1; z <= bt[b].instances; z++)
			{
				for (j = 0; j < btRange.length; j++)
				{
					if ((btRange.length == 1 && btRange[0] == 0) || btRange[j] == z)
					{
						name = 'Page'+bt[b].pnum+'_b'+z+'_Button_'+bt[b].bi;

						try
						{
							document.getElementById(name).src = img;
						}
						catch(e)
						{
							console.log("doBMI: No element of name "+name+" found!");
						}
					}
				}
			}
		}
	}
}
function doBSP(msg)
{
	var bt;
	var b;
	var addr;
	var addrRange;
	var left;
	var top;
	var right;
	var bottom;
	var i;
	var name;

	addr = getField(msg, 0, ',');
	left = getField(msg, 1, ',');
	top = getField(msg, 2, ',');
	right = getField(msg, 3, ',');
	bottom = getField(msg, 4, ',');

	addrRange = getRange(addr);

	for (i = 0; i < addrRange.length; i++)
	{
		bt = findButton(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doBSP: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (b = 0; b < bt.length; b++)
		{
			name = 'Page'+bt[b].pnum+'_b'+z+'_Button_'+bt[b].bi;

			try
			{
				document.getElementById(name).style.left = left;
				document.getElementById(name).style.top = left;
				document.getElementById(name).style.width = right - left;
				document.getElementById(name).style.height = bottom - top;
			}
			catch(e)
			{
				console.log("doBSP: No element of name "+name+" found!");
			}
		}
	}
}
function doCPF(msg)
{
	var bt;
	var addr;
	var addrRange;
	var name;
	var b;

	addr = getField(msg, 0, ',');
	addrRange = getRange(addr);

	for (i = 0; i < addrRange.length; i++)
	{
		bt = findButton(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doCPF: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (b = 0; b < bt.length; b++)
		{
			name = 'Page'+bt[b].pnum+'Button_'+bt[b].bi;

			try
			{
				document.getElementById(name).addEventListener('click', null);
			}
			catch(e)
			{
				console.log("doCPF: No element of name "+name+" found!");
			}
		}
	}
}
function doENA(msg)
{
	var bt;
	var addr;
	var addrRange;
	var val;
	var name;
	var b;

	addr = getField(msg, 0, ',');
	val = getField(msg, 1, ',');
	addrRange = getRange(addr);

	for (i = 0; i < addrRange.length; i++)
	{
		bt = findButton(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doCPF: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (b = 0; b < bt.length; b++)
		{
			name = 'Page'+bt[b].pnum+'Button_'+bt[b].bi;

			try
			{
				if (val == 0)
					document.getElementById(name).style.display = 'none';
				else
					document.getElementById(name).style.display = 'inline';
			}
			catch(e)
			{
				console.log("doCPF: No element of name "+name+" found!");
			}
		}
	}
}
function doICO(msg)
{
	var bt;
	var addr;
	var bts;
	var addrRange;
	var btRange;
	var idx;
	var bt;
	var i;
	var j;
	var z;
	var b;
	var name;
	var stat;

	addr = getField(msg, 0, ',');
	bts = getField(msg, 1, ',');
	idx = getField(msg, 2, ',');

	addrRange = getRange(addr);
	btRange = getRange(bts);

	for (i = 0; i < addrRange.length; i++)
	{
		bt = findButton(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doICO: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (b = 0; b < bt.length; b++)
		{
			for (z = 1; z <= bt[b].instances; z++)
			{
				for (j = 0; j < btRange.length; j++)
				{
					if ((btRange.length == 1 && btRange[0] == 0) || btRange[j] == z)
					{
						name = 'Page'+bt[b].pnum+'_b'+z+'_Button_'+bt[b].bi;

						try
						{
							document.getElementById(name+'_icon').src = "images/"+getIconFile(idx);
						}
						catch(e)
						{
							try
							{
								var elem = document.getElementById(name);
								var cWidth = elem.clientWidth;
								var cHeight = elem.clientHeight;
								elem.innerHTML = '<img id="'+name+'_icon" src="images/'+getIconFile(idx)+'" onload="imgCenter(this,\''+name+'\');">' + elem.innerHTML;
								imgCenter(document.getElementById(name+'_icon'), cWidth, cHeight);
							}
							catch(e)
							{
								console.log("doICO: No element of name "+name+" found! ["+e+"]");
							}
						}
					}
				}
			}
		}
	}
}
function parseMessage(msg)
{
	var i;

	splittCmd(msg);

	for (i in cmdArray.commands)
	{
		try
		{
			if (curCommand.indexOf(cmdArray.commands[i].cmd) >= 0)
			{
				cmdArray.commands[i].call(curCommand);
				break;
			}
		}
		catch(e)
		{
			console.log("parseMessage WARNING: Position: "+i+": Error: "+e);
		}
	}
}
function imgCenter(img, name)
{
	var pw;
	var ph;
	var iw;
	var ih;
	var elem;

	elem = document.getElementById(name);

	if (elem === null)
		return;

	pw = elem.clientWidth;
	ph = elem.clientHeight;
	iw = img.width;
	ih = img.height;

	if (pw == 0)
		pw = iw;

	if (ph == 0)
		ph = ih;

	if (iw == 0 || ih == 0)
	{
		console.log("imgCenter: Image with no size at "+name+"!");
		return;
	}

	img.style.position = 'absolute';

	if (pw > iw)
		img.style.left = (pw - iw) / 2 + 'px';

	if (ph > ih)
		img.style.top = (ph - ih) / 2 + 'px';
}
function posImage(img, name, code)
{
	var pw;
	var ph;
	var iw;
	var ih;
	var elem;

	elem = document.getElementById(name);

	if (elem === null)
		return;

	pw = elem.clientWidth;
	ph = elem.clientHeight;
	iw = img.width;
	ih = img.height;

	if (pw == 0)
		pw = iw;

	if (ph == 0)
		ph = ih;

	if (iw == 0 || ih == 0)
	{
		console.log("imgCenter: Image with no size at "+name+"!");
		return;
	}

	img.style.position = 'absolute';
	
	switch (code)
	{
		case 2:	// center, top
			img.style.top = "0px";

			if (pw > iw)
				img.style.left = (pw - iw) / 2 + 'px';
		break;
		
		case 3:	// right, top
			img.style.top = "0px";

			if (pw >= iw)
				img.style.left = (pw - iw) + 'px';
		break;
		
		case 4:	// left, middle
			img.style.left = "0px";

			if (ph > ih)
				img.style.top = (ph - ih) / 2 + 'px';
		break;
		
		case 6:	// right, middle
			if (pw >= iw)
				img.style.left = (pw - iw) + 'px';

			if (ph > ih)
				img.style.top = (ph - ih) / 2 + 'px';
		break;

		case 7:	// left, bottom
			img.style.left = "0px";

			if (ph >= ih)
				img.style.top = (ph - ih) + 'px';
		break;

		case 8:	// center, bottom
			if (pw > iw)
				img.style.left = (pw - iw) / 2 + 'px';

			if (ph >= ih)
				img.style.top = (ph - ih) + 'px';
		break;

		case 9:	// right, bottom
			if (pw >= iw)
				img.style.left = (pw - iw) + 'px';

			if (ph >= ih)
				img.style.top = (ph - ih) + 'px';
		break;

		default:	// center, middle
			if (pw > iw)
				img.style.left = (pw - iw) / 2 + 'px';

			if (ph > ih)
				img.style.top = (ph - ih) / 2 + 'px';
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

	setWiFi();
	parseMessage('1|@PPN-topmenu');
	parseMessage('1|^ICO-1071,0,23');
	parseMessage('1|^ICO-1072,0,24');
	parseMessage('1|^ICO-1073,0,27');
	parseMessage('1|^ICO-1078,0,1');
	parseMessage('1|@PPN-source_dreambox');
}

function writeTextOut(msg)
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
	if (i < 10)	
		i = "0" + i;

	return i;
}
navigator.getBattery().then(function(battery)
{
	function updateAllBatteryInfo()
	{
		updateChargeInfo();
		updateLevelInfo();
		console.log("navigator.getBattery()");
	}

	updateAllBatteryInfo();

	battery.addEventListener('chargingchange', function()
	{
		updateChargeInfo();
	});

	function updateChargeInfo()
	{
		try
		{
			if (battery.charging)
			{
				document.getElementById('batCharge1').style.display = "none";
				document.getElementById('batCharge2').style.display = "inline";
			}
			else
			{
				document.getElementById('batCharge1').style.display = "inline";
				document.getElementById('batCharge2').style.display = "none";
			}
		}
		catch(e)
		{
			console.log("updateChargeInfo: "+e);
		}
	}

	battery.addEventListener('levelchange', function()
	{
		updateLevelInfo();
	});
  
	function updateLevelInfo()
	{
		// FIXME: Add code (canvas?) to draw the level of the bargraph.
		console.log("Battery level: " + battery.level * 100 + "%");
	}
});
function setWiFi()
{
	var connection = window.navigator.connection ||
					window.navigator.mozConnection ||
					null;

	if (connection === null)
	{
		console.log("setWiFi: WiFi API not supported!");
		return;
	}

	if ('metered' in connection)	// Old style?
	{
		document.getElementById('nio-supported').classList.remove('hidden');
		[].slice.call(document.getElementsByClassName('old-api')).forEach(function(element)
		{
			element.classList.remove('hidden');
		});
 
//		var bandwidthValue = document.getElementById('b-value');
//		var meteredValue = document.getElementById('m-value');
 
		connection.addEventListener('change', function (event)
		{
			console.log("Band width value: "+connection.bandwidth);
			console.log("Metered value:"+(connection.metered ? '' : 'not ') + 'metered');
		});

		connection.dispatchEvent(new Event('change'));
	}
	else
	{
//		var typeValue = document.getElementById('t-value');

		[].slice.call(document.getElementsByClassName('new-api')).forEach(function(element)
		{
//			element.classList.remove('hidden');
		});
 
		connection.addEventListener('typechange', function (event)
		{
			console.log("Connection type: "+connection.type);
		});

		connection.dispatchEvent(new Event('typechange'));	
	}
}