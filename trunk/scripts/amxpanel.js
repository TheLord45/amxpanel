const sleep = (milliseconds) => {
	return new Promise(resolve => setTimeout(resolve, milliseconds))
}

var curPort;		// The port number the currently processed command depends on
var curCommand;		// The currently command stripped from the port number
var z_index = 0;
var __debug = false;

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
			{"cmd":"^BCB-","call":doBCB},			// Set the border color to the specified color.
			{"cmd":"^BCF-","call":doBCF},			// Set the fill color to the specified color.
			{"cmd":"^BCT-","call":doBCT},			// Set the text color to the specified color.
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
			{"cmd":"^SHO-","call":doSHO},
			{"cmd":"^TEC-","call":unsupported},
			{"cmd":"^TEF-","call":unsupported},
			{"cmd":"^TOP-","call":unsupported},
			{"cmd":"^TXT-","call":doTXT},
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
			{"cmd":"ABEEP","call":doABEEP},
			{"cmd":"ADBEEP","call":doADBEEP},
			{"cmd":"@AKB-","call":unsupported},
			{"cmd":"AKEYB-","call":unsupported},
			{"cmd":"AKEYP-","call":unsupported},
			{"cmd":"AKEYR-","call":unsupported},
			{"cmd":"@AKP-","call":unsupported},
			{"cmd":"@AKR","call":unsupported},
			{"cmd":"BEEP","call":doABEEP},
			{"cmd":"BRIT-","call":unsupported},
			{"cmd":"@BRT-","call":unsupported},
			{"cmd":"DBEEP","call":doADBEEP},
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
			{"cmd":"?PHN-","call":unsupported},
			{"cmd":"ON-","call":setON},
			{"cmd":"OFF-","call":setOFF},
            {"cmd":"LEVEL-","call":setLEVEL}
		]
	};

