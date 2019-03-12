/*
 * Copyright (C) 2019 by Andreas Theofilu <andreas@theosys.at>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Andreas Theofilu and his suppliers, if any.
 * The intellectual and technical concepts contained
 * herein are proprietary to Andreas Theofilu and its suppliers and
 * may be covered by European and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Andreas Theofilu.
 */

const SCR_TYPE = Object.freeze({
	SCR_NONE:			0,
	SCR_TIME_STANDARD:	1,
	SCR_TIME_AMPM:		2,
	SCR_TIME_24:		3,
	SCR_DATE_WEEKDAY:	4,
	SCR_DATE_D_MONTH_Y:	5,
	SCR_DATE_D_M:		6,
	SCR_DATE_D_M_Y:		7,
	SCR_DATE_M_D:		8,
	SCR_DATE_M_D_Y:		9,
	SCR_DATE_MONTH_D_Y:	10,
	SCR_DATE_Y_M_D:		11
});

const TEXT_ORIENTATION = Object.freeze({
	ORI_ABSOLUT:		0,
	ORI_TOP_LEFT:		1,
	ORI_TOP_MIDDLE:		2,
	ORI_TOP_RIGHT:		3,
	ORI_CENTER_LEFT:	4,
	ORI_CENTER_MIDDLE:	5,	// default
	ORI_CENTER_RIGHT:	6,
	ORI_BOTTOM_LEFT:	7,
	ORI_BOTTOM_MIDDLE:	8,
	ORI_BOTTOM_RIGHT:	9
});

const BUTTONTYPE = Object.freeze({
	NONE:					0,
	GENERAL:				1,
	MULTISTATE_GENERAL:		2,
	BARGRAPH:				3,
	MULTISTATE_BARGRAPH:	4,
	JOISTICK:				5,
	TEXT_INPUT:				6,
	COMPUTER_CONTROL:		7,
	TAKE_NOTE:				8,
	SUBPAGE_VIEW:			9
});

const FEEDBACK = Object.freeze({
	FB_NONE:		0,
	FB_CHANNEL:		1,
	FB_INV_CHANNEL:	2,	// inverted channel
	FB_ALWAYS_ON:	3,
	FB_MOMENTARY:	4,
	FB_BLINK:		5
});

const PAGETYPE = Object.freeze({
	PNONE:		0,
	PAGE:		1,
	SUBPAGE:	2
});

const SHOWEFFECT = Object.freeze({
	SE_NONE:				0,
	SE_FADE:				1,
	SE_SLIDE_LEFT:			2,
	SE_SLIDE_RIGHT:			3,
	SE_SLIDE_TOP:			4,
	SE_SLIDE_BOTTOM:		5,
	SE_SLIDE_LEFT_FADE:		6,
	SE_SLIDE_RIGHT_FADE:	7,
	SE_SLIDE_TOP_FADE:		8,
	SE_SLIDE_BOTTOM_FADE:	9
});

