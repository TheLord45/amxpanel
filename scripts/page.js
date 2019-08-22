/*
 * Copyright (C) 2019 by Andreas Theofilu <andreas@theosys.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */
'use strict';

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

const SYSTEMS = Object.freeze({
	OBJ_TIME_STANDARD:			141,
	OBJ_TIME_AMPM:				142,
	OBJ_TIME_24:				143,
	OBJ_DATE_151:				151,
	OBJ_DATE_152:				152,
	OBJ_DATE_153:				153,
	OBJ_DATE_154:				154,
	OBJ_DATE_155:				155,
	OBJ_DATE_156:				156,
	OBJ_DATE_157:				157,
	OBJ_DATE_158:				158,
	OBJ_BAT_LEVEL:				242,
	OBJ_BAT_CHARGE:				234
});

var sysBorders = { "borders":[
	{ "name": "Single Line", "number": 0, "border_style": "solid", "border_width": "1px", "border_radius": "" },
	{ "name": "Double Line", "number": 0, "border_style": "solid", "border_width": "2px", "border_radius": "" },
	{ "name": "Quad Line", "number": 0, "border_style": "solid", "border_width": "4px", "border_radius": "" },
	{ "name": "Picture Frame", "number": 0, "border_style": "double", "border_width": "", "border_radius": "" },
	{ "name": "Circle 15", "number": 8, "border_style": "solid", "border_width": "2px", "border_radius": "15px" },
	{ "name": "Circle 25", "number": 9, "border_style": "solid", "border_width": "2px", "border_radius": "25px" },
	{ "name": "Circle 35", "number": 10, "border_style": "solid", "border_width": "2px", "border_radius": "35px" },
	{ "name": "Circle 45", "number": 11, "border_style": "solid", "border_width": "2px", "border_radius": "45px" },
	{ "name": "Circle 55", "number": 12, "border_style": "solid", "border_width": "2px", "border_radius": "55px" },
	{ "name": "Circle 65", "number": 13, "border_style": "solid", "border_width": "2px", "border_radius": "65px" },
	{ "name": "Circle 75", "number": 14, "border_style": "solid", "border_width": "2px", "border_radius": "75px" },
	{ "name": "Circle 85", "number": 15, "border_style": "solid", "border_width": "2px", "border_radius": "85px" },
	{ "name": "Circle 95", "number": 16, "border_style": "solid", "border_width": "2px", "border_radius": "95px" },
	{ "name": "Circle 105", "number": 17, "border_style": "solid", "border_width": "2px", "border_radius": "105px" },
	{ "name": "Circle 115", "number": 18, "border_style": "solid", "border_width": "2px", "border_radius": "115px" },
	{ "name": "Circle 125", "number": 19, "border_style": "solid", "border_width": "2px", "border_radius": "125px" },
	{ "name": "Circle 135", "number": 20, "border_style": "solid", "border_width": "2px", "border_radius": "135px" },
	{ "name": "Circle 145", "number": 21, "border_style": "solid", "border_width": "2px", "border_radius": "145px" },
	{ "name": "Circle 155", "number": 22, "border_style": "solid", "border_width": "2px", "border_radius": "155px" },
	{ "name": "Circle 165", "number": 23, "border_style": "solid", "border_width": "2px", "border_radius": "165px" },
	{ "name": "Circle 175", "number": 24, "border_style": "solid", "border_width": "2px", "border_radius": "175px" },
	{ "name": "Circle 185", "number": 25, "border_style": "solid", "border_width": "2px", "border_radius": "185px" },
	{ "name": "Circle 195", "number": 26, "border_style": "solid", "border_width": "2px", "border_radius": "195px" },
	{ "name": "AMX Elite Inset _L", "number": 0, "border_style": "groove", "border_width": "10px", "border_radius": "" },
	{ "name": "AMX Elite Raised _L", "number": 0, "border_style": "ridge", "border_width": "10px", "border_radius": "" },
	{ "name": "AMX Elite Inset _M", "number": 0, "border_style": "groove", "border_width": "5px", "border_radius": "" },
	{ "name": "AMX Elite Raised _M", "number": 0, "border_style": "ridge", "border_width": "5px", "border_radius": "" },
	{ "name": "AMX Elite Inset _S", "number": 0, "border_style": "groove", "border_width": "2px", "border_radius": "" },
	{ "name": "AMX Elite Raised _S", "number": 0, "border_style": "ridge", "border_width": "2px", "border_radius": "" },
	{ "name": "Bevel Inset _L", "number": 0, "border_style": "inset", "border_width": "10px", "border_radius": "" },
	{ "name": "Bevel Raised _L", "number": 0, "border_style": "outset", "border_width": "10px", "border_radius": "" },
	{ "name": "Bevel Inset _M", "number": 0, "border_style": "inset", "border_width": "5px", "border_radius": "" },
	{ "name": "Bevel Raised _M", "number": 0, "border_style": "outset", "border_width": "5px", "border_radius": "" },
	{ "name": "Bevel Inset _S", "number": 0, "border_style": "inset", "border_width": "2px", "border_radius": "" },
	{ "name": "Bevel Raised _S", "number": 0, "border_style": "outset", "border_width": "2px", "border_radius": "" }
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

var EVENT_DOWN = "pointerdown";
var EVENT_UP = "pointerup";
var EVENT_MOVE = "pointermove";

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
		errlog("isSystemReserved: Error: "+e);
	}

	return false;
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
		errlog("getSystemReservedName: Error: "+e);
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
function getBorderStyleNum(num)
{
	var i;

	for (i in sysBorders.borders)
	{
		var brd = sysBorders.borders[i];

		if (brd.number == num)
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
function getBorderSize(name)
{
	for (var i in sysBorders.borders)
	{
		var brd = sysBorders.borders[i];

		if (brd.name == name)
			return parseInt(brd.border_width);
	}

	return 0;
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
function setCSSanim(name, content)
{
	try
	{
		var element = document.querySelector('ani_'+name);

		if (element !== null)
			return;

		var style = document.createElement('style');;
		style.type = 'text/css';
		style.innerHTML = "@keyframes ani_"+name+" {"+content+"}";
		document.getElementsByTagName('head')[0].appendChild(style);
	}
	catch (e)
	{
		errlog("setCSSanim: Error: "+e);
	}
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
		errlog("changePageText: Error: "+e);
	}
}
function getPage(pnum)
{
    return eval("structPage" + pnum);
}
function allElementsFromPoint(x, y)
{
    var element, elements = [];
	var old_visibility = [];

	if (typeof x != "number" || typeof y != "number")
	{
		errlog("allElementsFromPoint: Parameters x and y are not numeric!");
		return null;
	}

	if (x <= 0 || y <= 0)
	{
		errlog("allElementsFromPoint: Invalid parameters x and/or y: x="+x+", y="+y);
		return null;
	}

	while (true)
	{
        element = document.elementFromPoint(x, y);

		if (!element || element === document.documentElement)
            break;

        elements.push(element);
        old_visibility.push(element.style.visibility);
        element.style.visibility = 'hidden'; // Temporarily hide the element (without changing the layout)
    }

	for (var k = 0; k < elements.length; k++)
		elements[k].style.visibility = old_visibility[k];

    elements.reverse();
    return elements;
}
function getButtonKennung(name)
{
	if (typeof name != "string" || name.length < 17)
		return null;

	var pos1 = name.indexOf('_');
	var pos2 = name.indexOf('_', pos1 + 1);

	if (pos1 < 0 || pos2 < 0)
		return null;

	var pnum = parseInt(name.substr(pos1 + 1, pos2 - pos1));
	pos1 = name.indexOf('_', pos2 + 1);

	if (pos1 < 0)
		return null;

	var bi = parseInt(name.substr(pos1 + 1));

	if (isNaN(pnum) || isNaN(bi))
		return null;

	return [pnum, bi];
}
function hasGraphic(button, inst=0)
{
	if (button === null)
		return false;

	for (var i in button.sr)
	{
		var sr = button.sr[i];

		if ((inst == 0 || inst == parseInt(i)) && (sr.mi.length > 0 || sr.bm.length > 0))
			return true;
	}

	return false;
}
/*
 * Set the connection status bargraph. This is a system bargraph
 * and consists of several states. Here only 3 of them are used.
 */
function onlineStatus()
{
	if (navigator.online)
	{
		if (wsocket !== null)
		{
			if (wsocket.readyState == WebSocket.OPEN)	// online?
				setOnlineStatus(1);
			else
				setOnlineStatus(0);
		}
		else
			setOnlineStatus(0);
	}
	else
	{
		if (wsocket !== null)
		{
			if (wsocket.readyState == WebSocket.OPEN)	// online?
				setOnlineStatus(9);
			else
				setOnlineStatus(0);
		}
		else
			setOnlineStatus(0);
	}

	setSystemBattery(true);
}
function onOnline()
{
	debug("onOnline: We are online!");

	if (wsocket !== null)
	{
		if (wsocket.readyState == 1)	// online?
			setOnlineStatus(1);
		else
			setOnlineStatus(0);
	}
	else
		setOnlineStatus(0);
}
function onOffline()
{
	debug("onOffline: We are offline!");

	if (wsocket !== null)
	{
		if (wsocket.readyState == WebSocket.CLOSED)	// offline?
			setOnlineStatus(0);
		else if (wsocket.readyState == WebSocket.CLOSING)	// on the way to became offline
			setOnlineStatus(9);
		else
			setOnlineStatus(1);
	}
	else
		setOnlineStatus(0);
}
/*
 * Gets the x/y coordinates of the mouse click and finds the pixel
 * of the image. If the pixel is transparent, the image underneath
 * is tested and so on. When there is no more image, or a non
 * transparent pixel was found, the function stops.
 * When a non transparent pixel was found, the mouse event is
 * passed to the parent of that image (button).
 */
function activeTouch(event, name, object)
{
	var i;

	if (event === null)
	{
		errlog("activeTouch: Got no valid event on "+name);
		return;
	}

	if (typeof name != "string")
	{
		errlog("activeTouch: No object name!");
		return;
	}

	var rect = object.getBoundingClientRect();
	var x = event.clientX - rect.left;
	var y = event.clientY - rect.top;
	var objs = allElementsFromPoint(event.clientX, event.clientY);

	if (objs === null)
		return;

	for (i in objs)
	{
		if (objs[i].id.indexOf(name) == 0 && objs[i].id != name &&
			(objs[i].localName == "canvas" || objs[i].localName == "img" || objs[i].localName == "div" || objs[i].localName == "span"))
		{
			var ctx = document.createElement("canvas").getContext("2d");
			var w = objs[i].width,
				h = objs[i].height,
				alpha;

			if (w === null || typeof w != "number")
				w = rect.width;

			if (h === null || typeof h != "number")
				h = rect.height;

			ctx.canvas.width = w;
			ctx.canvas.height = h;
			var btKenn = getButtonKennung(objs[i].parentNode.id);

			if (btKenn === null)
				continue;

			var button = getButton(btKenn[0], btKenn[1]);
			var btPars = findButtonDistinct(btKenn[0], btKenn[1]);

			if (button === null || btPars === null)
				continue;

			if (btPars.enabled == 0)
				continue;

			if (hasGraphic(button))
			{
				if (objs[i].localName == "img")
					ctx.drawImage(objs[i], 0, 0, w, h);
				else if (objs[i].localName == "canvas")
					ctx = objs[i].getContext("2d");
				else if (objs[i].style.backgroundImage.length > 0)
				{
					var img = document.createElement("img");
					img.src = objs[i].style.backgroundImage.slice(4, -1).replace(/["']/g, "");
					ctx.drawImage(img, 0, 0, w, h);
				}
				else
					continue;

				alpha = ctx.getImageData(x, y, 1, 1).data[3]; // [0]R [1]G [2]B [3]A
			}
			else
			{
				var col = getAMXColor(button.sr[0].cf);

				if (col.length == 4)
					alpha = col[3];
				else
					alpha = 255;

				var idx = parseInt(i) + 1;
				// If we've reached the last layer with no graphics, it is selected.
				if (idx == objs.length)
					alpha = 255;
			}

 			// If pixel is not transparent, send a click event
			if( alpha != 0)
			{
				var name1 = "Page_"+btKenn[0]+"_Button_"+btKenn[1]+"_1";
				var name2 = "Page_"+btKenn[0]+"_Button_"+btKenn[1]+"_2";

				if (button.cp >= 0 && button.ch > 0)
				{
					if (event.type == "mousedown" || event.type == "pointerdown" || event.type == "touchdown")
					{
						if (button.fb == FEEDBACK.FB_MOMENTARY)
							switchDisplay(name1, name2, 1, button.cp, button.ch);
						else if (button.fb == FEEDBACK.FB_INV_CHANNEL)
							pushButton(button.cp, button.ch, 0);
						else
							pushButton(button.cp, button.ch, 1);

						if (button.op.length > 0)
							sendString(button.cp, button.ch, button.op);

						return;
					}
					else if (event.type == "mouseup" || event.type == "pointerup" || event.type == "touchup")
					{
						if (button.fb == FEEDBACK.FB_MOMENTARY)
							switchDisplay(name1, name2, 0, button.cp, button.ch);
						else if (button.fb == FEEDBACK.FB_INV_CHANNEL)
							pushButton(button.cp, button.ch, 1);
						else
							pushButton(button.cp, button.ch, 0);

						return;
					}
					else
					{
						writeTextOut("PUSH:"+button.cp+":"+button.ch+":1;");
						writeTextOut("PUSH:"+button.cp+":"+button.ch+":0;");

						if (button.op.length > 0)
							sendString(button.cp, button.ch, button.op);

						return;
					}
				}
			}
		}
	}
}
function getPageSize()
{
	var ID = Pages.pages[0].ID;
	var pgKey = eval("structPage"+ID);

	var width = pgKey.width;
	var height = pgKey.height;
	return [width, height];
}
function isModal(name)
{
	for (var i in Popups.pages)
	{
		var pop = Popups.pages[i];

		if (pop.name == name)
			return pop.modality;
	}

	return false;
}
function stopEvent(event)
{
	if (event.target.style.zIndex >= z_index ||
			event.target.style.zIndex < 0 ||
			event.target.style.zIndex === null ||
			typeof event.target.style.zIndex != "numeric")
		return true;

	event.stopPropagation();
	debug("stopEvent: Event \""+event.type+"\" was stopped. z_index: "+event.target.style.zIndex+", max: "+z_index);
	return false;
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
		errlog("drawPage: Couldn't find a page with the name "+name+"!");
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
		errlog("drawPopup: Couldn't find a popup with the name "+name+"!");
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

			if (isModal(pgKey.name))
			{
				var modal = document.createElement('div');
				var pgSize = getPageSize();
				modal.id = "Page_"+pageID+"_modal";
				modal.style.zIndex = newZIndex();
				modal.style.display = "inline-block";
				modal.style.position = "absolute";
				modal.style.left = 0+"px";
				modal.style.top = 0+"px";
				modal.style.width = pgSize[0]+"px";
				modal.style.height = pgSize[1]+"px";
				modal.style.backgroundColor = rgba(0, 0, 0, 0);
				modal.addEventListener("click", stopEvent, { capture: true });
				modal.addEventListener(EVENT_DOWN, stopEvent, { capture: true });
				modal.addEventListener(EVENT_UP, stopEvent, { capture: true });
				div.appendChild(modal);
				div = modal;
			}

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
		errlog("doDraw: Error getting page type: "+e);
		return false;
	}

	// The base popup
	if (what == PAGETYPE.SUBPAGE)
		page.id = "Page_"+pageID;

	if (pgKey.timeout > 0)
		window.setTimeout(tmPPT.bind(null, pgKey.name), pgKey.timeout * 100);

	page.style.position = "absolute";
	page.style.left = pgKey.left+"px";
	page.style.top = pgKey.top+"px";
	page.style.width = pgKey.width+"px";
	page.style.height = pgKey.height+"px";

	if (pgKey.showEffect > 0)
	{
		var pdim = getPageSize();
		var totalHeight = pdim[1];

		var style = "";

		switch(pgKey.showEffect)
		{
			case 1: 	// Fade
				style = "from { bottom: "+totalHeight+"px; opacity: 0; }";
				style += "to { bottom: "+(totalHeight-pgKey.height)+"px; opacity: 1; }"
			break;

			case 2:		// Slide left
			case 6:		// Slide left fade
				style = "from { left: -"+pgKey.left+"px; opacity: 0; }";
				style += "to { left: 0px; opacity: 1; }"
			break;

			case 3:		// Slide right
			case 7:		// Slide right fade
				style = "from { right: -"+(pgKey.left+pgKey.width)+"px; opacity: 0; }";
				style += "to { right: 0px; opacity: 1; }";
			break;

			case 4:		// Slide top
			case 8:		// Slide top fade
				style = "from { top: -"+pgKey.top+"px; opacity: 0; }";
				style += "to { top: 0px; opacity: 1; }";
			break;

			case 5:		// Slide bottom
			case 9:		// Slide bottom fade
				style = "from { top: "+totalHeight+"px; opacity: 0; }";
				style += "to { top: "+(totalHeight-pgKey.height)+"px; opacity: 1; }";
			break;
		}

		setCSSanim(pgKey.name, style);
		page.style.animationName = "ani_"+pgKey.name;
		page.style.animationDuration = (pgKey.showTime / 10.0) + "s";
	}

/*	if (pgKey.hideEffect > 0)
	{

	}
*/
	for (i in pgKey.sr)        // Page background and color
	{
		var sr = pgKey.sr[i];
		page.style.opacity = 1.0 / 255.0 * sr.oo;

		if (sr.cf.length > 0 && sr.mi.length == 0)
			page.style.backgroundColor = getWebColor(sr.cf);

		if (sr.ct.length > 0)
			page.style.color = getWebColor(sr.ct);

		if (sr.mi.length > 0)	// chameleon image?
		{
			try
			{
				var width = sr.mi_width;
				var height = sr.mi_height;
				var css = calcImagePosition(width, height, pgKey, CENTER_CODE.SC_BITMAP, sr.number);
				setCSSclass("Page_"+pageID+"_canvas", css+"display: flex; order: 1;");

				if (sr.bm.length > 0)
					drawButton(makeURL("images/"+sr.mi),makeURL("images/"+sr.bm),"Page_"+pageID,width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
				else
					drawArea(makeURL("images/"+sr.mi),"Page_"+pageID, width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
			}
			catch(e)
			{
				errlog("doDraw: Error getting image dimensions: "+e);
				return false;
			}
		}
		else if (sr.bm.length > 0)
		{
			page.style.backgroundImage = "url('images/"+sr.bm+"')";
			page.style.backgroundRepeat = "no-repeat";

			switch (sr.jb)
			{
				case 0:
					page.style.backgroundPositionX = sr.ix+'px';
					page.style.backgroundPositionY = sr.iy+'px';
				break;

				case 1: page.style.backgroundPosition = "left top"; break;
				case 2: page.style.backgroundPosition = "center top"; break;
				case 3: page.style.backgroundPosition = "right top"; break;
				case 4: page.style.backgroundPosition = "left center"; break;
				case 6: page.style.backgroundPosition = "right center"; break;
				case 7: page.style.backgroundPosition = "left bottom"; break;
				case 8: page.style.backgroundPosition = "center bottom"; break;
				case 9: page.style.backgroundPosition = "right bottom"; break;
				default:
					page.style.backgroundPosition = "center center";
			}
		}
	}

	// The elements of the page
	for (i in pgKey.buttons)
	{
		var button = pgKey.buttons[i];
		btArray = findButtonDistinct(pageID, button.bID);

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

			if (button.hs != "passThru")
			{
				if (button.cp > 0 && button.ch > 0 && button.sr.length == 2)		// clickable?
				{
					if (button.fb == FEEDBACK.FB_MOMENTARY)
					{
						var name1 = "Page_"+pageID+"_Button_"+button.bID+"_"+button.sr[0].number;
						var name2 = "Page_"+pageID+"_Button_"+button.bID+"_"+button.sr[1].number;

						if (button.hs.length == 0)
						{
							bt.addEventListener(EVENT_DOWN, function(event) {
								var pos = this.id.lastIndexOf('_');
								var nm;

								if (pos > 0)
									nm = this.id.substr(0, pos);
								else
									nm = this.id;

								activeTouch(event, nm, this);
							},false);
							bt.addEventListener(EVENT_UP, function(event) {
								var pos = this.id.lastIndexOf('_');
								var nm;

								if (pos > 0)
									nm = this.id.substr(0, pos);
								else
									nm = this.id;

								activeTouch(event, nm, this);
							},false);
						}
						else
						{
							bt.addEventListener(EVENT_DOWN, switchDisplay.bind(null, name1,name2,1,button.cp,button.ch),false);

							if (button.op.length > 0)
								bt.addEventListener(EVENT_DOWN, sendString.bind(null, button.cp, button.ch, button.op), false);

							bt.addEventListener(EVENT_UP, switchDisplay.bind(null, name1,name2,0,button.cp,button.ch), false);
						}
					}
					else if (button.fb == FEEDBACK.FB_CHANNEL || button.fb == 0)
					{
						if (button.hs.length == 0)
						{
							bt.addEventListener(EVENT_DOWN, function(event) {
								var pos = this.id.lastIndexOf('_');
								var nm;

								if (pos > 0)
									nm = this.id.substr(0, pos);
								else
									nm = this.id;

								activeTouch(event, nm, this);
							},false);
							bt.addEventListener(EVENT_UP, function(event) {
								var pos = this.id.lastIndexOf('_');
								var nm;

								if (pos > 0)
									nm = this.id.substr(0, pos);
								else
									nm = this.id;

								activeTouch(event, nm, this);
							},false);
						}
						else
						{
							bt.addEventListener(EVENT_DOWN, pushButton.bind(null, button.cp,button.ch,1),false);

							if (button.op.length > 0)
								bt.addEventListener(EVENT_DOWN, sendString.bind(null, button.cp, button.ch, button.op), false);

							bt.addEventListener(EVENT_UP, pushButton.bind(null, button.cp,button.ch,0),false);
						}
					}
					else if (button.fb == FEEDBACK.FB_INV_CHANNEL)
					{
						if (button.hs.length == 0)
						{
							bt.addEventListener(EVENT_DOWN, function(event) {
								var pos = this.id.lastIndexOf('_');
								var nm;

								if (pos > 0)
									nm = this.id.substr(0, pos);
								else
									nm = this.id;

								activeTouch(event, nm, this);
							},false);
							bt.addEventListener(EVENT_UP, function(event) {
								var pos = this.id.lastIndexOf('_');
								var nm;

								if (pos > 0)
									nm = this.id.substr(0, pos);
								else
									nm = this.id;

								activeTouch(event, nm, this);
							},false);
						}
						else
						{
							bt.addEventListener(EVENT_DOWN, pushButton.bind(null, button.cp,button.ch,0),false);

							if (button.op.length > 0)
								bt.addEventListener(EVENT_DOWN, sendString.bind(null, button.cp, button.ch, button.op), false);

							bt.addEventListener(EVENT_UP, pushButton.bind(null, button.cp,button.ch,1),false);
						}
					}
					else if (button.fb == FEEDBACK.FB_ALWAYS_ON)
					{
						if (button.hs.length == 0)
						{
							bt.addEventListener(EVENT_DOWN, function(event) {
								var pos = this.id.lastIndexOf('_');
								var nm;

								if (pos > 0)
									nm = this.id.substr(0, pos);
								else
									nm = this.id;

								activeTouch(event, nm, this);
							},false);
						}
						else
						{
							bt.addEventListener(EVENT_DOWN, pushButton.bind(null, button.cp,button.ch,1));

							if (button.op.length > 0)
								bt.addEventListener(EVENT_DOWN, sendString.bind(null, button.cp, button.ch, button.op), false);
						}
					}
				}
				else if (button.sr.length == 2 && button.op.length > 0)
					bt.addEventListener(EVENT_DOWN, sendString.bind(null, 1, 1, button.op), false);

				for (var x in button.pf)
				{
					var pf = button.pf[x];

					if (pf.pfType == "sShow")			// show popup
						bt.addEventListener(EVENT_UP, showPopup.bind(null, pf.pfName));
					else if (pf.pfType == "sHide")		// hide popup
						bt.addEventListener(EVENT_UP, hidePopup.bind(null, pf.pfName));
					else if (pf.pfType == "scGroup")	// hide group
						bt.addEventListener(EVENT_UP, hideGroup.bind(null, pf.pfName));
				}
			}

			var block = false;

			for (j in button.sr)
			{
				var sr = button.sr[j];
				var bsr = document.createElement('div');
				bsr.style.position = "absolute";
				bsr.style.left = "0px";
				bsr.style.top = "0px";
				bsr.style.width = bt.style.width;
				bsr.style.height = bt.style.height;
				bsr.style.opacity = 1.0 / 255.0 * sr.oo;
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
						for (var x = 0; x < brd.length; x++)
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

				var idx = parseInt(j);

				if ((button.btype != BUTTONTYPE.BARGRAPH && button.btype != BUTTONTYPE.MULTISTATE_BARGRAPH && button.btype != BUTTONTYPE.MULTISTATE_GENERAL) &&
					button.sr.length == 2 && sr.mi.length > 0)	// chameleon image?
				{
					var width = sr.mi_width;
					var height = sr.mi_height;
					var css = calcImagePosition(width, height, button, CENTER_CODE.SC_BITMAP, sr.number);
					setCSSclass(nm+sr.number+"_canvas", css+"display: flex; order: 1;");

					if (sr.bm.length > 0)
						drawButton(makeURL("images/"+sr.mi),makeURL("images/"+sr.bm),nm+sr.number,width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
					else
						drawArea(makeURL("images/"+sr.mi),nm+sr.number, width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
				}
				else if (button.btype == BUTTONTYPE.GENERAL && sr.sb == 1 && sr.bm.length > 0)
				{
					drawButtonRemote(button, nm+sr.number, idx);
				}
				else if (button.btype == BUTTONTYPE.MULTISTATE_GENERAL && button.ar == 1 && idx == 0)
				{
					block = true;
					drawButtonMultistateAni(button, nm);
				}
				else if (button.btype == BUTTONTYPE.BARGRAPH && button.sr.length == 2 && sr.mi.length > 0 && idx == 0)
				{
					var width = sr.mi_width;
					var height = sr.mi_height;
					var css = calcImagePosition(width, height, button, CENTER_CODE.SC_BITMAP, sr.number);
					setCSSclass(nm+sr.number+"_canvas", css+"display: flex; order: 1;");
					block = true;

					if (button.sr[idx+1].bm.length > 0)
					{
						var lev = getBargraphLevel(pgKey.ID, button.bID);
						var level = parseInt(100.0 / (button.rh - button.rl) * lev);
						var dir = true;
						var clickable = false;

						if (button.dr == "horizontal")
							dir = false;

						if (button.cp > 0 && button.ch > 0)
							clickable = true;

						drawBargraph(makeURL("images/"+sr.mi), makeURL("images/"+button.sr[idx+1].bm), nm+sr.number, level, width, height, getAMXColor(button.sr[idx+1].cf), getAMXColor(button.sr[idx+1].cb), dir, clickable, button);
					}
					else
						drawArea(makeURL("images/"+sr.mi), nm+sr.number, width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
				}
				else if (button.btype == BUTTONTYPE.BARGRAPH && button.sr.length == 2 && !hasGraphic(button, idx) && idx == 0)
				{
					var width = button.wt;
					var height = button.ht;
					setCSSclass(nm+sr.number+"_canvas", "display: flex; order: 1;");
					block = true;

					var lev = getBargraphLevel(pgKey.ID, button.bID);
					var level = parseInt(100.0 / (button.rh - button.rl) * lev);
					var dir = true;
					var clickable = false;

					if (button.dr == "horizontal")
						dir = false;

					if (button.cp > 0 && button.ch > 0)
						clickable = true;

					drawBargraphLight(nm+sr.number, level, width, height, getWebColor(button.sr[idx+1].cf), getWebColor(sr.cf), dir, clickable, button);
				}
				else if (button.btype == BUTTONTYPE.BARGRAPH && button.sr.length == 2 && sr.mi.length == 0 && sr.bm.length > 0 &&
						 hasGraphic(button, 1) && idx == 0)
				{
					var width = button.wt;
					var height = button.ht;
					setCSSclass(nm+sr.number+"_canvas", "display: flex; order: 1;");
					block = true;

					var lev = getBargraphLevel(pgKey.ID, button.bID);
					var level = parseInt(100.0 / (button.rh - button.rl) * lev);
					var dir = true;
					var clickable = false;

					if (button.dr == "horizontal")
						dir = false;

					if (button.cp > 0 && button.ch > 0)
						clickable = true;

					drawBargraph2Graph(makeURL("images/"+button.sr[idx+1].bm), makeURL("images/"+button.sr[idx].bm), nm+sr.number, level, width, height, dir, clickable, button);
				}
				else if (button.btype == BUTTONTYPE.MULTISTATE_BARGRAPH && idx == 0)
				{
					var lev = getBargraphLevel(pgKey.ID, button.bID);
					var level = parseInt(100.0 / (button.rh - button.rl) * lev);
					block = true;
					drawBargraphMultistate(button, nm, level);
				}
				else if (sr.bm.length > 0 && !block)
				{
					bsr.style.backgroundImage = "url('images/"+sr.bm+"')";
					bsr.style.backgroundRepeat = "no-repeat";

					switch (sr.jb)
					{
						case 0:
							bsr.style.backgroundPositionX = sr.ix+'px';
							bsr.style.backgroundPositionY = sr.iy+'px';
						break;

						case 1: bsr.style.backgroundPosition = "left top"; break;
						case 2: bsr.style.backgroundPosition = "center top"; break;
						case 3: bsr.style.backgroundPosition = "right top"; break;
						case 4: bsr.style.backgroundPosition = "left center"; break;
						case 6: bsr.style.backgroundPosition = "right center"; break;
						case 7: bsr.style.backgroundPosition = "left bottom"; break;
						case 8: bsr.style.backgroundPosition = "center bottom"; break;
						case 9: bsr.style.backgroundPosition = "right bottom"; break;
						default:
							bsr.style.backgroundPosition = "center center";
					}
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
						justifyImage(img, button, CENTER_CODE.SC_ICON, sr.number);
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
					fnt.style.paddingLeft = "4px";
					fnt.style.paddingRight = "4px";
					var border = getBorderSize(sr.bs);

					if (sr.jt != TEXT_ORIENTATION.ORI_ABSOLUT)
					{
						fnt.style.width = bsr.style.width - border * 2;
						fnt.style.height = bsr.style.height - border * 2;
					}

					// Prevent text from being selected.
					fnt.style.webkitTouchCallout = 'none';
					fnt.style.webkitUserSelect = 'none';
					fnt.style.khtmlUserSelect = 'none';
					fnt.style.mozUserSelect = 'none';
					fnt.style.userSelect = 'none';
					fnt.style.pointerEvents = 'none';
					fnt.style.fontFamily = "\""+font.name+"\"";
					fnt.style.fontSize = font.size+"pt";
					fnt.style.fontStyle = getFontStyle(font.subfamilyName);
					fnt.style.fontWeight = getFontWeight(font.subfamilyName);
					bsr.appendChild(fnt);

					switch(sr.jt)
					{
						case TEXT_ORIENTATION.ORI_ABSOLUT:
							fnt.style.left = sr.tx+"px";
							fnt.style.top = sr.ty+"px";
							fnt.style.width = (bsr.style.width - (sr.tx + border))+'px';
							fnt.style.height = (bsr.style.height - (sr.ty + border))+'px';
						break;
						case TEXT_ORIENTATION.ORI_TOP_LEFT:
							fnt.style.left = "0px";
							fnt.style.top = "0px";
						break;
						case TEXT_ORIENTATION.ORI_TOP_MIDDLE:
							fnt.style.left = "50%";
							fnt.style.transform = "translateX(-50%)";
							fnt.style.top = "0px";
						break;
						case TEXT_ORIENTATION.ORI_TOP_RIGHT:
							fnt.style.right = "0px";
							fnt.style.top = "0px";
						break;
						case TEXT_ORIENTATION.ORI_CENTER_LEFT:
							fnt.style.top = '50%';
							fnt.style.left = "0px";
							fnt.style.transform = "translate(0%, -50%)";
						break;
						case TEXT_ORIENTATION.ORI_CENTER_MIDDLE:
							fnt.style.left = "50%";
							fnt.style.top = '50%';
							fnt.style.transform = "translate(-50%, -50%)"
						break;
						case TEXT_ORIENTATION.ORI_CENTER_RIGHT:
							fnt.style.right = "0px";
							fnt.style.top = '50%';
							fnt.style.transform = "translateY(-50%)";
						break;
						case TEXT_ORIENTATION.ORI_BOTTOM_LEFT:
							fnt.style.left = "0px";
							fnt.style.bottom = "0px";
						break;
						case TEXT_ORIENTATION.ORI_BOTTOM_MIDDLE:
						fnt.style.left = "50%";
						fnt.style.transform = "translateX(-50%)";
						fnt.style.bottom = "0px";
						break;
						case TEXT_ORIENTATION.ORI_BOTTOM_RIGHT:
							fnt.style.right = "0px";
							fnt.style.bottom = "0px";
						break;
					}

					if (sr.te.length > 0)
						fnt.innerHTML = sr.te;
				}

				if (btArray !== null)
				{
					var comp = parseInt(j) + 1;

					if (button.ap == 0 && button.ad == 8)		// System network connection
					{
						if (wsStatus == idx)
							bsr.style.display = "inline";
						else
							bsr.style.display = "none";
					}
					else if (comp == btArray.ion && btArray.visible == 1)
						bsr.style.display = "inline";
					else
						bsr.style.display = "none";
				}
				else
				{
					if (button.ap == 0 && button.ad == 8)		// System network connection
					{
						if (wsStatus == idx)
							bsr.style.display = "inline";
						else
							bsr.style.display = "none";
					}
					else if (j == 0 && button.fb != FEEDBACK.FB_INV_CHANNEL && button.fb != FEEDBACK.FB_ALWAYS_ON)
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
			errlog("doDraw: Error on button '"+button.bname+"': "+e);
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
		errlog("dropPage: Error: "+e);
		return false;
	}

	return true;
}
function dropPopup(name)
{
	var pageID = 0;
	var pname = "";

	for (var i in Popups.pages)
	{
		if (Popups.pages[i].name == name)
		{
			pageID = Popups.pages[i].ID;
			pname = Popups.pages[i].name;
			break;
		}
	}

	if (pageID == 0)
	{
		errlog("dropPopup: Couldn't find a popup with the name "+name+"!");
		return false;
	}

	try
	{
		var divname = "";

		if (isModal(pname))
			divname = 'Page_'+pageID+"_modal";
		else
			divname = 'Page_'+pageID;

		var div = document.getElementById(divname);

		while (div.hasChildNodes())
			div.removeChild(div.firstChild);

		div.parentNode.removeChild(div);

		if (isModal(pname))
			freeZIndex();
	}
	catch(e)
	{
		errlog("dropPopup: Error: "+e);
		return false;
	}

	return true;
}
