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
function drawPage(name)
{
	var i;
	var pageID = 0;

	for (i in Pages.pages)
	{
		if (Pages.pages[i].name == name)
		{
			pageID = Ppages.pages[i].ID;
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

	try
	{
		if (what == PAGETYPE.SUBPAGE)
		{
			var div = document.getElementById('main');
			page = document.createElement('div');
			div.appendChild(page);
		}
		else
		{
			page = document.getElementById('main');
			page.style.overflow = "hidden";
		}
	}
	catch(e)
	{
		console.log("doDraw: Error: "+e);
		return false;
	}

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
			try
			{
				var idim = getImageSize(makeURL("images/"+sr.mi));
				var width = idim[0];
				var height = idim[1];

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

		try
		{
			var bt = document.createElement('div');
			page.appendChild(bt);
			bt.id = "Button_"+button.bID;
			bt.style.position = "absolute";
			bt.style.left = button.lt+"px";
			bt.style.top = button.tp+"px";
			bt.style.width = button.wt+"px";
			bt.style.height = button.ht+"px";

			if (button.hs == "bounding")
				bt.style.overflow = "hidden";

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

			if (button.pfType == "sShow")			// show popup
				bt.addEventListener("click", showPopup(button.pfName));
			else if (button.pfType == "sHide")		// hide popup
				bt.addEventListener("click", hidePopup(button.pfName));
			else if (button.pfType == "scGroup")	// hide group
				bt.addEventListener("click", hideGroup(button.pfName));

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
					else
						bsr.style.border = "none";
				}
				else
					bsr.style.border = "none";

				if (!(button.btype == BUTTONTYPE.BARGRAPH && button.sr.length == 2) && sr.mi.length > 0)	// chameleon image?
				{
					var idim = getImageSize("images/"+sr.mi);
					var width = idim[0];
					var height = idim[1];

					if (button.bm.length > 0)
						drawButton(makeURL("images/"+sr.mi),makeURL("images/"+sr.bm),nm+sr.number,width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));
					else
						drawArea(makeURL("images/"+sr.mi),nm+sr.number, width, height, getAMXColor(sr.cf), getAMXColor(sr.cb));

					var can = document.getElementById(nm+sr.number+"_canvas");
					can.style.position = "absolute";
					can.style.left = 0+'px';
					can.style.top = 0+'px';
				}
				else if (sr.bm.length > 0)
				{
					bsr.style.backgroundImage = "url('images/"+sr.bm+"')";
					bsr.style.backgroundRepeat = "no-repeat";
				}

				if (sr.ii > 0)		// Icon?
				{
					var ico = getIconFile(sr.ii);

					if (ico !== -1)
					{
						var idim = getImageSize("images/"+ico);
						var width = idim[0];
						var height = idim[1];

						var img = document.createElement('img');
						bsr.appendChild(img);
						img.id = nm+sr.number+'_img';
						img.src = makeURL('images/'+ico);

						if (sr.ji == 0)
						{
							if ((sr.ix + width) > button.wt)
							{
								var ix = button.wt - width;

								if (ix < 0)
								{
									ix = 0;
									img.width = button.wt+"px";
								}
								else
									img.width = width+"px";
							}
							else
								img.width = width+"px";

							if ((sr.iy + height) > button.ht)
							{
								var iy = button.ht - height;

								if (iy < 0)
								{
									iy = 0;
									img.height = button.ht+"px";
								}
								else
									img.height = height+"px";
							}
							else
								img.height = height+"px";
						}

						if (sr.ji == 0)
						{
							img.style.position = "absolute";
							img.style.left = sr.ix+"px";
							img.style.top = sr.iy+"px";
						}
						else if (sr.ji == 1)
						{
							img.style.position = "absolute";
							img.style.left = "0px";
							img.style.top = "0px";
						}
						else
						{
							img.addEventListener("load", posImage(img, nm+sr.number+'_img',sr.ji));
						}
					}
				}

				var font = findFont(sr.fi);

				if (font !== -1)
				{
					var fnt = document.createElement('span');
					fnt.style.position = "absolute";
					fnt.style.left = "0px";
					fnt.style.top = "0px";
					fnt.style.width = bsr.style.width;
					fnt.style.height = bsr.style.height;
					bsr.appendChild(fnt);
					fnt.id = nm+sr.number+'_font';
					fnt.style.fontFamily = font.name;
					fnt.style.fontSize = font.size+"pt";
					fnt.style.fontStyle = getFontStyle(font.subfamilyName);
					fnt.style.fontWeight = getFontWeight(font.subfamilyName);

					if (sr.ww  == 0 && (sr.jt == TEXT_ORIENTATION.ORI_CENTER_LEFT || sr.jt == TEXT_ORIENTATION.ORI_CENTER_MIDDLE || sr.jt == TEXT_ORIENTATION.ORI_CENTER_RIGHT))
						fnt.style.lineHeight = button.ht+"px";

					switch(sr.jt)
					{
						case TEXT_ORIENTATION.ORI_ABSOLUT:
							fnt.style.left = sr.tx+"px";
							fnt.style.top = sr.ty+"px";
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
console.log("doDraw: name: "+nm+", button.fb: "+button.fb);
				if (j == 0 && button.fb != FEEDBACK.FB_INV_CHANNEL && button.fb != FEEDBACK.FB_ALWAYS_ON)
					bsr.style.display = "inline";
				else if (j == 1 && button.fb == FEEDBACK.FB_INV_CHANNEL && button.fb == FEEDBACK.FB_ALWAYS_ON)
					bsr.style.display = "inline";
				else
					bsr.style.display = "none";
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
			div.removeChild(div.firstChild());
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
		var div = documant.getElementById('Page_'+pageID);

		while (div.hasChildNodes())
			div.removeChild(div.firstChild());
	}
	catch(e)
	{
		console.log("dropPopup: Error: "+e);
		return false;
	}

	return true;
}