function rgb(red, green, blue)
{
	return "rgb("+red+","+green+","+blue+")";
}
function rgba(red, green, blue, alpha)
{
	return "rgba("+red+","+green+","+blue+","+alpha+")";
}
function findFont(id)
{
	var i;

	for (i in fontList.fonts)
	{
		var fnt = fontList.fonts[i];

		if (fnt.number == id)
			return fnt;
	}

	return -1;
}
function getFontStyle(fs)
{
	if (fs == "Regular")
		return "normal";

	if (fs == "Italic" || fs == "Bold Italic")
		return "italic";

	return "normal";
}
function getFontWeight(fw)
{
	if (fw == "Regular")
		return "normal";

	if (fw == "Bold" || fw == "Bold Italic")
		return "bold";

	return "normal";
}
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
	debug("+zIndex="+z_index);
	return z_index;
}
function freeZIndex()
{
	if (z_index > 0)
		z_index = z_index - 1;

	debug("-zIndex="+z_index);
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
	var flds = [];
	var rest;
	var pos;
	var i;
	var bStr = false;
	var part = "";

	pos = msg.indexOf('-');			// Check for the command part preceding the parameters

	if (pos >= 0)
		rest = msg.substr(pos+1);	// Cut off the command part
	else
		rest = msg;					// No command part, so take the whole string

	for (i = 0; i < rest.length; i++)
	{
		if (rest.charAt(i) == sep && !bStr)
		{
			flds.push(part);
			part = "";
			continue;
		}
		else if (rest.charAt(i) == "'" && !bStr)
			bStr = true;
		else if (rest.charAt(i) == "'" && bStr)
			bStr = false;
		else
			part = part + rest.charAt(i);
	}

	if (part.length > 0)
		flds.push(part);

	if (flds.length <= field)
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
function getRGBAColor(name)
{
	var i;
	var nm = name;
	var colArr = [];

	for (i in palette.colors)
	{
		var col = palette.colors[i];

		if (col.name == nm)
		{
			colArr.push(col.red);
			colArr.push(col.green);
			colArr.push(col.blue);
			colArr.push(col.alpha);
			return colArr;
		}
	}

	return -1;
}
function getRGBColor(name)
{
	var i;
	var nm = name;
	var colArr = [];

	for (i in palette.colors)
	{
		var col = palette.colors[i];

		if (col.name == nm)
		{
			colArr.push(col.red);
			colArr.push(col.green);
			colArr.push(col.blue);
			return colArr;
		}
	}

	return -1;
}
function getAMXColor(value)
{
	var alpha = 1;
	var pos = value.indexOf('#');

	if (pos < 0)
		return getRGBAColor(value);

	var red = parseInt(value.substr(1, 2), 16);
	var green = parseInt(value.substr(3, 2), 16);
	var blue = parseInt(value.substr(5, 2), 16);
	var colArr = [];

	colArr.push(red);
	colArr.push(green);
	colArr.push(blue);

	if (value.length > 6)
	{
		alpha = parseInt(value.substr(7, 2), 16);
		colArr.push(alpha);
	}

	return colArr;
}
function getHexColor(value)
{
	var alpha = 1.0;
	var pos = value.indexOf('#');

	if (pos < 0)
		return getRGBAColor(value);

	var red = parseInt(value.substr(1, 2), 16);
	var green = parseInt(value.substr(3, 2), 16);
	var blue = parseInt(value.substr(5, 2), 16);
	var colArr = [];

	colArr.push(red);
	colArr.push(green);
	colArr.push(blue);

	if (value.length > 6)
	{
		alpha = parseInt(value.substr(7, 2), 16);
		alpha = 1.0 / 256.0 * alpha;
		colArr.push(alpha);
	}
	else
		colArr.push(alpha);

	return colArr;
}
function getWebColor(value)
{
	var pos = value.indexOf('#');

	if (pos < 0)
	{
		var col = getRGBAColor(value);
		return rgba(col[0],col[1],col[2],col[3]);
	}

	var red = parseInt(value.substr(1, 2), 16);
	var green = parseInt(value.substr(3, 2), 16);
	var blue = parseInt(value.substr(5, 2), 16);

	if (value.length > 6)
	{
		alpha = parseInt(value.substr(7, 2), 16);
		alpha = 1.0 / 256.0 * alpha;
		return rgba(red,green,blue,alpha);
	}

	return rgb(red,green,blue);
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
function findPageName(num)
{
	var i;

	for (i in Pages.pages)
	{
		if (Pages.pages[i].ID == num)
			return Pages.pages[i].name;
	}

	return -1;
}
function findPopupName(num)
{
	var i;

	for (i in Popups.pages)
	{
		if (Popups.pages[i].ID == num)
			return Popups.pages[i].name;
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
function findButtonPort(num)
{
	var bt;
	var btArray;
	var i;

	btArray = [];

	for (i in buttonArray.buttons)
	{
		bt = buttonArray.buttons[i];

		if (bt.ap == curPort && bt.ac == num)
			btArray.push(buttonArray.buttons[i]);
	}

	return btArray;
}
function findBargraphs(port, channel)
{
	var i;
	var bgArray = [];

	for (i in bargraphs.bargraphs)
	{
		var bg = bargraphs.bargraphs[i];

		if (bg.lp == port && bg.lc == channel)
			bgArray.push(bg);
	}

	return bgArray;
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
		return false;
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

	return -1;
}
function getIconDim(id)
{
	var i;

	for (i in iconArray.icons)
	{
		if (iconArray.icons[i].id == id)
			return [ iconArray.icons[i].width, iconArray.icons[i].height ];
	}

	return -1;
}
function setButtonOnline(pnum, id, stat)
{
	var i;

	for (i in buttonArray.buttons)
	{
		if (buttonArray.buttons[i].pnum == pnum && buttonArray.buttons[i].bi == id)
		{
			buttonArray.buttons[i].ion = stat;
			console.log("setButtonOnline: ion="+buttonArray.buttons[i].ion);
			break;
		}
	}
}
function hideGroup(name)
{
	var nm;
	var group;
	var i;
	group = popupGroups[name];

	debug("hideGroup("+name+")");

	if (name == "")
		return;

	for (i in group)
	{
		var pg;
		pg = findPopupNumber(group[i]);
		nm = 'Page_'+pg;

		try
		{
			var idx = getPopupIndex(group[i]);

			if (idx >= 0)
			{
				if (Popups.pages[idx].active == true)
				{
					dropPopup(Popups.pages[idx].name);
					freeZIndex();
				}

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

	debug("showPopup("+name+")");
	pID = findPopupNumber(name);
	group = findPageGroup(name);
	pname = "Page_"+pID;
	hideGroup(group);

	try
	{
		drawPopup(name);
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
		idx = getPopupIndex(name);

		if (idx >= 0)
		{
			if (!Popups.pages[idx].active)
			{
				drawPopup(name);
				document.getElementById(pname).style.zIndex = newZIndex();
			}

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
		idx = getPopupIndex(name);

		if (idx >= 0)
		{
			if (Popups.pages[idx].active)
			{
				dropPopup(name);
				freeZIndex();
			}

			Popups.pages[idx].active = false;
			Popups.pages[idx].lnpage = "";
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

	debug("hidePopup("+name+")");
	pID = findPopupNumber(name);
	pname = "Page_"+pID;

	try
	{
		idx = getPopupIndex(name);

		if (idx >= 0)
		{
			if (Popups.pages[idx].active)
			{
				dropPopup(name);
				freeZIndex();
			}

			Popups.pages[idx].active = false;
			Popups.pages[idx].lnpage = "";
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

	debug("showPage("+name+")");
	pID = findPageNumber(name);

	if (pID > 0)
		pname = "Page_"+pID;
	else
		pname = name;

	try
	{
		var ID;
		ID = getActivePage();

		dropPage();
		drawPage(name);

		for (i in Popups.pages)
		{
			pname = "Page_"+Popups.pages[i].ID;

			if (Popups.pages[i].active && Popups.pages[i].lnpage != name)
			{
				freeZIndex();
			}
			else if (Popups.pages[i].active)
			{
				if (document.getElementById(pname) === null)
					drawPopup(name);

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

	debug("hidePage("+name+")");
	pID = findPageNumber(name);

	if (pID >= 0)
		pname = "Page_"+pID;
	else
		pname = name;

	try
	{
		dropPage();

		for (i in Popups.pages)
		{
			if (Popups.pages[i].active)
			{
				if (Popups.pages[i].lnpage != name)
					freeZIndex();
				else
					Popups.pages[i].active = false;

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
	debug("switchDisplay("+name1+","+name2+","+dStat+","+cport+","+cnum+")");

	try
	{
		if (dStat == 1)
		{
			document.getElementById(name1).style.display = "none";
			document.getElementById(name2).style.display = "inline";
			writeTextOut("PUSH:"+cport+":"+cnum+":1;");
		}
		else
		{
			document.getElementById(name1).style.display = "inline";
			document.getElementById(name2).style.display = "none";
			writeTextOut("PUSH:"+cport+":"+cnum+":0;");
		}
	}
	catch(e)
	{
		console.log('switchDisplay: Error: name1='+name1+', name2='+name2+', dStat='+dStat+', cport='+cport+', cnum='+cnum);
	}
}
function pushButton(cport, cnum, stat)
{
	debug("pushButton("+cport+","+cnum+","+stat+")");
	writeTextOut("PUSH:"+cport+":"+cnum+":"+stat+";");
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

	name = 'Page_'+bt[b].pnum+'_Button_'+bt.bi+'_1_font';

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
			name = 'Page_'+bt[b].pnum+'_Button_'+bt.bi+'_'+i+'_font';

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
		name = 'Page_'+bt[b].pnum+'_Button_'+bt.bi+'_'+i+'_font';

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
			name = 'Page_'+bt[b].pnum+'_Button_'+bt.bi+'_'+i+'_font';

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
function setON(msg)
{
	var b;

	var addr = getField(msg, 0, ',');
	var bt = findButton(addr);

	if (bt.length == 0)
	{
		console.log('setON: Error button '+addr+' not found!');
		return;
	}

	for (b = 0; b < bt.length; b++)
	{
		try
		{
			var name1 = 'Page_'+bt[b].pnum+'_Button_'+bt[b].bi+'_1';
			var name2 = 'Page_'+bt[b].pnum+'_Button_'+bt[b].bi+'_2';

			setButtonOnline(bt[b].pnum, bt[b].bi, 2);
			document.getElementById(name1).style.display = 'none';
			document.getElementById(name2).style.display = 'inline';
			writeTextOut("ON:"+bt[b].cp+":"+bt[b].ch+";");
		}
		catch(e)
		{
			console.log("setON: [Page_"+bt[b].pnum+"_Button_"+bt[b].bi+"_?] Error: "+e);
		}
	}
}
function setOFF(msg)
{
	var b;

	var addr = getField(msg, 0, ',');
	var bt = findButton(addr);

	if (bt.length == 0)
	{
		console.log('setOFF: Error button '+addr+' not found!');
		return;
	}

	for (b = 0; b < bt.length; b++)
	{
		try
		{
			var name1 = 'Page_'+bt[b].pnum+'_Button_'+bt[b].bi+'_1';
			var name2 = 'Page_'+bt[b].pnum+'_Button_'+bt[b].bi+'_2';

			setButtonOnline(bt[b].pnum, bt[b].bi, 1);
			document.getElementById(name1).style.display = 'inline';
			document.getElementById(name2).style.display = 'none';
			writeTextOut("OFF:"+bt[b].cp+":"+bt[b].ch+";");
		}
		catch(e)
		{
			console.log("setOFF: [Page_"+bt[b].pnum+"_Button_"+bt[b].bi+"_?] Error: "+e);
		}
	}

}
function setLEVEL(msg)
{
	var i;

	var addr = getField(msg, 0, ',');
	var level = getField(msg, 1, ',');
	var bgArray = findBargraphs(curPort, addr);

	for (i in bgArray)
	{
		var value = parseInt(100.0 / bgArray[i].rh * level);
		var name = "Page_" + bgArray[i].pnum + "_Button_" + bgArray[i].bi + "_1";
		var width, height, dir;

		if (bgArray[i].states[0].mi.length > 0)
		{
			width = bgArray[i].states[0].mi_width;
			height = bgArray[i].states[0].mi_height;
		}
		else if (bgArray[i].states[0].bm.length > 0)
		{
			width = bgArray[i].states[0].bm_width;
			height = bgArray[i].states[0].bm_height;
		}
		else
		{
			width = 0;
			height = 0;
		}

		if (bgArray[i].dr == "horizontal")
			dir = false;
		else
			dir = true;

		if (bgArray[i].states[0].mi.length > 0 && bgArray[i].states[1].bm.length > 0)
		{
//			console.log("setLEVEL: name=" + name + ", mi=" + bgArray[i].states[0].mi + ", bm=" + bgArray[i].states[1].bm+", width="+width+", height="+height);
			try
			{
				var cv = document.getElementById(name+'_canvas');

				if (cv !== null)
					document.removeChild(cv);

				drawBargraph(makeURL("images/"+bgArray[i].states[0].mi), makeURL("images/"+bgArray[i].states[1].bm), name, value, width, height, getAMXColor(bgArray[i].states[0].cf), getAMXColor(bgArray[i].states[0].cb), dir);
			}
			catch(e)
			{
				console.log("setLEVEL: Error: "+e);
			}
		}
	}
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
		hidePopup(Popups.pages[i].name);
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
					document.getElementById('Page_'+bt[b].pnum+'_Button_'+bt[b].bi+'_'+j).addEventListener("click", showPopup(name));
			}
			else if (cmd.search('Hide') >= 0)
			{
				num = bt[b].instances;

				for (j = 0; j < num; j++)
					document.getElementById('Page_'+bt[b].pnum+'_Button_'+bt[b].bi+'_'+j).addEventListener("click", hidePopup(name));
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
						name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi+"_"+z+"_font";

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
function doBCB(msg)
{
	var i;
	var j;
	var z;
	var b;

	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');
	var col = getField(msg, 2, ',');

	var addrRange = getRange(addr);
	var btRange = getRange(bts);

	for (i = 0; i < addrRange.length; i++)
	{
		var bt = findButtonPort(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doBCB: Error button '+addrRange[i]+' not found!');
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
						var name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi+"_"+z;

						try
						{
							var colArr;

							if ((colArr = getHexColor(col)) === -1)
							{
								console.log("doBCT: Error getting color for "+name);
								continue;
							}

							var color;

							if (colArr.length > 3)
								color = rgba(colArr[0],colArr[1],colArr[2],colArr[3]);
							else
								color = rgb(colArr[0],colArr[1],colArr[2]);

							document.getElementById(name).style.borderColor = color;
						}
						catch(e)
						{
							console.log("doBCB: No element of name "+name+" found!");
						}
					}
				}
			}
		}
	}
}
function doBCF(msg)
{
	var i;
	var j;
	var z;
	var b;

	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');
	var col = getField(msg, 2, ',');

	var addrRange = getRange(addr);
	var btRange = getRange(bts);

	for (i = 0; i < addrRange.length; i++)
	{
		var bt = findButtonPort(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doBCF: Error button '+addrRange[i]+' not found!');
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
						var name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi+"_"+z;

						try
						{
							var colArr;

							if ((colArr = getHexColor(col)) === -1)
							{
								console.log("doBCT: Error getting color for "+name);
								continue;
							}

							var color;

							if (colArr.length > 3)
								color = rgba(colArr[0],colArr[1],colArr[2],colArr[3]);
							else
								color = rgb(colArr[0],colArr[1],colArr[2]);

							document.getElementById(name).style.backgroundColor = color;
						}
						catch(e)
						{
							console.log("doBCF: No element of name "+name+" found!");
						}
					}
				}
			}
		}
	}
}
function doBCT(msg)
{
	var i;
	var j;
	var z;
	var b;

	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');
	var col = getField(msg, 2, ',');

	var addrRange = getRange(addr);
	var btRange = getRange(bts);

	for (i = 0; i < addrRange.length; i++)
	{
		var bt = findButtonPort(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doBCT: Error button '+addrRange[i]+' not found!');
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
						var name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi+"_"+z;

						try
						{
							var colArr;

							if ((colArr = getHexColor(col)) === -1)
							{
								console.log("doBCT: Error getting color for "+name);
								continue;
							}

							var color;

							if (colArr.length > 3)
								color = rgba(colArr[0],colArr[1],colArr[2],colArr[3]);
							else
								color = rgb(colArr[0],colArr[1],colArr[2]);

							document.getElementById(name).style.color = color;
						}
						catch(e)
						{
							console.log("doBCT: No element of name "+name+" found! Error: "+e);
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
		bt = findButtonPort(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doBMI: Error button '+addrRange[i]+' not found!');
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
						name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi+"_"+z;

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
		bt = findButtonPort(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doBSP: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (b = 0; b < bt.length; b++)
		{
			name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi+"_"+z;

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
		bt = findButtonPort(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doCPF: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (b = 0; b < bt.length; b++)
		{
			name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi;

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
			name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi;

			try
			{
				bt[b].visible = val;

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
async function doICO(msg)
{
	var bt;
	var addr;
	var bts;
	var addrRange;
	var btRange;
	var idx;
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
		bt = findButtonPort(addrRange[i]);

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
						name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi+"_"+z;

						try
						{
							document.getElementById(name+'_img').src = "images/"+getIconFile(idx);
						}
						catch(e)
						{
							// Create a new icon image
							var span;
							var hasSpan = false;

							try
							{
								span = document.getElementById(name+'_font');
								hasSpan = true;
							}
							catch(e)
							{
								hasSpan = false;
							}

							var parent;
							var hasParent = false;
							var cnt = 0;
							var err = "";

							while(!hasParent && cnt < 20)
							{
								try
								{
									parent = document.getElementById(name);

									if (parent !== null)
									{
										hasParent = true;
										break;
									}
								}
								catch(e)
								{
									err = e;
								}

								await new Promise(r => setTimeout(r, 200));
								cnt++;
							}

							if (!hasParent || parent === null)
							{
								console.log("doICO: No parent of name "+name+" found! ["+err+"]");
								return;
							}

							var ico = getIconFile(idx);
							var dim = getIconDim(idx);

							if (ico !== -1)
							{
								var img = document.createElement('img');
								img.src = makeURL('images/'+ico);
								img.id = name+'_img';
								img.width = dim[0];
								img.height = dim[1];
								img.onload = imgCenter(img, name);
								img.style.display = "flex";
								img.style.order = 2;
								parent.appendChild(img);

								if (hasSpan)
									parent.insertBefore(img, span);
							}
						}
					}
				}
			}
		}
	}
}
function doSHO(msg)
{
	var bt;
	var i;
	var j;
	var z;
	var b;
	var name;

	var addr = getField(msg, 0, ',');
	var stat = getField(msg, 1, ',');

	var addrRange = getRange(addr);

	for (i = 0; i < addrRange.length; i++)
	{
		bt = findButton(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doSHO: Error button '+addrRange[i]+' not found!');
			continue;
		}

		for (b = 0; b < bt.length; b++)
		{
			for (z = 1; z <= bt[b].instances; z++)
			{
				name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi+"_"+z;

				try
				{
					bt[b].visible = stat;
					document.getElementById(name).style.display = ((stat == 0) ? 'none' : 'inline');
				}
				catch(e)
				{
					console.log("doSHO: No element of name "+name+" found! ["+e+"]");
				}
			}
		}
	}
}
async function doTXT(msg)
{
	var bt;
	var i;
	var j;
	var z;
	var b;
	var name;

	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');
	var text = getField(msg, 2, ',');

	var addrRange = getRange(addr);
	var btRange = getRange(bts);

	for (i = 0; i < addrRange.length; i++)
	{
		bt = findButtonPort(addrRange[i]);

		if (bt.length == 0)
		{
			console.log('doTXT: Error button '+addrRange[i]+' not found!');
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
						name = 'Page_'+bt[b].pnum+"_Button_"+bt[b].bi+"_"+z;
						changePageTextInst(bt[b].pnum, curPort, addrRange[i], z-1, text);

						try
						{
							document.getElementById(name+'_font').innerHTML = text;
						}
						catch(e)
						{
							var parent;
							var hasParent = false;
							var cnt = 0;
							var err = "";

							while(!hasParent && cnt < 5)
							{
								try
								{
									parent = document.getElementById(name);

									if (parent !== null)
									{
										hasParent = true;
										break;
									}
								}
								catch(e)
								{
									err = e;
								}

								await new Promise(r => setTimeout(r, 200));
								cnt++;
							}

							if (!hasParent || parent === null)
							{
								console.log("doTXT: No parent of name "+name+" found! ["+err+"]");
								continue;
							}

							var span = document.createElement('span');
							span.id = name+'_font';
							span.style.display = "flex";
							span.style.order = 3;
							span.innerHTML = text;
							parent.appendChild(span);
						}
					}
				}
			}
		}
	}
}
function doABEEP(msg)
{
	beep();
}
function doADBEEP(msg)
{
	beep();
	sleep(250).then(() => {
		beep();
	});
}
function parseMessage(msg)
{
	var i;

	console.log("parseMessage: msg="+msg);
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

	img.style.width = iw;
	img.style.height = ih;
}
function posImage(img, name, code, width, height)
{
	var pw;
	var ph;
	var iw;
	var ih;
	var elem;

	elem = document.getElementById(name);

	if (elem === null)
	{
		console.log("posImage: Couldn't find an element with name "+name+"!");
		return;
	}

	pw = width;
	ph = height;
	iw = img.width;
	ih = img.height;

	if (pw == 0 || ph == 0)
	{
		console.log("posImage: WARNING: Parent width/height is 0!");
		pw = iw;
	}

	if (iw == 0 || ih == 0)
	{
		console.log("posImage: Image with no size at "+name+"!");
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
function beep()
{
	var snd = new Audio("data:audio/wav;base64,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qJrx4KOQjahZxWbcZgztj2c49nKmkId44S71j0c8eV9yDK6uPRzx5X18eDvjvQ6yKo9ZSS6l//8elePK/Lf//IInrOF/FvDoADYAGBMGb7FtErm5MXMlmPAJQVgWta7Zx2go+8xJ0UiCb8LHHdftWyLJE0QIAIsI+UbXu67dZMjmgDGCGl1H+vpF4NSDckSIkk7Vd+sxEhBQMRU8j/12UIRhzSaUdQ+rQU5kGeFxm+hb1oh6pWWmv3uvmReDl0UnvtapVaIzo1jZbf/pD6ElLqSX+rUmOQNpJFa/r+sa4e/pBlAABoAAAAA3CUgShLdGIxsY7AUABPRrgCABdDuQ5GC7DqPQCgbbJUAoRSUj+NIEig0YfyWUho1VBBBA//uQZB4ABZx5zfMakeAAAAmwAAAAF5F3P0w9GtAAACfAAAAAwLhMDmAYWMgVEG1U0FIGCBgXBXAtfMH10000EEEEEECUBYln03TTTdNBDZopopYvrTTdNa325mImNg3TTPV9q3pmY0xoO6bv3r00y+IDGid/9aaaZTGMuj9mpu9Mpio1dXrr5HERTZSmqU36A3CumzN/9Robv/Xx4v9ijkSRSNLQhAWumap82WRSBUqXStV/YcS+XVLnSS+WLDroqArFkMEsAS+eWmrUzrO0oEmE40RlMZ5+ODIkAyKAGUwZ3mVKmcamcJnMW26MRPgUw6j+LkhyHGVGYjSUUKNpuJUQoOIAyDvEyG8S5yfK6dhZc0Tx1KI/gviKL6qvvFs1+bWtaz58uUNnryq6kt5RzOCkPWlVqVX2a/EEBUdU1KrXLf40GoiiFXK///qpoiDXrOgqDR38JB0bw7SoL+ZB9o1RCkQjQ2CBYZKd/+VJxZRRZlqSkKiws0WFxUyCwsKiMy7hUVFhIaCrNQsKkTIsLivwKKigsj8XYlwt/WKi2N4d//uQRCSAAjURNIHpMZBGYiaQPSYyAAABLAAAAAAAACWAAAAApUF/Mg+0aohSIRobBAsMlO//Kk4soosy1JSFRYWaLC4qZBYWFRGZdwqKiwkNBVmoWFSJkWFxX4FFRQWR+LsS4W/rFRb/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////VEFHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAU291bmRib3kuZGUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjAwNGh0dHA6Ly93d3cuc291bmRib3kuZGUAAAAAAAAAACU=");
	snd.play();
}
function writeTextOut(msg)
{
	console.log("--> "+msg);

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
			debug("Band width value: "+connection.bandwidth);
			debug("Metered value:"+(connection.metered ? '' : 'not ') + 'metered');
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
			debug("Connection type: "+connection.type);
		});

		connection.dispatchEvent(new Event('typechange'));
	}
}
function debug(text)
{
	if (!__debug)
		return;

	console.log(text);
}