var sysBorders = { "borders":[
	{ "name": "Single Line", "border_style": "solid", "border_width": "1px", "border_radius": "" },
	{ "name": "Double Line", "border_style": "solid", "border_width": "2px", "border_radius": "" },
	{ "name": "Quad Line", "border_style": "solid", "border_width": "4px", "border_radius": "" },
	{ "name": "Picture Frame", "border_style": "double", "border_width": "", "border_radius": "" },
	{ "name": "Circle 15", "border_style": "solid", "border_width": "2px", "border_radius": "15px" },
	{ "name": "Circle 25", "border_style": "solid", "border_width": "2px", "border_radius": "25px" },
	{ "name": "Circle 35", "border_style": "solid", "border_width": "2px", "border_radius": "35px" },
	{ "name": "Circle 45", "border_style": "solid", "border_width": "2px", "border_radius": "45px" },
	{ "name": "Circle 55", "border_style": "solid", "border_width": "2px", "border_radius": "55px" },
	{ "name": "Circle 65", "border_style": "solid", "border_width": "2px", "border_radius": "65px" },
	{ "name": "Circle 75", "border_style": "solid", "border_width": "2px", "border_radius": "75px" },
	{ "name": "Circle 85", "border_style": "solid", "border_width": "2px", "border_radius": "85px" },
	{ "name": "Circle 95", "border_style": "solid", "border_width": "2px", "border_radius": "95px" },
	{ "name": "Circle 105", "border_style": "solid", "border_width": "2px", "border_radius": "105px" },
	{ "name": "Circle 115", "border_style": "solid", "border_width": "2px", "border_radius": "115px" },
	{ "name": "Circle 125", "border_style": "solid", "border_width": "2px", "border_radius": "125px" },
	{ "name": "Circle 135", "border_style": "solid", "border_width": "2px", "border_radius": "135px" },
	{ "name": "Circle 145", "border_style": "solid", "border_width": "2px", "border_radius": "145px" },
	{ "name": "Circle 155", "border_style": "solid", "border_width": "2px", "border_radius": "155px" },
	{ "name": "Circle 165", "border_style": "solid", "border_width": "2px", "border_radius": "165px" },
	{ "name": "Circle 175", "border_style": "solid", "border_width": "2px", "border_radius": "175px" },
	{ "name": "AMX Elite Inset _L", "border_style": "groove", "border_width": "10px", "border_radius": "" },
	{ "name": "AMX Elite Raised _L", "border_style": "ridge", "border_width": "10px", "border_radius": "" },
	{ "name": "AMX Elite Inset _M", "border_style": "groove", "border_width": "5px", "border_radius": "" },
	{ "name": "AMX Elite Raised _M", "border_style": "ridge", "border_width": "5px", "border_radius": "" },
	{ "name": "AMX Elite Inset _S", "border_style": "groove", "border_width": "2px", "border_radius": "" },
	{ "name": "AMX Elite Raised _S", "border_style": "ridge", "border_width": "2px", "border_radius": "" },
	{ "name": "Bevel Inset _L", "border_style": "inset", "border_width": "10px", "border_radius": "" },
	{ "name": "Bevel Raised _L", "border_style": "outset", "border_width": "10px", "border_radius": "" },
	{ "name": "Bevel Inset _M", "border_style": "inset", "border_width": "5px", "border_radius": "" },
	{ "name": "Bevel Raised _M", "border_style": "outset", "border_width": "5px", "border_radius": "" },
	{ "name": "Bevel Inset _S", "border_style": "inset", "border_width": "2px", "border_radius": "" },
	{ "name": "Bevel Raised _S", "border_style": "outset", "border_width": "2px", "border_radius": "" }
]};

var sysReserved = { "dttm":[
	{ "channel":141, "name":"btTimeStandard" },	// Standard time
	{ "channel":142, "name":"btTimeAM/PM" },	// Time AM/PM
	{ "channel":143, "name":"btTime24" },		// 24 hour time
	{ "channel":151, "name":"btDate151" },		// Date: weekday
	{ "channel":152, "name":"btDate152" },		// Date: mm/dd
	{ "channel":153, "name":"btDate153" },		// Date: dd/mm
	{ "channel":154, "name":"btDate154" },		// Date: mm/dd/yyyy
	{ "channel":155, "name":"btDate155" },		// Date: dd/mm/yyyy
	{ "channel":156, "name":"btDate156" },		// Date: month dd, yyyy
	{ "channel":157, "name":"btDate157" },		// Date: dd month, yyyy
	{ "channel":158, "name":"btDate158" },		// Date: yyyy-mm-dd
	{ "channel":242, "name":"batLevel" },		// Battery level
	{ "channel":234, "name":"batCharge" }		// Battery charging/not charging
]};

