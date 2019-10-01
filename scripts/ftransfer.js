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
var _BLOCK_ALL = false;

function doFTR(msg)
{
	var cmd = getField(msg, 0, ':');
	var pgSize = getPageSize();
	var unit = pgSize[1] / 10;

	if (cmd == "START")
	{
		_BLOCK_ALL = true;
		dropPage();
		var main = document.getElementById("main");
		main.style.backgroundColor = "rgb(0, 0, 0)";
		main.style.backgroundImage = "";
		main.style.color = "rgb(255, 255, 255)";
		var line1Div = document.createElement("div");
		var line2Div = document.createElement("div");
		var line1 = document.createElement("span");
		var line2 = document.createElement("span");
		var progres1 = document.createElement("div");
		var progres2 = document.createElement("div");
		var width = pgSize[0];
		var height = pgSize[1];
		var lnHeight = unit / 100.0 * 80;
		var font = findFont(21);
		debug("doFTR: [START] width="+width+", height="+height+", unit="+unit+", lnHeight="+lnHeight);

		line1Div.id = "Line1";
		line1Div.style.position = "absolute";
		line1Div.style.width = (width / 100.0 * 80) + "px";
		line1Div.style.height = lnHeight + "px";
		line1Div.style.top = (unit * 2) + "px";
		line1Div.style.left = ((width - (width / 100.0 * 80)) / 2) + "px";

		line2Div.id = "Line2";
		line2Div.style.position = "absolute";
		line2Div.style.width = (width / 100.0 * 80)+"px";
		line2Div.style.height = lnHeight+"px";
		line2Div.style.top = (unit * 7) + "px";
		line2Div.style.left = ((width - (width / 100.0 * 80)) / 2) + "px";

		line1.id = "Line1_span";
		line1.style.position = "absolute";
		line1.style.paddingLeft = "4px";
		line1.style.paddingRight = "4px";
//		line1.style.width = (width / 100.0 * 80) + "px";
//		line1.style.height = (height / 100.0 * 10) + "px";
		line1.style.left = "50%";
		line1.style.top = '50%';
		line1.style.transform = "translate(-50%, -50%)";
		line1.style.overflow = "hidden";
		line1.style.textOverflow = "clip";
		// Prevent text from being selected.
		line1.style.webkitTouchCallout = 'none';
		line1.style.webkitUserSelect = 'none';
		line1.style.khtmlUserSelect = 'none';
		line1.style.mozUserSelect = 'none';
		line1.style.userSelect = 'none';
		line1.style.fontFamily = "\""+font.name+"\"";
		line1.style.fontSize = font.size+"pt";
		line1.style.fontStyle = getFontStyle(font.subfamilyName);
		line1.style.fontWeight = getFontWeight(font.subfamilyName);
		line1Div.appendChild(line1);

		line2.id = "Line2_span";
		line2.style.position = "absolute";
		line2.style.paddingLeft = "4px";
		line2.style.paddingRight = "4px";
//		line2.style.width = (width / 100.0 * 80) + "px";
//		line2.style.height = (height / 100.0 * 10) + "px";
		line2.style.left = "50%";
		line2.style.top = '50%';
		line2.style.transform = "translate(-50%, -50%)";
		line2.style.overflow = "hidden";
		line2.style.textOverflow = "clip";
		// Prevent text from being selected.
		line2.style.webkitTouchCallout = 'none';
		line2.style.webkitUserSelect = 'none';
		line2.style.khtmlUserSelect = 'none';
		line2.style.mozUserSelect = 'none';
		line2.style.userSelect = 'none';
		line2.style.fontFamily = "\""+font.name+"\"";
		line2.style.fontSize = font.size+"pt";
		line2.style.fontStyle = getFontStyle(font.subfamilyName);
		line2.style.fontWeight = getFontWeight(font.subfamilyName);
		line2Div.appendChild(line2);

		progres1.id = "Progres1";
		progres1.style.position = "absolute";
		progres1.style.width = (width / 100.0 * 80) + "px";
		progres1.style.height = unit + "px";
		progres1.style.top = (unit * 3) + "px";
		progres1.style.left = ((width - (width / 100.0 * 80)) / 2) + "px";
//		progres1.style.borderStyle = "solid";
//		progres1.style.borderWidth = "2px";

		var prog1 = document.createElement('progress');
		prog1.id = "bar1";
		prog1.max = 100;
		prog1.style.width = (width / 100.0 * 80.0) + "px";
		prog1.style.height = (unit / 100.0 * 80.0) + "px";
		prog1.style.backgroundColor = "rgb(0xe, 0xe, 0xe)";
		prog1.style.color = "rgb(255, 255, 255)";
		progres1.appendChild(prog1);

		progres2.id = "Progres2";
		progres2.style.position = "absolute";
		progres2.style.width = (width / 100.0 * 80) + "px";
		progres2.style.height = unit + "px";
		progres2.style.top = (unit * 5) + "px";
		progres2.style.left = ((width - (width / 100.0 * 80)) / 2) + "px";
//		progres2.style.borderStyle = "solid";
//		progres2.style.borderWidth = "2px";

		var prog2 = document.createElement('progress');
		prog2.id = "bar2";
		prog2.max = 100;
		prog2.style.width = (width / 100.0 * 80.0) + "px";
		prog2.style.height = (unit / 100.0 * 80.0) + "px";
		prog2.style.backgroundColor = "rgb(0xe, 0xe, 0xe)";
		prog2.style.color = "rgb(255, 255, 255)";
		progres2.appendChild(prog2);

		main.appendChild(line1Div);
		main.appendChild(progres1);
		main.appendChild(progres2);
		main.appendChild(line2Div);
	}
	else if (cmd == "SYNC")
	{
		var line1 = document.getElementById("Line1_span");
		line1.innerHTML = getField(msg, 2, ':');
		var percent = getField(msg, 1, ':');

		try
		{
			document.getElementById("bar1").value = percent;
		}
		catch(e)
		{
			errlog("doFTR: [FTRPART] "+e);
		}
	}
	else if (cmd == "FTRSTART")
	{
		try
		{
			document.getElementById("Line1_span").innerHTML = "Transfering files ...";
			document.getElementById("Line2_span").innerHTML = getField(msg, 3, ':');
			document.getElementById('bar1').value = getField(msg, 1, ':');
			document.getElementById('bar2').value = getField(msg, 2, ':');;
		}
		catch(e)
		{
			errlog("doFTR: [FTRSTART] "+e);
		}
	}
	else if (cmd == "FTRPART")
	{
		var percent = getField(msg, 2, ':');

		try
		{
			document.getElementById("bar2").value = percent;
		}
		catch(e)
		{
			errlog("doFTR: [FTRPART] "+e);
		}
	}
	else if (cmd == "END")
	{
		_BLOCK_ALL = false;
		dropPage();
		location.reload(true);
	}
}
