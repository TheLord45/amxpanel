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
	SCR_NONE:			Symbol(0),
	SCR_TIME_STANDARD:	Symbol(1),
	SCR_TIME_AMPM:		Symbol(2),
	SCR_TIME_24:		Symbol(3),
	SCR_DATE_WEEKDAY:	Symbol(4),
	SCR_DATE_D_MONTH_Y:	Symbol(5),
	SCR_DATE_D_M:		Symbol(6),
	SCR_DATE_D_M_Y:		Symbol(7),
	SCR_DATE_M_D:		Symbol(8),
	SCR_DATE_M_D_Y:		Symbol(9),
	SCR_DATE_MONTH_D_Y:	Symbol(10),
	SCR_DATE_Y_M_D:		Symbol(11)
});

const TEXT_ORIENTATION = Object.freeze({
	ORI_ABSOLUT:		Symbol(0),
	ORI_TOP_LEFT:		Symbol(1),
	ORI_TOP_MIDDLE:		Symbol(2),
	ORI_TOP_RIGHT:		Symbol(3),
	ORI_CENTER_LEFT:	Symbol(4),
	ORI_CENTER_MIDDLE:	Symbol(5),	// default
	ORI_CENTER_RIGHT:	Symbol(6),
	ORI_BOTTOM_LEFT:	Symbol(7),
	ORI_BOTTOM_MIDDLE:	Symbol(8),
	ORI_BOTTOM_RIGHT:	Symbol(9)
});

const BUTTONTYPE = Object.freeze({
	NONE:					Symbol(0),
	GENERAL:				Symbol(1),
	MULTISTATE_GENERAL:		Symbol(2),
	BARGRAPH:				Symbol(3),
	MULTISTATE_BARGRAPH:	Symbol(4),
	JOISTICK:				Symbol(5),
	TEXT_INPUT:				Symbol(6),
	COMPUTER_CONTROL:		Symbol(7),
	TAKE_NOTE:				Symbol(8),
	SUBPAGE_VIEW:			Symbol(9)
});

const FEEDBACK = Object.freeze({
	FB_NONE:		Symbol(0),
	FB_CHANNEL:		Symbol(1),
	FB_INV_CHANNEL:	Symbol(2),	// inverted channel
	FB_ALWAYS_ON:	Symbol(3),
	FB_MOMENTARY:	Symbol(4),
	FB_BLINK:		Symbol(5)
});

const PAGETYPE = Object.freeze({
	PNONE:		Symbol(0),
	PAGE:		Symbol(1),
	SUBPAGE:	Symbol(2)
});

const SHOWEFFECT = Object.freeze({
	SE_NONE:				Symbol(0),
	SE_FADE:				Symbol(1),
	SE_SLIDE_LEFT:			Symbol(2),
	SE_SLIDE_RIGHT:			Symbol(3),
	SE_SLIDE_TOP:			Symbol(4),
	SE_SLIDE_BOTTOM:		Symbol(5),
	SE_SLIDE_LEFT_FADE:		Symbol(6),
	SE_SLIDE_RIGHT_FADE:	Symbol(7),
	SE_SLIDE_TOP_FADE:		Symbol(8),
	SE_SLIDE_BOTTOM_FADE:	Symbol(9)
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

function drawPopup(name)
{
	var i, j;
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

	var pgKey = "structPage"+pageID;

	var div = document.getElementById('main');
	var page = div.createElement('div');
	// The base popup
	page.id = "Page_"+pageID;
	page.style.position = "absolute";
	page.style.left = pgKey.left+"px";
	page.style.top = pgKey.top+"px";
	page.style.width = pgKey.width+"px";
	page.style.height = pgKey.height+"px";

	for (i in pgKey.sr)
	{
		var sr = pgKey.sr[i];

		if (sr.cf.length > 0 && sr.mi.length == 0)
			page.style.backgroundColor = getWebColor(sr.cf);

		if (sr.ct.length > 0)
			page.style.color = getWebColor(sr.ct);

		if (sr.mi.length > 0)	// chameleon image?
		{
			var img = new Image();
			img.src = makeURL("images/"+sr.mi);

			if (sr.bm.length > 0)
				drawButton(makeURL("images/"+sr.mi),makeURL("images/"+sr.bm),"Page_"+pageID,img.width, img.height, getAMXColor(sr.cf), getAMXColor(sr.cb));
			else
				drawArea(makeURL("images/"+sr.mi),"Page_"+pageID,img.width, img.height, getAMXColor(sr.cf), getAMXColor(sr.cb));
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

		var bt = page.createElement('div');
		bt.id = "Button_"+button.bID;
		bt.style.position = "absolute";
		bt.style.left = button.lt+"px";
		bt.style.top = pgKey.tp+"px";
		bt.style.width = pgKey.wt+"px";
		bt.style.height = pgKey.ht+"px";

		if (button.hs == "bounding")
			bt.style.overflow = "hidden";

		for (j in button.sr)
		{
			var sr = button.sr[j];
			var bsr = bt.createElement('div');
			var nm = "Button_"+button.bID+"_"

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
			}

			if (button.cp > 0 && button.ch > 0)		// clickable?
			{
				if (button.fb == FEEDBACK.FB_MOMENTARY)
				{
					bt.addEventListener('mousedown',switchDisplay(nm+buttin.sr[0].number,nm+button.sr[1].number,1,button.cp,button.ch));
					bt.addEventListener('mouseup',switchDisplay(nm+buttin.sr[0].number,nm+button.sr[1].number,0,button.cp,button.ch));
				}
				else if (button.fb == FEEDBACK.FB_CHANNEL)
				{
					bt.addEventListener('mousedown',pushButton(button.cp,button.ch,1));
					bt.addEventListener('mouseup',pushButton(button.cp,button.ch,0));
				}
				else if (button.fb == FEEDBACK.FB_INV_CHANNEL)
				{
					bt.addEventListener('mousedown',pushButton(button.cp,button.ch,0));
					bt.addEventListener('mouseup',pushButton(button.cp,button.ch,1));
				}
				else if (button.fb == FEEDBACK.FB_ALWAYS_ON)
					bt.addEventListener('click',pushButton(button.cp,button.ch,1));
			}

			if (!(button.btype == BUTTONTYPE.BARGRAPH && button.sr.length == 2) && button.mi.length > 0)	// chameleon image?
			{
				var img = new Image();
				img.src = makeURL("images/"+sr.mi);

				if (button.bm.length > 0)
					drawButton(makeURL("images/"+sr.mi),makeURL("images/"+sr.bm),nm+sr.number,img.width, img.height, getAMXColor(sr.cf), getAMXColor(sr.cb));
				else
					drawArea(makeURL("images/"+sr.mi),nm+sr.number,img.width, img.height, getAMXColor(sr.cf), getAMXColor(sr.cb));

				var can = document.getElementById(nm+sr.number+"_canvas");
				can.style.position = "absolute";
				can.style.left = 0+'px';
				can.style.top = 0+'px';
			}
			else if (sr.bm.length > 0)
			{
				page.style.backgroundImage = "url('images/"+sr.bm+"')";
				page.style.backgroundRepeat = "no-repeat";
			}

			if (i == 0 && button.fb != FEEDBACK.FB_INV_CHANNEL && button.fb != FEEDBACK.FB_ALWAYS_ON)
				bsr.style.display = "inline";
			else if (i == 1 && button.fb == FEEDBACK.FB_INV_CHANNEL && button.fb == FEEDBACK.FB_ALWAYS_ON)
				bsr.style.display = "inline";
			else
				bsr.style.display = "none";
		}
	}

	return true;
}