function isSystemReserved(channel)
{
	var i;

	try
	{
		for (i in sysReserved.dttm)
		{
			if (sysReserved.dttm[i].channel == channel)
				return true;
		}
	}
	catch(e)
	{
		console.log("isSystemReserved: Error: "+e);
	}

	return false;
}
function getSystemReservedFunc(channel)
{
	var i;

	try
	{
		for (i in sysReserved.dttm)
		{
			if (sysReserved.dttm[i].channel == channel)
				return sysReserved.dttm[i].func;
		}
	}
	catch(e)
	{
		console.log("getSystemReservedFunc: Error: "+e);
	}

	return -1;
}
function getSystemReservedName(channel)
{
	var i;

	try
	{
		for (i in sysReserved.dttm)
		{
			if (sysReserved.dttm[i].channel == channel)
				return sysReserved.dttm[i].name;
		}
	}
	catch(e)
	{
		console.log("getSystemReservedName: Error: "+e);
	}

	return -1;
}
function getBorderStyle(name)
{
	var i;

	for (i in sysBorders.borders)
	{
		var brd = sysBorders.borders[i];

		if (brd.name == name)
		{
			var ret = [];
			ret.push(brd.border_style);

			if (brd.border_width.length > 0)
				ret.push(brd.border_width);

			if (brd.border_radius.length > 0)
				ret.push(brd.border_radius);

			return ret;
		}
	}

	return -1;
}
function setCSSclass(name, content)
{
	var element = document.querySelector('.'+name);

	if (element !== null)
		return;

	var style = document.createElement('style');;
	style.type = 'text/css';
	style.innerHTML = "."+name+" {"+content+"}";
	document.getElementsByTagName('head')[0].appendChild(style);
}
function changePageText(num, port, channel, text)
{
	try
	{
		var pgKey = eval("structPage"+num);
		var i;

		for (i in pgKey.buttons)
		{
			button = pgKey.buttons[i];

			if (button.ap == port && button.ad == channel)
			{
				var j;

				for (j in button.sr)
				{
					var sr = button.sr[i];
					sr.te = text;
				}
			}
		}
	}
	catch(e)
	{
		console.log("changePageText: Error: "+e);
	}
}
function changePageTextInst(num, port, channel, inst, text)
{
	try
	{
		var pgKey = eval("structPage"+num);
		var i;

		for (i in pgKey.buttons)
		{
			button = pgKey.buttons[i];

			if (button.ap == port && button.ad == channel)
			{
				if (inst < button.sr.length)
					button.sr[inst].te = text;
			}
		}
	}
	catch(e)
	{
		console.log("changePageText: Error: "+e);
	}
}
function drawPage(name)
{
	var i;
	var pageID = 0;

	for (i in Pages.pages)
	{
		if (Pages.pages[i].name == name)
		{
			pageID = Pages.pages[i].ID;
			break;
		}
	}

	if (pageID == 0)
	{
		console.log("drawPage: Couldn't find a page with the name "+name+"!");
		return false;
	}

	var pgKey = eval("structPage"+pageID);

	if (!dropPage())
		return false;

	return doDraw(pgKey, pageID, PAGETYPE.PAGE);
}
function drawPopup(name)
{
	var i;
	var pageID = 0;

	for (i in Popups.pages)
	{
		if (Popups.pages[i].name == name)
		{
			pageID = Popups.pages[i].ID;
			break;
		}
	}

	if (pageID == 0)
	{
		console.log("drawPopup: Couldn't find a popup with the name "+name+"!");
		return false;
	}

	var pgKey = eval("structPage"+pageID);
	return doDraw(pgKey, pageID, PAGETYPE.SUBPAGE);
}
function doDraw(pgKey, pageID, what)
{
	var page;
	var i, j;
	var btArray;

	try
	{
		if (what == PAGETYPE.SUBPAGE)
		{
			var div = document.getElementById('main');
			page = document.createElement('div');
			page.style.display = "inline-block"
			div.appendChild(page);
		}
		else
		{
			page = document.getElementById('main');
			page.style.display = "inline";
			page.style.overflow = "hidden";
		}
	}
	catch(e)
	{
		console.log("doDraw: Error: "+e);
		return false;
	}

	// The base popup
	if (what == PAGETYPE.SUBPAGE)
		page.id = "Page_"+pageID;

	page.style.position = "absolute";
	page.style.left = pgKey.left+"px";
	page.style.top = pgKey.top+"px";
	page.style.width = pgKey.width+"px";
	page.style.height = pgKey.height+"px";

	for (i in pgKey.sr)        // Page background and color
	{
		var sr = pgKey.sr[i];

		if (sr.cf.length > 0 && sr.mi.length == 0)
			page.style.backgroundColor = getWebColor(sr.cf);

		if (sr.ct.length > 0)
			page.style.color = getWebColor(sr.ct);

		if (sr.mi.length > 0 && sr.bm.length > 0)	// chameleon image?
		{
			try
			{
				var width = sr.mi_width;
				var height = sr.mi_height;

				if (sr.bm.length > 0)
					drawButton(makeURL("images/"+sr.mi),makeURL("images/"+sr.bm),"Page_"+pageID,width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
				else
					drawArea(makeURL("images/"+sr.mi),"Page_"+pageID, width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
			}
			catch(e)
			{
				console.log("doDraw: Error: "+e);
				return false;
			}
		}
		else if (sr.bm.length)
		{
			page.style.backgroundImage = "url('images/"+sr.bm+"')";
			page.style.backgroundRepeat = "no-repeat";
		}
	}

	// The elements of the page
	for (i in pgKey.buttons)
	{
		var button = pgKey.buttons[i];
		btArray = null;

		for (i in buttonArray.buttons)
		{
			var but = buttonArray.buttons[i];

			if (but.pnum == pageID && but.bi == button.bID)
			{
				btArray = but;
				break;
			}
		}

		try
		{
			var bt = document.createElement('div');
			bt.id = "Page_"+pageID+"_Button_"+button.bID;
			page.appendChild(bt);
			bt.style.position = "absolute";
			bt.style.left = button.lt+"px";
			bt.style.top = button.tp+"px";
			bt.style.width = button.wt+"px";
			bt.style.height = button.ht+"px";

			if (button.hs == "bounding")
				bt.style.overflow = "hidden";

			if (button.cp > 0 && button.ch > 0 && button.sr.length == 2)		// clickable?
			{
				if (button.fb == FEEDBACK.FB_MOMENTARY)
				{
					var name1 = "Page_"+pageID+"_Button_"+button.bID+"_"+button.sr[0].number;
					var name2 = "Page_"+pageID+"_Button_"+button.bID+"_"+button.sr[1].number;
					bt.addEventListener('mousedown', switchDisplay.bind(null, name1,name2,1,button.cp,button.ch));
					bt.addEventListener('mouseup', switchDisplay.bind(null, name1,name2,0,button.cp,button.ch));
				}
				else if (button.fb == FEEDBACK.FB_CHANNEL || button.fb == 0)
				{
					bt.addEventListener('mousedown', pushButton.bind(null, button.cp,button.ch,1));
					bt.addEventListener('mouseup', pushButton.bind(null, button.cp,button.ch,0));
				}
				else if (button.fb == FEEDBACK.FB_INV_CHANNEL)
				{
					bt.addEventListener('mousedown', pushButton.bind(null, button.cp,button.ch,0));
					bt.addEventListener('mouseup', pushButton.bind(null, button.cp,button.ch,1));
				}
				else if (button.fb == FEEDBACK.FB_ALWAYS_ON)
					bt.addEventListener('click', pushButton.bind(null, button.cp,button.ch,1));
			}

			if (button.pfType == "sShow")			// show popup
				bt.addEventListener('click', showPopup.bind(null, button.pfName));
			else if (button.pfType == "sHide")		// hide popup
				bt.addEventListener('click', hidePopup.bind(null, button.pfName));
			else if (button.pfType == "scGroup")	// hide group
				bt.addEventListener('click', hideGroup.bind(null, button.pfName));

			for (j in button.sr)
			{
				var sr = button.sr[j];
				var bsr = document.createElement('div');
				bsr.style.position = "absolute";
				bsr.style.left = "0px";
				bsr.style.top = "0px";
				bsr.style.width = bt.style.width;
				bsr.style.height = bt.style.height;
				bt.appendChild(bsr);
				var nm;

				if (button.ap == 0 && isSystemReserved(button.ad))
					nm = getSystemReservedName(button.ad);
				else
					nm = "Page_"+pageID+"_Button_"+button.bID+"_"

				bsr.id = nm+sr.number;
				bsr.style.color = getWebColor(sr.ct);

				if (sr.mi.length == 0 || sr.bs.length > 0)
					bsr.style.backgroundColor = getWebColor(sr.cf);

				if (sr.bs.length > 0)		// Border
				{
					var brd = getBorderStyle(sr.bs);

					if (brd !== -1)
					{
						var x;

						for (x = 0; x < brd.length; x++)
						{
							switch(x)
							{
								case 0: bsr.style.borderStyle = brd[x]; break;
								case 1: bsr.style.borderWidth = brd[x]; break;
								case 2: bsr.style.borderRadius = brd[x]; break;
							}
						}

						bsr.style.borderColor = getWebColor(sr.cb);
					}
					else
						bsr.style.border = "none";
				}
				else
					bsr.style.border = "none";

				if (!(button.btype == BUTTONTYPE.BARGRAPH && button.sr.length == 2) && sr.mi.length > 0)	// chameleon image?
				{
					var width = sr.mi_width;
					var height = sr.mi_height;
					var can;
					setCSSclass(nm+sr.number+"_canvas", "position: absolute; left: 0px; top: 0px; width: "+width+"px; height: "+height+"px; display: flex; order: 1;");

					if (sr.bm.length > 0)
						drawButton(makeURL("images/"+sr.mi),makeURL("images/"+sr.bm),nm+sr.number,width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
					else
						drawArea(makeURL("images/"+sr.mi),nm+sr.number, width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
				}
				else if (button.btype == BUTTONTYPE.BARGRAPH && button.sr.length == 2 && sr.mi.length > 0)
				{
					var width = sr.mi_width;
					var height = sr.mi_height;
					var can;
					setCSSclass(nm + sr.number + "_canvas", "position: absolute; left: 0px; top: 0px; width: " + width + "px; height: " + height + "px; display: flex; order: 1;");

					if (sr.bm.length > 0)
					{
						var level = parseInt(100.0 / button.rh * button.lv);
						drawBargraph(makeURL("images/" + sr.mi), makeURL("images/" + sr.bm), nm + sr.number, level, width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
					}
					else
						drawArea(makeURL("images/" + sr.mi), nm + sr.number, width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
				}
				else if (sr.bm.length > 0)
				{
					bsr.style.backgroundImage = "url('images/"+sr.bm+"')";
					bsr.style.backgroundRepeat = "no-repeat";
				}

				if (sr.ii > 0)		// Icon?
				{
					var ico = getIconFile(sr.ii);
					var dim = getIconDim(sr.ii);

					if (ico !== -1)
					{
						var img = document.createElement('img');
						img.src = makeURL('images/'+ico);
						img.id = nm+sr.number+'_img';
						img.width = dim[0];
						img.height = dim[1];
						img.style.position = "absolute";

						if (sr.ji == 0)
						{
							img.style.left = sr.ix+'px';
							img.style.top = sr.iy+'px';
						}
						else
							posImage(img, nm+sr.number, sr.ji, dim[0], dim[1]);

						img.style.display = "flex";
						img.style.order = 2;
						bsr.appendChild(img);
					}
				}

				var font = findFont(sr.fi);

				if (font !== -1)
				{
					var fnt = document.createElement('span');
					fnt.id = nm+sr.number+'_font';
					fnt.style.position = "absolute";

					if (sr.jt != TEXT_ORIENTATION.ORI_ABSOLUT)
					{
						fnt.style.left = "0px";
						fnt.style.top = "0px";
						fnt.style.width = bsr.style.width;
						fnt.style.height = bsr.style.height;
					}

					fnt.style.fontFamily = font.name;
					fnt.style.fontSize = font.size+"pt";
					fnt.style.fontStyle = getFontStyle(font.subfamilyName);
					fnt.style.fontWeight = getFontWeight(font.subfamilyName);
//					fnt.style.display = "flex";
//					fnt.style.order = 3;
					bsr.appendChild(fnt);

					if (sr.ww == 0 && (sr.jt == TEXT_ORIENTATION.ORI_CENTER_LEFT || sr.jt == TEXT_ORIENTATION.ORI_CENTER_MIDDLE || sr.jt == TEXT_ORIENTATION.ORI_CENTER_RIGHT))
						fnt.style.lineHeight = button.ht+"px";

					switch(sr.jt)
					{
						case TEXT_ORIENTATION.ORI_ABSOLUT:
							fnt.style.left = sr.tx+"px";
							fnt.style.top = sr.ty+"px";
							fnt.style.width = (bsr.style.width - sr.tx)+'px';
							fnt.style.height = (bsr.style.height - sr.ty)+'px';
						break;
						case TEXT_ORIENTATION.ORI_TOP_LEFT:
							fnt.style.textAlign = "left";
							fnt.style.verticalAlign = "top";
						break;
						case TEXT_ORIENTATION.ORI_TOP_MIDDLE:
							fnt.style.textAlign = "center";
							fnt.style.verticalAlign = "top";
						break;
						case TEXT_ORIENTATION.ORI_TOP_RIGHT:
							fnt.style.textAlign = "right";
							fnt.style.verticalAlign = "top";
						break;
						case TEXT_ORIENTATION.ORI_CENTER_LEFT:
							fnt.style.textAlign = "left";
							fnt.style.verticalAlign = "middle";
						break;
						case TEXT_ORIENTATION.ORI_CENTER_MIDDLE:
							fnt.style.textAlign = "center";
							fnt.style.verticalAlign = "middle";
						break;
						case TEXT_ORIENTATION.ORI_CENTER_RIGHT:
							fnt.style.textAlign = "right";
							fnt.style.verticalAlign = "middle";
						break;
						case TEXT_ORIENTATION.ORI_BOTTOM_LEFT:
							fnt.style.textAlign = "left";
							fnt.style.verticalAlign = "bottom";
						break;
						case TEXT_ORIENTATION.ORI_BOTTOM_MIDDLE:
							fnt.style.textAlign = "center";
							fnt.style.verticalAlign = "bottom";
						break;
						case TEXT_ORIENTATION.ORI_BOTTOM_RIGHT:
							fnt.style.textAlign = "right";
							fnt.style.verticalAlign = "bottom";
						break;
					}

					if (sr.te.length > 0)
						fnt.innerHTML = sr.te;
				}

				if (btArray !== null)
				{
					var comp = parseInt(j) + 1;
					console.log("doDraw: pnum="+btArray.pnum+", bi="+btArray.bi+", ion="+btArray.ion+" ["+comp+"], visible="+btArray.visible);
					if (comp == btArray.ion && btArray.visible == 1)
						bsr.style.display = "inline";
					else
						bsr.style.display = "none";
				}
				else
				{
					if (j == 0 && button.fb != FEEDBACK.FB_INV_CHANNEL && button.fb != FEEDBACK.FB_ALWAYS_ON)
						bsr.style.display = "inline";
					else if (j == 1 && (button.fb == FEEDBACK.FB_INV_CHANNEL || button.fb == FEEDBACK.FB_ALWAYS_ON))
						bsr.style.display = "inline";
					else
						bsr.style.display = "none";
				}
			}
		}
		catch(e)
		{
			console.log("doDraw: Error: "+e);
			return false;
		}
	}

	return true;
}
function dropPage()
{
	try
	{
		var div = document.getElementById('main');
		// Delete all elements
		while (div.hasChildNodes())
			div.removeChild(div.firstChild);
	}
	catch(e)
	{
		console.log("dropPage: Error: "+e);
		return false;
	}

	return true;
}
function dropPopup(name)
{
	var i;
	var pageID = 0;

	for (i in Popups.pages)
	{
		if (Popups.pages[i].name == name)
		{
			pageID = Popups.pages[i].ID;
			break;
		}
	}

	if (pageID == 0)
	{
		console.log("dropPopup: Couldn't find a popup with the name "+name+"!");
		return false;
	}

	try
	{
		var div = document.getElementById('Page_'+pageID);

		while (div.hasChildNodes())
			div.removeChild(div.firstChild);

		div.parentNode.removeChild(div);
	}
	catch(e)
	{
		console.log("dropPopup: Error: "+e);
		return false;
	}

	return true;
}
