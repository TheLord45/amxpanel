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

const sleep = (milliseconds) => {
    return new Promise(resolve => setTimeout(resolve, milliseconds));
}

var curPort;                // The port number the currently processed command depends on
var curCommand;             // The currently command stripped from the port number
var registrationID;         // The unique ID the App is registered on Server
var regStatus = false;      // TRUE = Registration to server was successfull
var isBackground = false;   // TRUE = App is in background
var isStandby = false;      // TRUE = Device is in standby (display is off)
var z_index = 0;            // Draw order of elements
var panelID = 0;            // The ID of the panel in the range from 10000 to 11000
var hdOffTimer = null;
var __debug = true;         // TRUE = Display debugging messages
var __errlog = true;        // TRUE = Display error messages
var __TRACE = true;         // TRUE = Display trace messages

var cmdArray = {
    "commands": [
        { "cmd": "@WLD-", "call": unsupported },
        { "cmd": "@AFP-", "call": doAFP },
        { "cmd": "@GCE-", "call": unsupported },
        { "cmd": "@APG-", "call": doAPG },	// Add a popup to a popup group
        { "cmd": "@CPG-", "call": doCPG },	// Clear all popups from a group
        { "cmd": "@DPG-", "call": doDPG },	// Delete a specific popup page from specified popup group if it exists.
        { "cmd": "@PDR-", "call": unsupported },	// Set the popup location reset flag
        { "cmd": "@PHE-", "call": doPHE },	// Set the hide effect for the specified popup page to the named hide effect.
        { "cmd": "@PHP-", "call": unsupported },	// Set the hide effect position.
        { "cmd": "@PHT-", "call": doPHT },	// Set the hide effect time for the specified popup page.
        { "cmd": "@PPA-", "call": doPPA },	// Close all popups on a specified page.
        { "cmd": "@PPF-", "call": doPPF },	// Popup off
        { "cmd": "@PPG-", "call": doPPG }, 	// Toggle a popup
        { "cmd": "@PPK-", "call": doPPK },	// Close popup on all pages
        { "cmd": "@PPM-", "call": doPPM },	// Set the modality of a specific popup page to Modal or NonModal.
        { "cmd": "@PPN-", "call": doPPN },	// Popup on
        { "cmd": "@PPT-", "call": doPPT },	// Set a specific popup page to timeout within a specified time.
        { "cmd": "@PPX",  "call": doPPX },	// close all popups on all pages
        { "cmd": "@PSE-", "call": doPSE },	// Set the show effect for the specified popup page to the named show effect.
        { "cmd": "@PSP-", "call": unsupported },	// Set the show effect position.
        { "cmd": "@PST-", "call": doPST },	// Set the show effect time for the specified popup page.
        { "cmd": "PAGE-", "call": doPAGE },	// Flip to page
        { "cmd": "PPOF-", "call": doPPF },	// Popup off
        { "cmd": "PPOG-", "call": doPPG },	// Toggle a specific popup page on either a specified page or the current page.
        { "cmd": "PPON-", "call": doPPN },	// Popup on
        { "cmd": "^ANI-", "call": doANI },	// Run a button animation
        { "cmd": "^APF-", "call": doAPF },	// Add page flip action to button
        { "cmd": "^BAT-", "call": doBAT },	// Append non-unicode text.
        { "cmd": "^BAU-", "call": doBAU },	// Append unicode text
        { "cmd": "^BCB-", "call": doBCB },	// Set the border color to the specified color.
        { "cmd": "^BCF-", "call": doBCF },	// Set the fill color to the specified color.
        { "cmd": "^BCT-", "call": doBCT },	// Set the text color to the specified color.
        { "cmd": "^BDO-", "call": unsupported },	// Set the button draw order. Is this possible?
        { "cmd": "^BFB-", "call": doBFB },	// Set the feedback type of the button.
        { "cmd": "^BIM-", "call": doBIM },	// Set the input mask for the specified address.
        { "cmd": "^BLN-", "call": unsupported },	// Set the number of lines removed equally from the top and bottom of a composite video signal. --> will never be implemented!
        { "cmd": "^BMC-", "call": doBMC },	// Button copy command.
        { "cmd": "^BMF-", "call": doBMF },	// Set any/all button parameters by sending embedded codes and data.
        { "cmd": "^BMI-", "call": doBMI },	// Set the button mask image.
        { "cmd": "^BML-", "call": doBML },	// Set the maximum length of the text area button.
        { "cmd": "^BMP-", "call": doBMP },	// Assign a picture to those buttons with a defined addressrange.
        { "cmd": "^BNC-", "call": unsupported },	// Clear current TakeNote annotations.
        { "cmd": "^BNN-", "call": unsupported },	// Set the TakeNote network name for the specified Addresses.
        { "cmd": "^BNT-", "call": unsupported },	// Set the TakeNote network port for the specified Addresses.
        { "cmd": "^BOP-", "call": doBOP }, // Set the button opacity.
        { "cmd": "^BOR-", "call": doBOR }, // Set a border to a specific border style.
        { "cmd": "^BOS-", "call": unsupported },	// Set the button to display either a Video or Non-Video window.
        { "cmd": "^BPP-", "call": unsupported },	// Set or clear the protected page flip flag of a button.
        { "cmd": "^BRD-", "call": doBRD },	// Set the border of a button state/states.
        { "cmd": "^BSF-", "call": doBSF },	// Set the focus to the text area.
        { "cmd": "^BSP-", "call": doBSP },	// Set the button size and position.
        { "cmd": "^BSM-", "call": doBSM },	// Submit text for text area buttons.
        { "cmd": "^BSO-", "call": doBSO },	// Set the sound played when a button is pressed.
        { "cmd": "^BVL-", "call": unsupported },
        { "cmd": "^BVN-", "call": unsupported },
        { "cmd": "^BVP-", "call": unsupported },
        { "cmd": "^BVT-", "call": unsupported },
        { "cmd": "^BWW-", "call": doBWW },	// Set the button word wrap feature to those buttons with a defined address range.
        { "cmd": "^CPF-", "call": doCPF },	// Clear all page flips from a button.
        { "cmd": "^DLD-", "call": unsupported },
        { "cmd": "^DPF-", "call": doDPF },	// Delete page flips from button if it already exists.
        { "cmd": "^ENA-", "call": doENA },	// Enable or disable buttons with a set variable text range.
        { "cmd": "^FON-", "call": doFON },	// Set a font to a specific Font ID value for those buttons with a defined address range.
        { "cmd": "^GDI-", "call": doGDI },	// Change the bargraph drag increment
        { "cmd": "^GIV-", "call": unsupported },	// Invert the joystick axis to move the origin to another corner.
        { "cmd": "^GLH-", "call": doGLH },	// Change the bargraph upper limit.
        { "cmd": "^GLL-", "call": doGLL },	// Change the bargraph lower limit.
        { "cmd": "^GRD-", "call": doGRD },	// Change the bargraph ramp-down time in 1/10th of a second.
        { "cmd": "^GRU-", "call": doGRU },	// Change the bargraph ramp-up time in 1/10th of a second.
        { "cmd": "^GSC-", "call": doGSC },	// Change the bargraph slider color or joystick cursor color.
        { "cmd": "^GSN-", "call": unsupported },	// Change the bargraph slider name or joystick cursor name.
        { "cmd": "^ICO-", "call": doICO }, // Set the icon to a button.
        { "cmd": "^IRM-", "call": unsupported },	// Set the IR channel --> Not possible!
        { "cmd": "^JSB-", "call": doJSB },	// Set bitmap/picture alignmentusing a numeric keypad layout for those buttons witha defined address range.
        { "cmd": "^JSI-", "call": doJSI },	// Set icon alignment using a numeric keypad layout for those buttons with a defined address range.
        { "cmd": "^JST-", "call": doJST },	// Set text alignment using anumeric keypad layout for those buttons with a defined address range.
        { "cmd": "^MBT-", "call": unsupported },	// Set the Mouse Button mode On for the virtual PC.
        { "cmd": "^MDC-", "call": unsupported },	// Turn On the ’Mouse double-click’ feature for the virtual PC.
        { "cmd": "^SHO-", "call": doSHO },	// Show or hide a button with a set variable text range.
        { "cmd": "^TEC-", "call": doTEC },	// Set the text effect color for the specified addresses/states to the specified color.
        { "cmd": "^TEF-", "call": doTEF },	// Set the text effect.
        { "cmd": "^TOP-", "call": unsupported },	// Send events to the Master as string events.
        { "cmd": "^TXT-", "call": doTXT },	// Assign a text string to those buttons with a defined address range.
		{ "cmd": "^UNI-", "call": unsupported },	// Set Unicode text.
        { "cmd": "^LPC-", "call": unsupported },
        { "cmd": "^LPR-", "call": unsupported },
        { "cmd": "^LPS-", "call": unsupported },
        { "cmd": "?BCB-", "call": doGetBCB },	// Get the current border color.
        { "cmd": "?BCF-", "call": doGetBCF },	// Get the current fill color.
        { "cmd": "?BCT-", "call": doGetBCT },	// Get the current text color.
        { "cmd": "?BMP-", "call": doGetBMP },	// Get the current bitmap name.
        { "cmd": "?BOP-", "call": doGetBOP },	// Get the overall button opacity.
        { "cmd": "?BRD-", "call": doGetBRD },	// Get the current border name.
        { "cmd": "?BWW-", "call": doGetBWW },	// Get the current word wrap flag status.
        { "cmd": "?FON-", "call": doGetFON },	// Get the current font index.
        { "cmd": "?ICO-", "call": doGetICO },	// Get the current icon index.
        { "cmd": "?JSB-", "call": doGetJSB },	// Get the current bitmap justification.
        { "cmd": "?JSI-", "call": doGetJSI },	// Get the current icon justification.
        { "cmd": "?JST-", "call": doGetJST },	// Get the current text justification.
        { "cmd": "?TEC-", "call": doGetTEC },	// Get the current text effect color.
        { "cmd": "?TEF-", "call": doGetTEF },	// Get the current text effect name.
        { "cmd": "?TXT-", "call": doGetTXT },	// Get the current text information.
        { "cmd": "ABEEP", "call": doABEEP },
        { "cmd": "ADBEEP", "call": doADBEEP },
        { "cmd": "@AKB-", "call": doAKB },      // Pop up the keyboard icon and initialize the text string to that specified.
        { "cmd": "AKEYB-", "call": doAKEYB },   // Pop up the keyboard icon and initialize the text string to that specified.
        { "cmd": "AKEYP-", "call": doAKEYP },   // Pop up the keypad icon and initialize the text string to that specified.
        { "cmd": "AKEYR-", "call": unsupported },
        { "cmd": "@AKP-", "call": doAKP },      // Pop up the keypad icon and initialize the text string to that specified.
        { "cmd": "@AKR", "call": unsupported },
        { "cmd": "BEEP", "call": doABEEP },
        { "cmd": "BRIT-", "call": unsupported },
        { "cmd": "@BRT-", "call": unsupported },
        { "cmd": "DBEEP", "call": doADBEEP },
        { "cmd": "@EKP-", "call": doEKP },
        { "cmd": "PKEYP-", "call": doPKEYP },
        { "cmd": "@PKP-", "call": doPKP },
        { "cmd": "SETUP", "call": unsupported },
        { "cmd": "SHUTDOWN", "call": unsupported },
        { "cmd": "SLEEP", "call": unsupported },
        { "cmd": "@SOU-", "call": doSOU },	// Play a sound file
        { "cmd": "@TKP-", "call": unsupported },
        { "cmd": "TPAGEON", "call": unsupported },
        { "cmd": "TPAGEOFF", "call": unsupported },
        { "cmd": "@VKB", "call": unsupported },
        { "cmd": "WAKE", "call": unsupported },
        { "cmd": "^CAL", "call": unsupported },
        { "cmd": "^KPS-", "call": unsupported },
        { "cmd": "^VKS-", "call": unsupported },
        { "cmd": "@PWD-", "call": unsupported },
        { "cmd": "^PWD-", "call": unsupported },
        { "cmd": "^BBR-", "call": doBBR }, // Set the bitmap of a button to use a particular resource.
        { "cmd": "^RAF-", "call": doRAF }, // Add new resources.
        { "cmd": "^RFR-", "call": doRFR }, // Force a refresh for a given resource.
        { "cmd": "^RMF-", "call": doRMF }, // Modify an existing resource.
        { "cmd": "^RSR-", "call": doRSR }, // Change the refresh rate for a given resource.
        { "cmd": "^MODEL?", "call": unsupported },
        { "cmd": "^ICS-", "call": unsupported },
        { "cmd": "^ICE-", "call": unsupported },
        { "cmd": "^ICM-", "call": unsupported },
        { "cmd": "^PHN-", "call": unsupported },
        { "cmd": "?PHN-", "call": unsupported },
        { "cmd": "ON-", "call": setON },
        { "cmd": "OFF-", "call": setOFF },
        { "cmd": "LEVEL-", "call": setLEVEL },
        { "cmd": "#REG-", "call": doREG },
        { "cmd": "#ERR-", "call": doERR },
        { "cmd": "#PONG-", "call": unsupported }
    ]
};

const CENTER_CODE = Object.freeze({
    SC_ICON: 0,
    SC_BITMAP: 1,
    SC_TEXT: 2
});

function rgb(red, green, blue)
{
    return "rgb(" + red + "," + green + "," + blue + ")";
}

function rgba(red, green, blue, alpha)
{
    return "rgba(" + red + "," + green + "," + blue + "," + alpha + ")";
}

function amxInt(num)
{
    if (typeof num == "string")
    {
        if (num.charAt(0) == '#')
            return parseInt(num.substr(1), 16);
    }

    return parseInt(num);
}

function findFont(id)
{
    for (var i in fontList.fonts)
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
    var bef = "";
    var pos = msg.search('-');

    if (pos >= 0)
        bef = msg.substr(0, pos);
    else
        bef = msg;

    TRACE("Command " + bef + " is currently not supported!");
}

function newZIndex()
{
    z_index = z_index + 1;
    return z_index;
}

function freeZIndex()
{
    if (z_index > 0)
        z_index = z_index - 1;
}

function splittCmd(msg)
{
    var pID = 0;
    var pos = msg.indexOf('|');
    var dp = msg.indexOf(':');

    if (pos > 0 && dp > 0 && dp < pos)
    {
        pID = parseInt(msg.substr(0, dp));
        curPort = parseInt(msg.substr(dp + 1, pos));
        curCommand = msg.substr(pos + 1);
    }
    else
    {
        curPort = 0;
        curCommand = msg;
    }

    return pID;
}

function iterateButtonStates(addr, bts, callback, pars, port = true)
{
    var addrRange = getRange(addr);
    var btRange = getRange(bts);

	if (addrRange === null || addrRange.length == 0 || btRange === null || btRange.length == 0)
		return;

    for (var i in addrRange)
    {
		var bt = null;

		if (port)
			bt = findButtonPort(addrRange[i]);
		else
			bt = findButton(addrRange[i]);

        if (bt === null || bt.length == 0)
            continue;

        for (var b in bt)
        {
            for (var z = 1; z <= bt[b].instances; z++)
            {
                for (var j in btRange)
                {
                    if ((btRange.length == 1 && btRange[0] == 0) || btRange[j] == z)
                    {
                        var name = 'Page_' + bt[b].pnum + "_Button_" + bt[b].bi + "_" + z;
                        var button = getButton(bt[b].pnum, bt[b].bi);
                        var idx = 0;

						if (button !== null)
						{
							for (var x in button.sr)
							{
								if (button.sr[x].number == z)
									idx = parseInt(x);
							}

							callback(name, button, bt[b], idx, pars);
						}
						else
						{
							errlog('iterateButtons: Error button ' + name + ' seems not to exist!');
						}
                    }
                }
            }
        }
    }
}

function getButton(pnum, bi)
{
    var pgKey = eval("structPage" + pnum);

    if (pgKey === null)
        return null;

    for (var i in pgKey.buttons)
    {
        if (pgKey.buttons[i].bID == bi)
            return pgKey.buttons[i];
    }

    return null;
}

function getBargraphLevel(pnum, id)
{
    for (var i in bargraphs.bargraphs)
    {
        var bg = bargraphs.bargraphs[i];

        if (bg.pnum == pnum && bg.bi == id)
            return bg.level;
    }

    return 0;
}

function getBargraphPC(pnum, id)
{
    for (var i in bargraphs.bargraphs)
    {
        var bg = bargraphs.bargraphs[i];

        if (bg.pnum == pnum && bg.bi == id)
            return [bg.lp, bg.lv];
    }

    return -1;
}

function getBargraphPars(pnum, id)
{
    for (var i in bargraphs.bargraphs)
    {
        var bg = bargraphs.bargraphs[i];

        if (bg.pnum == pnum && bg.bi == id)
            return bg;
    }

    return -1;
}

function setBargraphLevel(pnum, id, level)
{
    var PC = getBargraphPC(pnum, id);

    if (PC === -1)
        return;

    for (var i in bargraphs.bargraphs)
    {
        var bg = bargraphs.bargraphs[i];

        if (bg.lp == PC[0] && bg.lv == PC[1])
            bargraphs.bargraphs[i].level = level;
        else if (bg.pnum == pnum && bg.bi == id)
            bargraphs.bargraphs[i].level = level;
    }
}

function getField(msg, field, sep)
{
    var flds = [];
    var rest = "";
    var bStr = false;
    var part = "";

    var pos = msg.indexOf('-'); // Check for the command part preceding the parameters

    if (pos >= 0)
        rest = msg.substr(pos + 1); // Cut off the command part
    else
        rest = msg; // No command part, so take the whole string

    for (var i = 0; i < rest.length; i++)
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
	if (typeof sr == "undefined" || sr.length == 0)
		return null;

    var narr = [];

    if (sr.indexOf('&') < 0 && sr.indexOf('.') < 0)
    {
        narr.push(parseInt(sr));
        return narr;
    }

    if (sr.indexOf('&') >= 0)
    {
        var sp1 = sr.split('&');

        for (var i = 0; i < sp1.length; i++)
        {
            if (sp1[i].indexOf('.') >= 0)
            {
                var sp2 = sp1[i].split('.');

                for (var a = parseInt(sp2[0]); a <= parseInt(sp2[1]); a++)
                    narr.push(a);
            }
            else
                narr.push(parseInt(sp1[i]));
        }
    }
    else
    {
        var sp1 = sr.split('.');

        for (var a = parseInt(sp1[0]); a <= parseInt(sp1[1]); a++)
            narr.push(a);
    }

    return narr;
}

function getRGBAColor(name)
{
    var colArr = [];

    for (var i in palette.colors)
    {
        var col = palette.colors[i];

        if (col.name == name)
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
    var colArr = [];

    for (var i in palette.colors)
    {
        var col = palette.colors[i];

        if (col.name == name)
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
	{
		var col = getRGBAColor(value);

		if (col === -1)
			return [0, 0, 0, 0];

		return col;
	}

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
	{
		var col = getRGBAColor(value);

		if (col === -1)
			return [0, 0, 0, 0];

		return col;
	}

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

		if (col === -1)
			return [0, 0, 0, 0];

		return rgba(col[0], col[1], col[2], col[3]);
	}

	var red = parseInt(value.substr(1, 2), 16);
	var green = parseInt(value.substr(3, 2), 16);
	var blue = parseInt(value.substr(5, 2), 16);

	if (value.length > 6)
	{
		var alpha = parseInt(value.substr(7, 2), 16);
		alpha = 1.0 / 256.0 * alpha;
		return rgba(red, green, blue, alpha);
	}

	return rgb(red, green, blue);
}

function findPopupNumber(name, pg = "")
{
	var p = ((pg.length > 0) ? pg : getActivePageName());

	for (var i in Popups.pages)
	{
		if (Popups.pages[i].name == name && Popups.pages[i].lnpage[p] !== null)
			return Popups.pages[i].ID;
	}

	return -1;
}

function findPageNumber(name)
{
	for (var i in Pages.pages)
	{
		if (Pages.pages[i].name == name)
			return Pages.pages[i].ID;
	}

	return -1;
}

function findPageName(num)
{
    for (var i in Pages.pages)
    {
        if (Pages.pages[i].ID == num)
            return Pages.pages[i].name;
    }

    return -1;
}

function findPopupName(num)
{
    for (var i in Popups.pages)
    {
        if (Popups.pages[i].ID == num)
            return Popups.pages[i].name;
    }

    return -1;
}

function findPageGroup(name)
{
    for (var i in Popups.pages)
    {
        if (Popups.pages[i].name == name)
            return Popups.pages[i].group;
    }

    return -1;
}

function findButton(num)
{
    var btArray = [];

    for (var i in buttonArray.buttons)
    {
        var bt = buttonArray.buttons[i];

        if (bt.cp == curPort && bt.ch == num)
            btArray.push(bt);
    }

    return btArray;
}

function findButtonPort(num)
{
    var btArray = [];

    for (var i in buttonArray.buttons)
    {
        var bt = buttonArray.buttons[i];

        if (bt.ap == curPort && bt.ac == num)
            btArray.push(bt);
    }

    return btArray;
}

function findButtonDistinct(pnum, bi)
{
    for (var i in buttonArray.buttons)
    {
        var bt = buttonArray.buttons[i];

        if (bt.pnum == pnum && bt.bi == bi)
            return buttonArray.buttons[i];
    }

    return null;
}

function findBargraphs(port, channel)
{
    var i;
    var bgArray = [];

    for (i in bargraphs.bargraphs)
    {
        var bg = bargraphs.bargraphs[i];

        if (bg.lp == port && bg.lv == channel)
            bgArray.push(bg);
    }

    return bgArray;
}

function getPopupIndex(name, pg = "")
{
	var p = ((pg.length > 0) ? pg : getActivePageName());

    for (var i in Popups.pages)
    {
        if (Popups.pages[i].name == name && Popups.pages[i].lnpage[p] !== null)
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

function getPopupStatus(name) // return true if popup is shown
{
    var pID;
    var pname;
    var stat;

    pID = findPopupNumber(name);

    if (pID > 0)
        pname = "Page_" + pID;

    try
    {
        stat = document.getElementById(pname).style.display;

        if (stat == "none")
            return false;
        else
            return true;
    }
    catch (e)
    {
        errlog('getPopupStatus: Error on name <' + name + '> and page ' + pname + ': ' + e);
        return false;
    }
}

function getPageStatus(name) // return true if popup is shown
{
    for (var i in basePages)
    {
        if (Pages.pages[i].name == name)
            return Pages.pages[i].active;
    }

    return false;
}

function getActivePage()
{
    for (var i in Pages.pages)
    {
        if (Pages.pages[i].active == true)
            return Pages.pages[i].ID;
    }

    return 0;
}

function getActivePageIndex()
{
    for (var i in Pages.pages)
    {
        if (Pages.pages[i].active == true)
            return i;
    }

    return -1;
}

function getActivePageName()
{
    for (var i in Pages.pages)
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

    return null;
}

function getIconDim(id)
{
    var i;

    for (i in iconArray.icons)
    {
        if (iconArray.icons[i].id == id)
            return [iconArray.icons[i].width, iconArray.icons[i].height];
    }

    return -1;
}

function getIconPosInfo(pnum, bi, inst)
{
    var i, name, pgKey;

    name = "structPage" + pnum;
    pgKey = eval(name);

    if (pgKey === null)
        return null;

    for (i in pgKey.buttons)
    {
        var j;
        var but = pgKey.buttons[i];

        if (but.bID == bi)
        {
            for (j in but.sr)
            {
                if (but.sr[j].number == inst)
                    return [but.sr[j].ji, but.sr[j].ix, but.sr[j].iy];
            }
        }
    }

    return null;
}

function setButtonOnline(pnum, id, stat)
{
    var i;

    for (i in buttonArray.buttons)
    {
        if (buttonArray.buttons[i].pnum == pnum && buttonArray.buttons[i].bi == id)
        {
            buttonArray.buttons[i].ion = stat;
            break;
        }
    }
}

function isPopupOnPage(page, popup)
{
	for (var i in Popups.pages)
	{
		if (Popups.pages[i].name == popup)
		{
			var lnpg = Popups.pages[i].lnpage;

			for (var j in lnpg)
			{
				if (lnpg[j] == page)
					return true;
			}

			return false;
		}
	}

	return false;
}
function addPopupToPage(page, popup)
{
	for (var i in Popups.pages)
	{
		if (Popups.pages[i].name == popup)
		{
			if (!isPopupOnPage(page, popup))
			{
				Popups.pages[i].lnpage.push(page);
				return;
			}

			return;
		}
	}
}
function removePopupFromPage(page, popup)
{
	for (var i in Popups.pages)
	{
		if (Popups.pages[i].name == popup)
		{
			for (var j in Popups.pages[i].lnpage)
			{
				if (Popups.pages[i].lnpage[j] == page)
				{
					Popups.pages[i].lnpage.splice(j, 1);
					return;
				}
			}

			return;
		}
	}
}

function hideGroup(name, page = "")
{
    var group = popupGroups[name];

    if (group === null || typeof group == "undefined" || name === null || typeof name == "undefined" || name.length == 0)
    {
        errlog("hideGroup: Invalid group name '" + name + "'!");
        return;
    }

    for (var i in group)
    {
		var pg = findPopupNumber(group[i], page);

		if (pg == -1)
			continue;

        var nm = 'Page_' + pg;

        try
        {
            var idx = getPopupIndex(group[i], page);

            if (idx >= 0)
            {
                if (Popups.pages[idx].active == true)
                {
                    dropPopup(Popups.pages[idx].name);
                    freeZIndex();
                }

				Popups.pages[idx].active = false;
				var page = ((page.length > 0) ? page : getActivePageName());
				removePopupFromPage(page, Popups.pages[idx].name);
			}
        }
        catch (e)
        {
            errlog('hideGroup: Error on name <' + name + '> and page ' + nm + ': ' + e);
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
    pname = "Page_" + pID;
    hideGroup(group);

    try
    {
        drawPopup(name);
        document.getElementById(pname).style.zIndex = newZIndex();
        idx = getPopupIndex(name);

        if (idx >= 0)
        {
			Popups.pages[idx].active = true;
			addPopupToPage(getActivePageName(), Popups.pages[idx].name);
        }
    }
    catch (e)
    {
        errlog('showPopup: Error on name <' + name + '> and page ' + pname + ': ' + e);
    }
}

function showPopupOnPage(name, pg)
{
    var pID = findPopupNumber(name, pg);
    var group = findPageGroup(name);
    var pname = "Page_" + pID;
    hideGroup(group, pg);

    try
    {
        var idx = getPopupIndex(name);

        if (idx >= 0)
        {

			var actPage = getActivePageName();
			var page = ((pg.length > 0) ? pg : actPage);
			addPopupToPage(page, Popups.pages[idx].name);

			if (!Popups.pages[idx].active)
            {
				Popups.pages[idx].active = true;

				if (actPage == page)
				{
                	drawPopup(name);
					document.getElementById(pname).style.zIndex = newZIndex();
				}
            }
        }
    }
    catch (e)
    {
        errlog('showPopupOnPage: Error on name <' + name + '> and page ' + pname + ': ' + e);
    }
}

function hidePopupOnPage(name, pg)
{
    var pID = findPopupNumber(name, pg);
//    var group = findPageGroup(name);
    var pname = "Page_" + pID;
//    hideGroup(group);

    try
    {
		var actPage = getActivePageName();
		var page = ((pg.length > 0) ? pg : actPage);
        var idx = getPopupIndex(name);

        if (idx >= 0)
        {
			removePopupFromPage(page, Popups.pages[idx].name);
			var active = Popups.pages[idx].active;
			Popups.pages[idx].active = false;

			if (active && actPage == page)
            {
				dropPopup(name);
                freeZIndex();
            }

        }
    }
    catch (e)
    {
        errlog('hidePopupOnPage: Error on name <' + name + '> and page ' + pname + ': ' + e);
    }
}

function hidePopup(name)
{
    var pID = findPopupNumber(name);
    var pname = "Page_" + pID;

    try
    {
        var idx = getPopupIndex(name);

        if (idx >= 0)
        {
			var active = Popups.pages[idx].active;
			Popups.pages[idx].active = false;
			removePopupFromPage(getActivePageName(), Popups.pages[idx].name);

			if (active)
            {
                dropPopup(name);
                freeZIndex();
            }

        }
    }
    catch (e)
    {
        errlog('hidePopup: Error on name <' + name + '> and page ' + pname + ': ' + e);
    }
}

function showPage(name)
{
    var pname = "";
    var pID = findPageNumber(name);

    if (pID > 0)
        pname = "Page_" + pID;
    else
        pname = name;

    try
    {
        var idx = getActivePageIndex();
		dropPage();

		if (idx >= 0)
			Pages.pages[idx].active = false;

		var idx = getPageIndex(name);

		if (idx == -1)
			return;

		Pages.pages[idx].active = true;
		drawPage(name);
		var page = getActivePageName();

        for (var i in Popups.pages)
        {
	        pname = "Page_" + Popups.pages[i].ID;

			if (Popups.pages[i].modality)
	            pname = pname+"_modal";

            if (Popups.pages[i].active && !isPopupOnPage(page, name))
            {
                freeZIndex();
            }
            else if (Popups.pages[i].active)
            {
				if (document.getElementById(pname) === null)
				{
					addPopupToPage(page, Popups.pages[i].name);
					drawPopup(name);
				}

                document.getElementById(pname).style.zIndex = newZIndex();
            }
        }
    }
    catch (e)
    {
        errlog('showPage: Error on name <' + name + '> and page ' + pname + ': ' + e);
    }
}

function hidePage(name)
{
	var idx = getPageIndex(name);

	if (idx >= 0)
	{
		if (!Pages.pages[idx].active)
			return;

		Pages.pages[idx].active = false;
	}
	else
		return;

    try
    {
        dropPage();

        for (var i in Popups.pages)
		{
			if (Popups.pages[i].active)
			{
				if (!isPopupOnPage(name, Popups.pages[idx].name))
					freeZIndex();

				Popups.pages[i].active = false;
            }
        }
    }
    catch (e)
    {
        errlog('hidePage: Error on page ' + name + ': ' + e);
    }
}

function switchDisplay(name1, name2, dStat, cport, cnum)
{
    var btKenn = getButtonKennung(name1);

    if (btKenn === null)
        return;

    var bt = findButtonDistinct(btKenn[0], btKenn[1]);

    if (bt === null)
        return;

    if (bt.enabled == 0)
        return;

    try
    {
        if (dStat == 1)
        {
            document.getElementById(name1).style.display = "none";
            document.getElementById(name2).style.display = "inline";
            writeTextOut("PUSH:" + panelID + ":" + cport + ":" + cnum + ":1");
        }
        else
        {
            document.getElementById(name1).style.display = "inline";
            document.getElementById(name2).style.display = "none";
            writeTextOut("PUSH:" + panelID + ":" + cport + ":" + cnum + ":0");
        }
    }
    catch (e)
    {
        errlog('switchDisplay: Error: name1='+name1+', name2='+name2+', dStat='+dStat+',panelID='+panelID+', cport='+cport+', cnum='+cnum);
    }
}

function pushButton(cport, cnum, stat)
{
    writeTextOut("PUSH:" + panelID + ":" + cport + ":" + cnum + ":" + stat);
}

function sendString(cport, cnum, txt)
{
	writeTextOut("STRING:" + panelID + ":" + cport + ":" + cnum + ":" + txt);
}

function textToWeb(txt)
{
	if (typeof txt != "string" || txt.length == 0)
		return txt;

    var nt = txt.replace(/&/g, "&amp;");
    nt = nt.replace(/>/g, "&gt;");
    nt = nt.replace(/</g, "&lt;");
    // Spaces
    nt = nt.replace(/ /g, "&nbsp;");
    // Line break
    nt = nt.replace(/\n/g, "<br>");
    nt = nt.replace(/\r/g, "");
    // Special characters
    nt = nt.replace(/\u00f6/g, "&ouml;");
    nt = nt.replace(/\u00e4/g, "&auml;");
    nt = nt.replace(/\u00fc/g, "&uuml;");
    nt = nt.replace(/\u00df/g, "&szlig;");
    nt = nt.replace(/\u00d6/g, "&Ouml;");
    nt = nt.replace(/\u00c4/g, "&Auml;");
    nt = nt.replace(/\u00dc/g, "&Uuml;");
    return nt;
}

function setON(msg)
{
    var b;

    var addr = getField(msg, 0, ',');
    var bt = findButton(addr);

    if (bt.length == 0)
    {
        errlog('setON: Error button ' + addr + ' not found!');
        return;
    }

    for (b = 0; b < bt.length; b++)
    {
        if (bt[b].enabled == 0)
            continue;

        try
        {
            var name1 = 'Page_' + bt[b].pnum + '_Button_' + bt[b].bi + '_1';
            var name2 = 'Page_' + bt[b].pnum + '_Button_' + bt[b].bi + '_2';

            setButtonOnline(bt[b].pnum, bt[b].bi, 2);
            document.getElementById(name1).style.display = 'none';
            document.getElementById(name2).style.display = 'inline';
			writeTextOut("ON:"+panelID+':' + bt[b].cp + ":" + bt[b].ch);
			var button = findButtonDistinct(bt[b].pnum, bt[b].bi);

			if (button.sr[1].sd.length > 0)
			{
				var snd = new Audio("sounds/"+button.sr[1].sd);
				snd.play();
			}
        }
        catch (e)
        {
            errlog("setON: [Page_" + bt[b].pnum + "_Button_" + bt[b].bi + "_?] Error: " + e);
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
        errlog('setOFF: Error button ' + addr + ' not found!');
        return;
    }

    for (b = 0; b < bt.length; b++)
    {
        if (bt[b].enabled == 0)
            continue;

        try
        {
            var name1 = 'Page_' + bt[b].pnum + '_Button_' + bt[b].bi + '_1';
            var name2 = 'Page_' + bt[b].pnum + '_Button_' + bt[b].bi + '_2';

            setButtonOnline(bt[b].pnum, bt[b].bi, 1);
            document.getElementById(name1).style.display = 'inline';
            document.getElementById(name2).style.display = 'none';
            writeTextOut("OFF:"+panelID+':' + bt[b].cp + ":" + bt[b].ch);
			var button = findButtonDistinct(bt[b].pnum, bt[b].bi);

			if (button.sr[0].sd.length > 0)
			{
				var snd = new Audio("sounds/"+button.sr[0].sd);
				snd.play();
			}
        }
        catch (e)
        {
            errlog("setOFF: [Page_" + bt[b].pnum + "_Button_" + bt[b].bi + "_?] Error: " + e);
        }
    }

}

function setLEVEL(msg)
{
    var i, j;

    var addr = getField(msg, 0, ',');
    var level = getField(msg, 1, ',');
    var bgArray = findBargraphs(curPort, addr);

    for (i in bgArray)
    {
        var value = parseInt(100.0 / (bgArray[i].rh - bgArray[i].rl) * level);
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
            var button = getButton(bgArray[i].pnum, bgArray[i].bi);

            if (button !== null)
            {
                width = button.wt;
                height = button.ht;
            }
            else
            {
                width = 0;
                height = 0;
            }
        }

        if (bgArray[i].dr == "horizontal")
            dir = false;
        else
            dir = true;

        if (bgArray[i].states[0].mi.length > 0 && bgArray[i].states[1].bm.length > 0)
        {
            setBargraphLevel(bgArray[i].pnum, bgArray[i].bi, level);

            try
            {
                drawBargraph(makeURL("images/" + bgArray[i].states[0].mi), makeURL("images/" + bgArray[i].states[1].bm), name, value, width, height, getAMXColor(bgArray[i].states[1].cf), getAMXColor(bgArray[i].states[1].cb), dir);
            }
            catch (e)
            {
                errlog("setLEVEL: Error: " + e);
            }
        }
        else if (bgArray[i].states[0].mi.length == 0 && bgArray[i].states[0].bm.length == 0 && // No graphics at all
            bgArray[i].states[1].mi.length == 0 && bgArray[i].states[1].bm.length == 0)
        {
            try
            {
                drawBargraphLight(name, value, width, height, getWebColor(bgArray[i].states[1].cf), getWebColor(bgArray[i].states[0].cf), dir);
            }
            catch (e)
            {
                errlog("setLEVEL: Error: " + e);
            }
        }
        else if (bgArray[i].states[0].mi.length == 0 && bgArray[i].states[0].bm.length > 0 &&
            bgArray[i].states[1].mi.length == 0 && bgArray[i].states[1].bm.length > 0)
        {
            try
            {
                drawBargraph2Graph(makeURL("images/" + bgArray[i].states[1].bm), makeURL("images/" + bgArray[i].states[0].bm), name, value, width, height, dir);
            }
            catch (e)
            {
                errlog("setLEVEL: Error: " + e);
            }
        }
    }
}
/*
 * Flips to a page with the specified page name using an animated transition.
 *
 * Attention: The animation is currently not implemented. This works
 * the same way like doPPN()!
 */
function doAFP(msg)
{
    var pg = getField(msg, 0, ',');
//    var animation = getField(msg, 1, ',');
//    var origin = getField(msg, 2, ',');
//    var time = getField(msg, 3, ',');

    showPopup(pg);
}
/*
 * Pop up the keyboard icon and initialize the text string to that
 * specified.
 */
function doAKB(msg)
{
    var promptText = getField(msg, 0, ';');
    var initText = getField(msg, 1, ';');

    if (promptText === null || promptText.length == 0)
        promptText = getAkbText();

    var text = doKeyboard(initText, promptText);

    if (text === null)
        writeTextOut('KEY:'+panelID+':0:0:KEYB-ABORT');
    else
    {
        setAkbText(text);
        sendKeyboardText(text);
    }
}
/*
 * Pop up the keyboard icon and initialize the text string to that
 * specified.
 */
function doAKEYB(msg)
{
    var initText = getField(msg, 0, ',');
    var promptText = getAkbText();
    var text = doKeyboard(initText, promptText);

    if (text === null)
        writeTextOut('KEY:'+panelID+':0:0:KEYB-ABORT');
    else
    {
        setAkbText(text);
        sendKeyboardText(text);
    }
}
/*
 * Pop up the keypad icon and initialize the text string to that
 * specified.
 */
function doAKP(msg)
{
    var promptText = getField(msg, 0, ';');
    var initText = getField(msg, 1, ';');

    if (promptText === null || promptText.length == 0)
        promptText = getAkpText();

    var text = doKeyboard(initText, promptText);

    if (text === null)
        writeTextOut('KEY:'+panelID+':0:0:KEYP-ABORT');
    else
    {
        setAkpText(text);
        sendKeypadText(text);
    }
}
/*
 * Pop up the keypad icon and initialize the text string to that
 * specified.
 */
function doAKEYP(msg)
{
    var initText = getField(msg, 0, ',');
    var promptText = getAkpText();
    var text = doKeyboard(initText, promptText);

    if (text === null)
        writeTextOut('KEY:'+panelID+':0:0:KEYP-ABORT');
    else
    {
        setAkpText(text);
        sendKeypadText(text);
    }
}
/*
 * Add a specific popup page to a specified popup group.
 */
function doAPG(msg)
{
    var pg = getField(msg, 0, ";");
    var name = getField(msg, 1, ";");

    if (pg.length > 0 && group.length > 0)
    {
        for (var i in popupGroups)
        {
            if (popupGroups[i] == name)
            {
                var have = false;
                var group = popupGroups[i];

                for (var j in group)
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
/*
 * Pop up the keyboard icon and initialize the text string to that
 * specified.
 */
function doEKP(msg)
{
    var promptText = getField(msg, 0, ';');
    var initText = getField(msg, 1, ';');

    if (promptText === null || promptText.length == 0)
        promptText = getAkpText();

    var text = doKeyboard(initText, promptText);

    if (text === null)
        writeTextOut('KEY:'+panelID+':0:0:KEYP-ABORT');
    else
    {
        setAkpText(text);
        sendKeypadText(text);
    }
}
/*
 * Present a private keypad.
 */
async function doPKEYP(msg)
{
    var initText = getField(msg, 0, ',');

    try
    {
        var text = await passwordPrompt(initText, "");
        sendKeyboardText(text);
    }
    catch(e)
    {
        writeTextOut('KEY:'+panelID+':0:0:KEYB-ABORT');
    }
}
/*
 * Present a private keypad.
 */
async function doPKP(msg)
{
    var promptText = getField(msg, 0, ';');
    var initText = getField(msg, 1, ';');

    try
    {
        var text = await passwordPrompt(initText, promptText);
        sendKeyboardText(text);
    }
    catch(e)
    {
        writeTextOut('KEY:'+panelID+':0:0:KEYB-ABORT');
    }
}
/*
 * Activate a specific popup page to launch on either a specified
 * page or the current page.
 */
function doPPN(msg)
{
    var pos;
    var pg;
    var name;

    pg = "";
    pos = msg.indexOf(";"); // Do we have a page name?
    // FIXME: Page names are not supported currently!
    if (pos < 0)
        pos = msg.length;
    else
    {
        pg = msg.substr(pos + 1);
        pos = msg.length - pos - 5;
    }

    name = msg.substr(5, pos); // Extract the popup name

    if (pg.length > 0)
        showPopupOnPage(name, pg);
    else
        showPopup(name);
}
/*
 * Deactivate a specific popup page on either a specified page or
 * the current page.
 */
function doPPF(msg)
{
    var pg = "";
    var pos = msg.indexOf(";"); // Do we have a page name?
    // FIXME: Page names are not supported currently!
    if (pos < 0)
        pos = msg.length - 5;
    else {
        pg = msg.substr(pos + 1);
        pos = msg.length - pos - 5;
    }

    var name = msg.substr(5, pos); // Extract the popup name

    if (pg.length > 0)
        hidePopupOnPage(name, pg);
    else
        hidePopup(name);
}
/*
 * Toggle a specific popup page on either a specified page or
 * the current page.
 */
function doPPG(msg)
{
    var pos = msg.indexOf(";"); // Do we have a page name?
    // FIXME: Page names are not supported currently!
    if (pos < 0)
        pos = msg.length;

    var name = msg.substr(5, pos); // Extract the popup name

    if (getPopupStatus(name)) // Is popup visible?
        hidePopup(name);
    else
        showPopup(name);
}
/*
 * Kill a specific popup page from all pages.
 */
function doPPK(msg)
{
    var name = msg.substr(5); // Extract the popup name
    var group = findPageGroup(name);

    if (group.length > 0)
        hideGroup(group);
    else
        hidePopup(name);
}
/*
 * Set the modality of a specific popup page to Modal or NonModal.
 */
function doPPM(msg)
{
	var pname = getField(msg, 0, ';');
	var mode = getField(msg, 1, ';');

	for (var i in Popups.pages)
	{
		var pop = Popups.pages[i];

		if (pop.name == pname)
		{
			if (mode == 0)
				pop.modality = false;
			else
				pop.modality = true;

			break;
		}
	}
}
/*
 * Set a specific popup page to timeout within a specified time.
 */
function doPPT(msg)
{
	var pname = getField(msg, 0, ';');
	var tm = getField(msg, 1, ';');

	var pnum = findPopupNumber(pname);

	if (pnum == -1)
		return;

	var name = "Page_"+pnum;

	try
	{
		document.getElementById(name);		// Make sure, the popup is visible
		window.setTimeout(tmPPT.bind(null, pname), tm * 100);
	}
	catch (e)
	{
		errlog("doPPT: Error: "+e);
	}
}
function tmPPT(name)
{
	hidePopup(name);
}
/*
 * Close all popups on all pages.
 */
function doPPX(msg)
{
    for (var i in Popups.pages)
    {
        hidePopup(Popups.pages[i].name);
    }
}
/*
 * Set the show effect for the specified popup page to the named show effect.
 */
function doPSE(msg)
{
	var pname = getField(msg, 0, ';');
	var hname = getField(msg, 1, ';');

	var pnum = findPopupNumber(pname);

	if (pnum == -1)
		return;

	var pgKey = getPage(pnum);

	if (hname.toLowerCase() == "fade")
		pgKey.showEffect = 1;
	else if (hname.toLowerCase() == "slide from left")
		pgKey.showEffect = 2;
	else if (hname.toLowerCase() == "slide from right")
		pgKey.showEffect = 3;
	else if (hname.toLowerCase() == "slide from top")
		pgKey.showEffect = 4;
	else if (hname.toLowerCase() == "slide from bottom")
		pgKey.showEffect = 5;
	else if (hname.toLowerCase() == "slide from left fade")
		pgKey.showEffect = 6;
	else if (hname.toLowerCase() == "slide from right fade")
		pgKey.showEffect = 7;
	else if (hname.toLowerCase() == "slide from top fade")
		pgKey.showEffect = 8;
	else if (hname.toLowerCase() == "slide from bottom fade")
		pgKey.showEffect = 9;
}
/*
 * Set the show effect time for the specified popup page.
 */
function doPST(msg)
{
	var pname = getField(msg, 0, ';');
	var htime = getField(msg, 1, ';');

	var pnum = findPopupNumber(pname);

	if (pnum == -1)
		return;

	var pgKey = getPage(pnum);
	pgKey.showTime = htime;
}
/*
 * Flip to a specified page.
 */
function doPAGE(msg)
{
    var name = msg.substr(5);
    showPage(name);
}
/*
 * Clear all popup pages from specified popup group.
 */
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
/*
 * Run a button animation (in 1/10 second).
 */
function doANI(msg)
{
    var addr = getField(msg, 0, ',');
    var stStart = getField(msg, 1, ',');
    var stEnd = getField(msg, 2, ',');
    var zeit = getField(msg, 3, ',');

    var addrRange = getRange(addr);

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButton(addrRange[i]);

        if (bt.length == 0)
        {
            errlog("doANI: Error button "+ addrRange[i] + " not found!");
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
            var name = 'Page_' + bt[b].pnum + '_Button_' + bt[b].bi + '_';
            var button = getButton(bt[b].pnum, bt[b].bi);
            drawButtonMultistateAni(button, name, 1, stStart, stEnd, zeit);
        }
    }
}
/*
 * Add page flip action to a button if it does not already exist.
 */
function doAPF(msg)
{
    var name;
    var cmd;
    var addr;
    var addrRange;

    addr = getField(msg, 0, ',');
    addrRange = getRange(addr);
    cmd = getField(msg, 1, ',');
    name = getField(msg, 2, ',');

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButton(addrRange[i]);

        if (bt.length == 0)
        {
            errlog('doAPF: Error button ' + addrRange[i] + ' not found!');
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
            var nm = 'Page_' + bt[b].pnum + '_Button_' + bt[b].bi + '_';
            var num = bt[b].instances;

            if (cmd.search('Show') >= 0)
            {
                for (var j = 0; j < num; j++)
                {
                    var idx = j + 1;
                    document.getElementById(nm+idx).addEventListener(EVENT_UP, showPopup.bind(null, name), false);
                }
            }
            else if (cmd.search('Hide') >= 0)
            {
                for (var j = 0; j < num; j++)
                {
                    var idx = j + 1;
                    document.getElementById(nm+idx).addEventListener(EVENT_UP, hidePopup.bind(null, name), false);
                }
            }
            // FIXME: There are more commands!
        }
    }
}

/*
 * Close all popups on a specified page.
 */
function doPPA(msg)
{
	var pname = getField(msg, 0, ',');

	for (var i in Popup.pages)
	{
		var pop = Popup.pages[i];

		if (pop.active == true && isPopupOnPage(pname, pop.name))
			hidePopup(pop.name);
	}
}
/*
 * Delete a specific popup page from specified popup group if it exists.
 */
function doDPG(msg)
{
    var i, j, pos;
    var pg, name;

    pg = getField(msg, 0, ';');
    name = getField(msg, 1, ';');

    if (pg.length > 0 && group.length > 0)
    {
        for (i in popupGroups)
        {
            if (popupGroups[i] == name)
            {
                var have = false;
                var group = popupGroups[i];
                pos = 0;

                for (j in group)
                {
                    if (group[j] == pg)
                    {
                        have = true;
                        pos = parseInt(j);
                        break;
                    }
                }

                if (have)
                    popupGroups[i].splice(pos, 1);
            }
        }
    }

}
/*
 * Set the hide effect for the specified popup page to the named hide effect.
 */
function doPHE(msg)
{
	var pname = getField(msg, 0, ';');
	var hname = getField(msg, 1, ';');

	var pnum = findPopupNumber(pname);

	if (pnum == -1)
		return;

	var pgKey = getPage(pnum);

	if (hname.toLowerCase() == "fade")
		pgKey.hideEffect = 1;
	else if (hname.toLowerCase() == "slide to left")
		pgKey.hideEffect = 2;
	else if (hname.toLowerCase() == "slide to right")
		pgKey.hideEffect = 3;
	else if (hname.toLowerCase() == "slide to top")
		pgKey.hideEffect = 4;
	else if (hname.toLowerCase() == "slide to bottom")
		pgKey.hideEffect = 5;
	else if (hname.toLowerCase() == "slide to left fade")
		pgKey.hideEffect = 6;
	else if (hname.toLowerCase() == "slide to right fade")
		pgKey.hideEffect = 7;
	else if (hname.toLowerCase() == "slide to top fade")
		pgKey.hideEffect = 8;
	else if (hname.toLowerCase() == "slide to bottom fade")
		pgKey.hideEffect = 9;
}
/*
 * Set the hide effect time for the specified popup page.
 */
function doPHT(msg)
{
	var pname = getField(msg, 0, ';');
	var htime = getField(msg, 1, ';');

	var pnum = findPopupNumber(pname);

	if (pnum == -1)
		return;

	var pgKey = getPage(pnum);
	pgKey.hideTime = htime;
}
/*
 * Append non-unicodetext.
 */
function doBAT(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var text = getField(msg, 2, ',');

    text = textToWeb(text);
    iterateButtonStates(addr, bts, cbBAT, text);
}
function cbBAT(name, button, bt, idx, text)
{
    button.sr[idx].te = button.sr[idx].te + text;

    try
    {
        document.getElementById(name).innerHTML = button.sr[idx].te;
    }
    catch (e)
    {
        errlog("cbBAT: No element of name " + name + " found!");
    }
}
/*
 * Append unicode text.
 */
function doBAU(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var text = getField(msg, 2, ',');

    var utext = "";

    for (var i = 0; i < text.length; i += 4)
        utext = utext + "\\u" + text.substring(i, i + 4);

    iterateButtonStates(addr, bts, cbBAU, unescape(encodeURIComponent(utext)));
}
function cbBAU(name, button, bt, idx, text)
{
    text = textToWeb(text);
    button.sr[idx].te = button.sr[idx].te + text;

    try
    {
        document.getElementById(name).innerHTML = button.sr[idx].te;
    }
    catch (e)
    {
        errlog("cbBAU: No element of name " + name + " found!");
    }
}
/*
 * Set the bitmap of a button to use a particular ressource.
 */
function doBBR(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var name = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbBBR, name);
    refreshResource(name);
}
function cbBBR(name, button, bt, idx, nm)
{
    button.sr[idx].sb = 1;
    button.sr[idx].bm = nm;
}
/*
 * Set the border color to the specified color.
 */
function doBCB(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var col = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbBCB, col);
}
function cbBCB(name, button, bt, idx, col)
{
    button.sr[idx].cb = col;

    try
    {
        var colArr;

        if ((colArr = getHexColor(col)) === -1)
        {
            errlog("cbBCB: Error getting color for " + name);
            return;
        }

        var color;

        if (colArr.length > 3)
            color = rgba(colArr[0], colArr[1], colArr[2], colArr[3]);
        else
            color = rgb(colArr[0], colArr[1], colArr[2]);

        document.getElementById(name).style.borderColor = color;
    }
    catch (e)
    {
        errlog("cbBCB: No element of name " + name + " found!");
    }
}
/*
 * Set the fill color to the specified color.
 */
function doBCF(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var col = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbBCF, col);
}
function cbBCF(name, button, bt, idx, col)
{
    button.sr[idx].cf = col;

    try
    {
        var colArr;

        if ((colArr = getHexColor(col)) === -1)
        {
            errlog("cbBCT: Error getting color for " + name);
            return;
        }

        var color;

        if (colArr.length > 3)
            color = rgba(colArr[0], colArr[1], colArr[2], colArr[3]);
        else
            color = rgb(colArr[0], colArr[1], colArr[2]);

        document.getElementById(name).style.backgroundColor = color;
    }
    catch (e)
    {
        errlog("cbBCF: No element of name " + name + " found!");
    }
}
/*
 * Set the text color to the specified color.
 */
function doBCT(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var col = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbBCT, col);
}
function cbBCT(name, button, bt, idx, col)
{
    button.sr[idx].ct = col;

    try
    {
        var colArr;

        if ((colArr = getHexColor(col)) === -1)
        {
            errlog("cbBCT: Error getting color for " + name);
            return;
        }

        var color;

        if (colArr.length > 3)
            color = rgba(colArr[0], colArr[1], colArr[2], colArr[3]);
        else
            color = rgb(colArr[0], colArr[1], colArr[2]);

        document.getElementById(name).style.color = color;
    }
    catch (e)
    {
        errlog("cbBCT: No element of name " + name + " found! Error: " + e);
    }
}
/*
 * Button copy command. Copy attributes of the source button
 * to all the destination buttons.
 */
function doBMC(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');
	var src_port = getField(msg, 2, ',');
	var src_addr = getField(msg, 3, ',');
	var src_state = getField(msg, 4, ',');
	var codes = getField(msg, 5, ',');

    var src_button = null;		// will contain the source button
	// Find the source button
    for (var i in buttonArray.buttons)
    {
        var abut = buttonArray.buttons[i];

		if (abut.ap == src_port && abut.ac == src_addr && src_state <= abut.instances)
		{
			var but = getPage(abut.pnum);

			for (var j in but.button)
			{
				if (but.buttons[j].bID == abut.bi)
				{
					for (var x in but.buttons[j].sr)
					{
						if (but.buttons[j].sr[x].number == src_state)
						{
							src_button = but.buttons[j].sr[x];
							break;
						}
					}

					break;
				}
			}

			break;
		}
	}

	if (src_button === null)	// Not found?
	{
		errlog("cbBMC: Button "+src_port+","+src_addr+","+src_state+" not found!");
		return;
	}
	// Here we have our source button.
	// Create an array with the parameter codes.
	var pos = 0;
	var cd = "";
	var cdArr = [];

	for (var i = 0; i < codes.length; i++)
	{
		var c = codes.charAt(i);

		if (c >= 'A' && c <= 'Z')
		{
			cd = cd + c;
			pos++;

			if (pos > 1)
			{
				cdArr.push(cd);
				cd = c;
				pos = 1;
			}
		}
	}

	var pars = [src_button, cdArr];
	iterateButtonStates(addr, bts, cbBMC, pars, true);
}
function cbBMC(name, button, bt, idx, pars)
{
	var sr = pars[0];
	var cdArr = pars[1];

	// We'll start to copy the parameters.
	for (var i in cdArr)
	{
		if (cdArr[i] == "BM")		// Picture/Bitmap
		{
			button.sr[idx].bm = sr.bm;
			button.sr[idx].bm_width = sr.bm_width;
			button.sr[idx].bm_height = sr.bm_height;
			button.sr[idx].mi = sr.bm;
			button.sr[idx].mi_width = sr.bm_width;
			button.sr[idx].mi_height = sr.bm_height;
		}
		else if (cdArr[i] == "BR")	// Border
			button.sr[idx].bs = sr.bs;
		else if (cdArr[i] == "CB")	// Border color
			button.sr[idx].cb = sr.cb;
		else if (cdArr[i] == "CF")	// Fill color
			button.sr[idx].cf = sr.cf;
		else if (cdArr[i] == "CT")	// Text color
			button.sr[idx].ct = sr.ct;
		else if (cdArr[i] == "EC")	// Text effect color
			button.sr[idx].ec = sr.ec;
		else if (cdArr[i] == "EF")	// Text effect
			button.sr[idx].et = sr.et;
		else if (cdArr[i] == "FT")	// Font
			button.sr[idx].fi = sr.fi;
		else if (cdArr[i] == "IC")	// Icon
			button.sr[idx].ii = sr.ii;
		else if (cdArr[i] == "JB")	// Bitmap alignment
			button.sr[idx].jb = sr.jb;
		else if (cdArr[i] == "JI")	// Icon alignment
			button.sr[idx].jb = sr.jb;
		else if (cdArr[i] == "JT")	// Text alignment
			button.sr[idx].jt = sr.jt;
		else if (cdArr[i] == "OP")	// Opacity
			button.sr[idx].oo = sr.oo;
//		else if (cdArr[i] == "SO")	// Button sound
//			button.sr[idx].so = sr.so
		else if (cdArr[i] == "TX")	// Text
			button.sr[idx].te = sr.te;
		else if (cdArr[i] == "WW")	// Word wrap
			button.sr[idx].ww = sr.ww;
	}
}
/*
 * Set the feedbacktype of the button.
 */
function doBFB(msg)
{
    var addr = getField(msg, 0, ',');
    var fb = getField(msg, 1, ',');

    var addrRange = getRange(addr);

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButtonPort(addrRange[i]);

        if (bt.length == 0)
        {
            errlog('doBFB: Error button ' + addrRange[i] + ' not found!');
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
            var pgKey = eval("structPage"+bt[b].pnum);

            for (var z = 1; z <= bt[b].instances; z++)
            {
                for (var x = 0; x < pgKey.buttons.length; x++)
                {
                    var button = pgKey.buttons[x];

                    if (button.bID == bt[b].bi)
                    {
                        if (fb.toLowerCase() == "none")
                            button.fb = FEEDBACK.FB_NONE;
                        else if (fb.toLowerCase() == "channel")
                            button.fb = FEEDBACK.FB_CHANNEL;
                        else if (fb.toLowerCase() == "invert")
                            button.fb = FEEDBACK.FB_INV_CHANNEL;
                        else if (fb.toLowerCase() == "on")
                            button.fb = FEEDBACK.FB_ALWAYS_ON;
                        else if (fb.toLowerCase() == "momentary")
                            button.fb = FEEDBACK.FB_MOMENTARY;
                        else if (fb.toLowerCase() == "blink")
                            button.fb = FEEDBACK.FB_BLINK;

                        break;
                    }
                }
            }
        }
    }
}
/*
 * Set the input mask for the specified address.
 */
function doBIM(msg)
{
	var addr = getField(msg, 0, ',');
	var imask = getField(msg, 1, ',');

	var addrRange = getRange(addr);

	for (var i = 0; i < addrRange.length; i++)
	{
		var bt = findButtonPort(addrRange[i]);

		if (bt.length == 0)
		{
			errlog("doBIM: Error: Button "+addrRange[i]+" not found!");
			continue;
		}

		for (var b = 0; b < bt.length; b++)
		{
			var pgKey = getPage(bt[b].pnum);

            for (var z = 1; z <= bt[b].instances; z++)
            {
                for (var x = 0; x < pgKey.buttons.length; x++)
                {
                    var button = pgKey.buttons[x];

                    if (button.bID == bt[b].bi && button.btype == BUTTONTYPE.TEXT_INPUT)
                    {
						button.im = imask;
					}
				}
			}
		}
	}
}
/*
 * Set any/all button parameters by sending embedded codes and data.
 */
function doBMF(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var data = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbBMF, data);
}
function cbBMF(name, button, bt, idx, data)
{
    var len = data.length;
    var bPFlag = false;
    var z = button.sr[idx].number;
    // var content = "";

    for (var a = 0; a < len; a++)
    {
        if (data.charAt(a) == '%' && !bPFlag)       // start of command
        {
            bPFlag = true;
            continue;
        }
        else if (data.charAt(a) == '%' && bPFlag)   // % is the content
        {
//            content += '%';
            bPFlag = false;
            continue;
        }
        else if (bPFlag && data.charAt(a) == 'R')   // Set rectangle.
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var str = "";

            if (next > a)
                str = data.substring(a, next);
            else
                str = data.substring(a);

            var parts = str.split(',');

            if (parts.length != 4)
            {
                errlog('doBMF: Rectangle (%R) needs 4 coordinates!');
                a = a + str.length;
                continue;
            }

            a = a + str.length;
            str = "^BSP-"+bt.bi+','+parts[0]+','+parts[1]+','+parts[2]+','+parts[3];
            doBSP(str);
        }
        else if (bPFlag && data.charAt(a) == 'B')   // Border style
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var str = "";

            if (next > a)
            {
                str = data.substring(a, next);
                a = next;
            }
            else
            {
                str = data.substring(a);
                a = data.length - 1;
            }

            doBRD("^BRD-"+bt.bi+','+z+','+str);
        }
        else if (bPFlag && data.charAt(a) == 'D' && data.charAt(a+1) == 'O')    // Draw order
        {
            errlog("doBMF: Command %DO is not implemented.");
            bPFlag = false;
            var next = data.indexOf('%', a);

            if (next > a)
                a = next;
            else
                a = data.length - 1;
        }
        else if (bPFlag && data.charAt(a) == 'F')   // Set the font
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var fID = 0;

            if (next > a)
                fID = parseInt(data.substring(a, next));
            else
                fID = parseInt(data.substring(a));

            var str = "^FON-"+bt.bi+','+z+','+fID;
            doFON(str);
        }
        else if (bPFlag && data.charAt(a) == 'M' && data.charAt(a+1) == 'I')    // Set the mask image
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var str = "";

            if (next > a)
            {
                str = data.substring(a, next);
                a = next;
            }
            else
            {
                str = data.substring(a);
                a = data.length - 1;
            }

            doBMI("^BMI-"+bt.bi+','+z+','+str);
        }
        else if (bPFlag && data.charAt(a) == 'T')   // Set the text
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var str = "";

            if (next > a)
            {
                str = data.substring(a, next);
                a = next;
            }
            else
            {
                str = data.substring(a);
                a = data.length - 1;
            }

            doTXT("^TXT-"+bt.bi+','+z+','+str);
        }
        else if (bPFlag && data.charAt(a) == 'P')   // Set the picture filename
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var str = "";

            if (next > a)
            {
                str = data.substring(a, next);
                a = next;
            }
            else
            {
                str = data.substring(a);
                a = data.length - 1;
            }

            doBMP("^BMP-"+bt.bi+','+z+','+str);
        }
        else if (bPFlag && data.charAt(a) == 'I')   // Set the icon number
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var fID = 0;

            if (next > a)
                fID = parseInt(data.substring(a, next));
            else
                fID = parseInt(data.substring(a));

            doICO("^ICO-"+bt.bi+','+z+','+fID);
        }
        else if (bPFlag && data.charAt(a) == 'J' && data.charAt(a+1) == 'T')    // Alignment of text
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var str = "";

            if (next > a)
            {
                str = data.substring(a, next);
                a = next;
            }
            else
            {
                str = data.substring(a);
                a = data.length - 1;
            }

            doJST("^JST-"+bt.bi+","+z+","+str);
        }
        else if (bPFlag && data.charAt(a) == 'J' && data.charAt(a+1) == 'B')    // Alignment of bitmap/picture
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var str = "";

            if (next > a)
            {
                str = data.substring(a, next);
                a = next;
            }
            else
            {
                str = data.substring(a);
                a = data.length - 1;
            }

            doJSB("^JSB-"+bt.bi+","+z+","+str);
        }
        else if (bPFlag && data.charAt(a) == 'J' && data.charAt(a+1) == 'I')    // Alignment of icon
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var str = "";

            if (next > a)
            {
                str = data.substring(a, next);
                a = next;
            }
            else
            {
                str = data.substring(a);
                a = data.length - 1;
            }

            doJSI("^JSI-"+bt.bi+","+z+","+str);
        }
        else if (bPFlag && data.charAt(a) == 'J')    // Alignment of text
        {
            bPFlag = false;
            var next = data.indexOf('%', a);
            var str = "";

            if (next > a)
            {
                str = data.substring(a, next);
                a = next;
            }
            else
            {
                str = data.substring(a);
                a = data.length - 1;
            }

            doJST("^JST-"+bt.bi+","+z+","+str);
        }
    }
}
/*
 * Set the button mask image.
 */
function doBMI(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var img = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbBMI, img);
}
function cbBMI(name, button, bt, idx, img)
{
    var sr = button.sr[idx].mi = img;

    try
    {
        // This is needed to be sure the element exists currently.
        var bsr = document.getElementById(name);

        if ((button.btype != BUTTONTYPE.BARGRAPH && button.btype != BUTTONTYPE.MULTISTATE_BARGRAPH && button.btype != BUTTONTYPE.MULTISTATE_GENERAL) &&
            button.sr.length == 2)	// chameleon image?
        {
            if (sr.bm.length > 0)
                drawButton(makeURL("images/"+img),makeURL("images/"+sr.bm),name,sr.mi_width, sr.mi_height, getAMXColor(sr.cf), getAMXColor(sr.cb));
            else
                drawArea(makeURL("images/"+img),name, sr.mi_width, sr.mi_height, getAMXColor(sr.cf), getAMXColor(sr.cb));
        }
        else if (button.btype == BUTTONTYPE.BARGRAPH && button.sr.length == 2 && sr.mi.length > 0 && idx == 0)
        {
            if (button.sr[idx+1].bm.length > 0)
            {
                var lev = getBargraphLevel(bt.pnum, bt.bi);
                var level = parseInt(100.0 / (button.rh - button.rl) * lev);
                var dir = true;

                if (button.dr == "horizontal")
                    dir = false;

                drawBargraph(makeURL("images/"+img), makeURL("images/"+button.sr[idx+1].bm), name, level, sr.mi_width, sr.mi_height, getAMXColor(button.sr[idx+1].cf), getAMXColor(button.sr[idx+1].cb), dir, true, button);
            }
            else
                drawArea(makeURL("images/"+img), name, sr.mi_width, sr.mi_height, getAMXColor(sr.cf), getAMXColor(sr.cb));
        }
    }
    catch(e)
    {
        errlog("cbBMI: Button " + name + " not found!");
    }
}
/*
 * Set the maximum length of the text area button.
 */
function doBML(msg)
{
	var addr = getField(msg, 0, ',');
	var len = parseInt(getField(msg, 1, ','));

	var addrRange = getRange(addr);

	for (var i = 0; i < addrRange.length; i++)
	{
		var bt = findButtonPort(addrRange[i]);

		if (bt.length == 0)
		{
			errlog("doBML: Error: Button "+addrRange[i]+" not found!");
			continue;
		}

		for (var b = 0; b < bt.length; b++)
		{
			var pgKey = getPage(bt[b].pnum);

            for (var z = 1; z <= bt[b].instances; z++)
            {
                for (var x = 0; x < pgKey.buttons.length; x++)
                {
                    var button = pgKey.buttons[x];

                    if (button.bID == bt[b].bi && button.btype == BUTTONTYPE.TEXT_INPUT && button.im.length == 0)
                    {
						if (len > 2000)
							len = 2000;
						else if (len < 0)
							len = 0;

						button.mt = len;
					}
				}
			}
		}
	}
}
/*
 * Assign a picture to those buttons with a defined addressrange.
 */
function doBMP(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var img = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbBMP, img);
}
function cbBMP(name, button, bt, idx, img)
{
	button.sr[idx].bm = img;

	if (button.sr[idx].bm_width == 0)
		button.sr[idx].bm_width = button.wt;

	if (button.sr[idx].bm_height == 0)
		button.sr[idx].bm_height = button.ht;

    try
    {
        document.getElementById(name).src = "image/"+img;
    }
    catch (e)
    {
        errlog("doBMP: No element of name " + name + " found!");
    }
}
/*
 * Set the button opacity.
 */
function doBOP(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var oo = getField(msg, 2, ',');
    var opacity = amxInt(oo);

    iterateButtonStates(addr, bts, cbBOP, opacity);
}
function cbBOP(name, button, bt, idx, opacity)
{
    try
    {
        button.sr[idx].oo = opacity;
        document.getElementById(name).style.opacity = 1.0 / 255.0 * opacity;
    }
    catch (e)
    {
        errlog("cbBOP: No element of name " + name + " found!");
    }
}
/*
 * Set a border to a specific border style associated with a border
 * value for those buttons with a defined address range.
 */
function doBOR(msg)
{
    var addr = getField(msg, 0, ',');
    var bname = getField(msg, 1, ',');
    var border = -1;

    if (bname.charAt(0) >= '0' && bname.charAt(0) <= '9')
        border = getBorderStyleNum(bname);

    if (border === -1)
        border = getBorderStyle(bname);

    if (border === -1)
    {
        errlog("doBOR: Invalid or unsupported border " + bname + "!");
        return;
    }

    var addrRange = getRange(addr);

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButtonPort(addrRange[i]);

        if (bt.length == 0)
        {
            errlog('doBOR: Error button ' + addrRange[i] + ' not found!');
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
            var button = getButton(bt[b].pnum, bt[b].bi);

            for (var j = 0; j < button.sr.length; j++)
            {
                var sr = button.sr[j];
                var name = 'Page_' + bt[b].pnum + "_Button_" + bt[b].bi + "_" + sr.number;

                try
                {
                    var style = document.getElementById(name).style;

                    for (var x = 0; x < border.length; x++)
                    {
                        switch(x)
                        {
                            case 0: style.borderStyle = border[x]; break;
                            case 1: style.borderWidth = border[x]; break;
                            case 2: style.borderRadius = border[x]; break;
                        }
                    }
                }
                catch(e)
                {
                    errlog("doBOR: Button " + name + " nicht gefunden!");
                }
            }
        }
    }
}
/*
 * Set the border of a button state/states.
 */
function doBRD(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var frame = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbBRD, frame);
}
function cbBRD(name, button, bt, idx, frame)
{
    button.sr[idx].bs = frame;
    var brd = getBorderStyle(button.sr[idx].bs);

    try
    {
        var bsr = document.getElementById(name);

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
        }
    }
    catch(e)
    {
        errlog("cbBRD: Button "+ name + " nicht gefunden!");
    }
}
/*
 * Set the focus to the text area.
 */
function doBSF(msg)
{
	var addr = getField(msg, 0, ',');
	var selv = getField(msg, 1, ',');

	var addrRange = getRange(addr);

	var bt = findButtonPort(addrRange[0]);

	if (bt.length == 0)
	{
		errlog("doBSF: Error: Button "+addrRange[0]+" not found!");
		return;
	}

	for (var b = 0; b < bt.length; b++)
	{
		var pgKey = getPage(bt[b].pnum);

		for (var x = 0; x < pgKey.buttons.length; x++)
		{
			var button = pgKey.buttons[x];

			if (button.bID == bt[b].bi && button.btype == BUTTONTYPE.TEXT_INPUT)
			{
				try
				{
					document.getElementById('"'+button.bname+'"').focus();
					break;
				}
				catch(e)
				{
					errlog("doBSF: Error button "+button.bname+" not found!");
				}
			}
		}
	}
}
/*
 * Set the button size and position.
 */
function doBSP(msg)
{
    var addr = getField(msg, 0, ',');
    var left = getField(msg, 1, ',');
    var top = getField(msg, 2, ',');
    var right = getField(msg, 3, ',');
    var bottom = getField(msg, 4, ',');

    var addrRange = getRange(addr);

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButtonPort(addrRange[i]);

        if (bt.length == 0)
        {
            errlog('doBSP: Error button ' + addrRange[i] + ' not found!');
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
            try
            {
                var button = getButton(bt[b].pnum, bt[b].bi);

                for (var x = 0; x < button.sr.length; x++)
                {
                    var name = 'Page_' + bt[b].pnum + "_Button_" + bt[b].bi + "_" + button.sr[x].number;

                    button.lt = left;
                    button.tp = top;
                    button.wt = right - left;
                    button.ht = bottom - top;
                    document.getElementById(name).style.left = left;
                    document.getElementById(name).style.top = left;
                    document.getElementById(name).style.width = right - left;
                    document.getElementById(name).style.height = bottom - top;
                }
            }
            catch (e)
            {
                errlog("doBSP: No element of name " + name + " found!");
            }
        }
    }
}
/*
 * Submit text for text area buttons.
 */
function doBSM(msg)
{
	var addr = getField(msg, 0, ',');
	var addrRange = getRange(addr);

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButtonPort(addrRange[i]);

        if (bt.length == 0)
        {
            errlog('doBSM: Error button ' + addrRange[i] + ' not found!');
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
			var pgKey = getPage(bt[b].pnum);

			for (var j = 0; j < pgKey.buttons.length; j++)
			{
				var button = pgKey.buttons[j];

				if (button.bID == bt[b].bi)
				{
					try
					{
						var txt = document.getElementById('"'+button.bname+"'").value;
						sendString(button.ap, button.ad, txt);
					}
					catch(e)
					{
						errlog("doBSM: Error at button "+button.bname+": "+e);
					}
				}
			}
		}
	}
}
/*
 * Set the sound played when a button is pressed.
 */
function doBSO(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');
	var sound = getField(msg, 2, ',');

	iterateButtonStates(addr, bts, cbBSO, sound);
}
function cbBSO(name, button, bt, idx, sound)
{
	if (soundExist(sound))
		button.sr[idx].sd = sound;
}
/*
 * Set the button word wrap feature to those buttons
 * with a defined address range.
 */
function doBWW(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');
	var ww = getField(msg, 2, ',');

	iterateButtonStates(addr, bts, cdBWW, ww);
}
function cbBWW(name, button, bt, idx, ww)
{
	button.sr[idx].ww = (ww == 0 ? 0 : 1);
}
/*
 * Clear all page flips from a button.
 */
function doCPF(msg)
{
    var addr = getField(msg, 0, ',');
    var addrRange = getRange(addr);

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButtonPort(addrRange[i]);

        if (bt.length == 0)
        {
            errlog('doCPF: Error button ' + addrRange[i] + ' not found!');
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
            var name = 'Page_' + bt[b].pnum + "_Button_" + bt[b].bi;

            try
            {
                var elem = document.getElementById(name);

                try
                {
                    elem.removeEventListener(EVENT_UP, showPopup);
                }
                catch (e)
                {}

                try
                {
                    elem.removeEventListener(EVENT_UP, hidePopup);
                }
                catch (e)
                {}

                try
                {
                    elem.removeEventListener(EVENT_UP, hideGroup);
                }
                catch (e)
                {}
            }
            catch (e)
            {
                errlog("doCPF: No element of name " + name + " found!");
            }
        }
    }
}
/*
 * Delete page flips from button if it already exists.
 */
function doDPF(msg)
{
	var addr = getField(msg, 0, ',');
	var action = getField(msg, 1, ',');
	var pname = getField(msg, 2, ',');

	var addrRange = getRange(addr);

	for (var i in addrRange)
	{
		findPopupName()
		var bt = findButton(addrRange[i]);

		for (var j in bt)
		{
			var page = getPage(bt[j].pnum);

			if (page.name == pname)
			{
				for (var x in page.buttons)
				{
					var button = page.buttons[x];

					if (button.bID = addrRange[i])
					{
						if (action.toLowerCase() == "stan")			// Flip to standard page
							_delPageFlip(button, "stan");
						if (action.toLowerCase() == "prev")			// Flip to previous page
							_delPageFlip(button, "prev");
						if (action.toLowerCase() == "show")			// Show popup page
							_delPageFlip(button, "sShow")
						else if (action.toLowerCase() == "hide")	// Hide popup page
							_delPageFlip(button, "sHide")
						else if (action.toLowerCase() == "togg")	// Toggle popup state
							_delPageFlip(button, "sToggle")
						else if (action.toLowerCase() == "clearg")	// Clear popup page group from all pages
							_delPageFlip(button, "scGroup")
						else if (action.toLowerCase() == "clearp")	// Clear all popup pages from a page with the specified page name
							_delPageFlip(button, "scPage")
						else if (action.toLowerCase() == "cleara")	// Clear all popup pages from all pages
							_delPageFlip(button, "scPanel")
					}
				}
			}
		}
	}
}
function _delPageFlip(but, name)
{
	for (var a in but.pf)
	{
		var pf = but.pf[a];

		if (pf.pfType == name)
		{
			but.pf.splice(a, 1);
			break;
		}
	}
}
/*
 * Enable or disable buttons with a set variable text range.
 */
function doENA(msg)
{
    var addr = getField(msg, 0, ',');
    var val = getField(msg, 1, ',');
    var addrRange = getRange(addr);

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButton(addrRange[i]);

        if (bt.length == 0)
        {
            errlog('doCPF: Error button ' + addrRange[i] + ' not found!');
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
            var name = 'Page_' + bt[b].pnum + "_Button_" + bt[b].bi;
            bt[b].enabled = val;
        }
    }
}
/*
 * Set a font to a specific Font ID value for those buttons with a
 * defined address range.
 */
function doFON(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var fID = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbFON, fID);
}
function cbFON(name, button, bt, idx, fID)
{
	var sr = button.sr[idx].fi = fID;

	try
	{
		var font = findFont(fID);

		if (font === -1)
			return;

		document.getElementById(name).style.fontFamily = font.name;
		document.getElementById(name).style.fontSize = font.size+"pt";
		document.getElementById(name).style.fontStyle = getFontStyle(font.subfamilyName);
		document.getElementById(name).style.fontWeight = getFontWeight(font.subfamilyName);
	}
	catch(e)
	{
		errlog('cbFON: Button '+name+' not found.');
	}
}
/*
 * Change the bargraph drag increment
 */
function doGDI(msg)
{
	var addr = getField(msg, 0, ',');
	var inc = getField(msg, 1, ',');

	var addrRange = getRange(addr);

	for (var i in addrRange)
	{
		var bt = findBargraphs(addrRange[i]);

		for (var j in bt)
		{
			var button = getButton(bt[j].pnum, bt[j].bi);

			if (button === null)
				continue;

			button.rn = inc;
		}
	}
}
/*
 * Change the bargraph upper limit.
 */
function doGLH(msg)
{
	var addr = getField(msg, 0, ',');
	var limit = getField(msg, 1, ',');

	var addrRange = getRange(addr);

	for (var i in addrRange)
	{
		var bt = findBargraphs(addrRange[i]);

		for (var j in bt)
		{
			var button = getButton(bt[j].pnum, bt[j].bi);

			if (button === null)
				continue;

			button.rh = limit;
		}
	}
}
/*
 * Change the bargraph lower limit.
 */
function doGLL(msg)
{
	var addr = getField(msg, 0, ',');
	var limit = getField(msg, 1, ',');

	var addrRange = getRange(addr);

	for (var i in addrRange)
	{
		var bt = findBargraphs(addrRange[i]);

		for (var j in bt)
		{
			var button = getButton(bt[j].pnum, bt[j].bi);

			if (button === null)
				continue;

			button.rl = limit;
		}
	}
}
/*
 * Change the bargraph ramp-down time in 1/10th of a second.
 */
function doGRD(msg)
{
	var addr = getField(msg, 0, ',');
	var limit = getField(msg, 1, ',');

	var addrRange = getRange(addr);

	for (var i in addrRange)
	{
		var bt = findBargraphs(addrRange[i]);

		for (var j in bt)
		{
			var button = getButton(bt[j].pnum, bt[j].bi);

			if (button === null)
				continue;

			button.rd = limit;
		}
	}
}
/*
 * Change the bargraph ramp-up time in 1/10th of a second.
 */
function doGRU(msg)
{
	var addr = getField(msg, 0, ',');
	var limit = getField(msg, 1, ',');

	var addrRange = getRange(addr);

	for (var i in addrRange)
	{
		var bt = findBargraphs(addrRange[i]);

		for (var j in bt)
		{
			var button = getButton(bt[j].pnum, bt[j].bi);

			if (button === null)
				continue;

			button.ru = limit;
		}
	}
}
/*
 * Change the bargraph slider color or joystick cursor color.
 */
function doGSC(msg)
{
	var addr = getField(msg, 0, ',');
	var color = getField(msg, 1, ',');

	var addrRange = getRange(addr);

	for (var i in addrRange)
	{
		var bt = findBargraphs(addrRange[i]);

		for (var j in bt)
		{
			var button = getButton(bt[j].pnum, bt[j].bi);

			if (button === null)
				continue;

			button.sc = color;
		}
	}
}
/*
 * Set the icon to a button.
 */
async function doICO(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var idx = getField(msg, 2, ',');

    iterateButtonStates(addr, bts, cbICO, idx);
}
function cbICO(name, button, bt, i, idx)
{
    button.sr[i].ii = idx;
    var newIcon = false;
    var iFile = getIconFile(idx);

    if (iFile !== null)
    {
        try
        {
            var img = document.getElementById(name + '_img');
            img.src = "images/" + iFile;
            var dim = getIconDim(idx);
            img.width = dim[0];
            img.height = dim[1];
            img.removeAttribute("style");
            img.style.display = "flex";
            img.style.order = 2;
            justifyImage(img, button, CENTER_CODE.SC_ICON, button.sr[i].number);
        }
        catch (e)
        {
            newIcon = true;
        }
    }
    else
    {
        // Delete icon if it exists
        try
        {
            var ico = document.getElementById(name + '_img');
            var parent = document.getElementById(name);

            if (parent !== null && ico !== null)
                parent.removeChild(ico);
            else if (ico !== null)
                ico.style.display = 'none';
        }
        catch (e)
        {
            TRACE("cbICO: Delete icon error: " + e);
        }
    }

    if (newIcon)
    {
        // Create a new icon image
        var span;
        var hasSpan = false;

        try
        {
            span = document.getElementById(name + '_font');
            hasSpan = true;
        }
        catch (e)
        {
            hasSpan = false;
        }

        var parent;
        var hasParent = false;
        var err = "";

        try
        {
            parent = document.getElementById(name);

            if (parent !== null)
                hasParent = true;
        }
        catch (e)
        {
            err = e;
        }

        if (!hasParent || parent === null)
        {
            errlog("cbICO: No parent of name " + name + " found! [" + err + "]");
            return;
        }

        var dim = getIconDim(idx);

        if (iFile !== null)
        {
            var img = document.createElement('img');
            img.src = makeURL('images/' + iFile);
            img.id = name + '_img';
            img.width = dim[0];
            img.height = dim[1];
            img.style.display = "flex";
            img.style.order = 2;
            parent.appendChild(img);

            if (hasSpan)
                parent.insertBefore(img, span);

            var icoPos = getIconPosInfo(bt.pnum, bt.bi, button.sr[i].number);

            if (icoPos !== null)
                justifyImage(img, button, CENTER_CODE.SC_ICON, button.sr[i].number);
        }
    }
}
/*
 * Set bitmap/picture alignment using a numeric keypad layout
 * for those buttons with a defined address range.
 */
function doJSB(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var align = getField(msg, 2, ',');
    var pars = [];
    pars.push(align);

    if (align == 0)
    {
        pars.push(getField(msg, 3, ','));
        pars.push(getField(msg, 4, ','));
    }

    iterateButtonStates(addr, bts, cbJSB, pars);
}
function cbJSB(name, button, bt, idx, pars)
{
    var left, top;
    var align = pars[0];

    if (align == 0)
    {
        left = pars[1];
        top = pars[2];
    }

    button.sr[idx].jb = align;

    if (align == 0)
    {
        button.sr[idx].ix = left;
        button.sr[idx].iy = top;
    }

    try
    {
        var bsr = document.getElementById(name);

        switch (align)
        {
            case 0:
                bsr.style.backgroundPositionX = left+'px';
                bsr.style.backgroundPositionY = top+'px';
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
    catch (e)
    {
        errlog("cbJSB: Button " + name + " not found!");
    }
}
/*
 * Set icon alignment using a numeric keypad layout
 * for those buttons with a defined address range.
 */
function doJSI(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var align = getField(msg, 2, ',');
    var pars = [];
    pars.push(align);

    if (align == 0)
    {
        pars.push(getField(msg, 3, ','));
        pars.push(getField(msg, 4, ','));
    }

    iterateButtonStates(addr, bts, cbJSI, pars);
}
function cbJSI(name, button, bt, idx, pars)
{
    var left, top;
    var align = pars[0];

    if (align == 0)
    {
        left = pars[1];
        top = pars[2];
    }

    button.sr[idx].ji = align;

    if (align == 0)
    {
        button.sr[idx].ix = left;
        button.sr[idx].iy = top;
    }

    try
    {
        var img = document.getElementById(name);
        justifyImage(img, button, CENTER_CODE.SC_ICON, button.sr[idx].number);
    }
    catch (e)
    {
        errlog("cbJSI: Button " + name + " not found!");
    }
}
/*
 * Set text alignment using a numeric keypad layout
 * for those buttons with a defined address range.
 */
function doJST(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var align = getField(msg, 2, ',');
    var pars = [];
    pars.push(align);

    if (align == 0)
    {
        pars.push(getField(msg, 3, ','));
        pars.push(getField(msg, 4, ','));
    }

    iterateButtonStates(addr, bts, cbJST, pars);
}
function cbJST(name, button, bt, idx, pars)
{
    var left, top;
    var align = pars[0];

    if (align == 0)
    {
        left = pars[1];
        top = pars[2];
    }

    button.sr[idx].jt = align;

    if (align == 0)
    {
        button.sr[idx].tx = left;
        button.sr[idx].ty = top;
    }

    try
    {
        var fnt = document.getElementById(name);
        var sr = button.sr[idx];

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
    }
    catch (e)
    {
        errlog("cbJST: Button " + name + " not found!");
    }
}
/*
 * Add new resources.
 */
function doRAF(msg)
{
    var protocol = "",
        host = "",
        path = "",
        file = "";
    var name = getField(msg, 0, ',');
    var data = getField(msg, 1, ',');

    var teile = data.split("%");
    var order = [];

    for (var i in teile)
    {
        var str = teile[i];

        if (teile[i].length == 0)
            continue;

        switch (str.charAt(0))
        {
            case 'P':
                order.push({ "type": "P", "str": str.substr(1) });
            break;

            case 'H':
                order.push({ "type": "H", "str": str.substr(1) });
            break;

            case 'A':
                order.push({ "type": "A", "str": str.substr(1) });
            break;

            case 'F':
                order.push({ "type": "F", "str": str.substr(1) });
            break;

            case 'U':
                order.push({ "type": "U", "str": str.substr(1) });
            break;

            case 'S':
                order.push({ "type": "S", "str": str.substr(1) });
            break;

            case 'R':
                order.push({ "type": "R", "str": str.substr(1) });
            break;
        }
    }

    for (var i in order)
    {
        var str = order[i].str.replace("%%", "%");

        switch (order[i].type)
        {
            case 'P': // Protocol
                protocol = str;
            break;

            case 'H': // Host
                host = str;
            break;

            case 'A': // Path
                path = str;
            break;

            case 'F': // File
                file = str;
            break;

            case 'U': // User
                user = str;
            break;

            case 'S': // Password
                pass = str;
            break;

            case 'R': // Refresh
                refresh = parseInt(str);
            break;
        }
    }

    addResource(name, protocol, host, path, file, user, pass, refresh);
}
/*
 * Force a refresh for a given resource.
 */
function doRFR(msg)
{
    var name = getField(msg, 0, ',');
    refreshResource(name);
}
/*
 * Modify an existing resource.
 */
function doRMF(msg)
{
    var protocol = "",
        host = "",
        path = "",
        file = "",
        user = "",
        pass = "";
    var refresh = -1;
    var name = getField(msg, 0, ',');
    var data = getField(msg, 1, ',');

    var teile = data.split("%");
    var order = [];

    for (var i in teile)
    {
        var str = teile[i];

        if (teile[i].length == 0)
            continue;

        switch (str.charAt(0))
        {
            case 'P':
                order.push({ "type": "P", "str": str.substr(1) });
                break;
            case 'H':
                order.push({ "type": "H", "str": str.substr(1) });
                break;
            case 'A':
                order.push({ "type": "A", "str": str.substr(1) });
                break;
            case 'F':
                order.push({ "type": "F", "str": str.substr(1) });
                break;
            case 'U':
                order.push({ "type": "U", "str": str.substr(1) });
                break;
            case 'S':
                order.push({ "type": "S", "str": str.substr(1) });
                break;
            case 'R':
                order.push({ "type": "R", "str": str.substr(1) });
                break;
        }
    }

    for (var i in order)
    {
        var str = order[i].str.replace("%%", "%");

        switch (order[i].type)
        {
            case 'P': // Protocol
                protocol = str;
                break;

            case 'H': // Host
                host = str;
                break;

            case 'A': // Path
                path = str;
                break;

            case 'F': // File
                file = str;
                break;

            case 'U': // User
                user = str;
                break;

            case 'S': // Password
                pass = str;
                break;

            case 'R': // Refresh
                refresh = parseInt(str);
                break;
        }
    }

    var r = findRessource(name);

    if (refresh == -1)
        refresh = r.refresh;

    var res = { "name": name, "protocol": protocol, "host": host, "path": path, "file": file, "user": user, "password": pass, "refresh": refresh, "encrypted": false };
    updateResource(res);
}
/*
 * Change the refresh rate for a given resource.
 */
function doRSR(msg)
{
    var name = getField(msg, 0, ',');
    var refresh = getField(msg, 1, ',');

    var res = findRessource(name);

    if (res === null)
    {
        errlog("doRSR: Resource " + name + " not found!");
        return;
    }

    res.refresh = parseInt(refresh);
    updateResource(res);
}
/*
 * Show or hide a button with a set variable text range.
 */
function doSHO(msg)
{
    var addr = getField(msg, 0, ',');
    var stat = getField(msg, 1, ',');

    var addrRange = getRange(addr);

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButton(addrRange[i]);

        if (bt.length == 0)
        {
            errlog('doSHO: Error button ' + addrRange[i] + ' not found!');
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
            for (var z = 1; z <= bt[b].instances; z++)
            {
                var name = 'Page_' + bt[b].pnum + "_Button_" + bt[b].bi + "_" + z;

                try
                {
                    bt[b].visible = stat;
                    document.getElementById(name).style.display = ((stat == 0) ? 'none' : 'inline');
                }
                catch (e)
                {
                    errlog("doSHO: No element of name " + name + " found! [" + e + "]");
                }
            }
        }
    }
}
/*
 * Set the text effect color for the specified addresses/states
 * to the specified color.
 */
function doTEC(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');
	var color = getField(msg, 2, ',');

	iterateButtonStates(addr, bts, cbTEC, color);
}
function cbTEC(name, button, bt, idx, color)
{
	button.sr[idx].ec = color;

	if (button.sr[idx].et > 0)
		cbTEF(name, button, bt, idx, button.sr[idx].et);
}
/*
 * Set the text effect.
 */
function doTEF(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');
	var effect = getField(msg, 2, ',');

	var tef = 0;

	if (effect == "Outline-S")
		tef = 1;
	else if (effect == "Outline-M")
		tef = 2;
	else if (effect == "Outline-L")
		tef = 3;
	else if (effect == "Outline-X")
		tef = 4;
	else if (effect == "Glow-S")
		tef = 5;
	else if (effect == "Glow-M")
		tef = 6;
	else if (effect == "Glow-L")
		tef = 7;
	else if (effect == "Glow-X")
		tef = 8;
	else if (effect == "Soft Drop Shadow 1")
		tef = 9;
	else if (effect == "Soft Drop Shadow 2")
		tef = 10;
	else if (effect == "Soft Drop Shadow 3")
		tef = 11;
	else if (effect == "Soft Drop Shadow 4")
		tef = 12;
	else if (effect == "Soft Drop Shadow 5")
		tef = 13;
	else if (effect == "Soft Drop Shadow 6")
		tef = 14;
	else if (effect == "Soft Drop Shadow 7")
		tef = 15;
	else if (effect == "Soft Drop Shadow 8")
		tef = 16;
	else if (effect == "Medium Drop Shadow 1")
		tef = 17;
	else if (effect == "Medium Drop Shadow 2")
		tef = 18;
	else if (effect == "Medium Drop Shadow 3")
		tef = 19;
	else if (effect == "Medium Drop Shadow 4")
		tef = 20;
	else if (effect == "Medium Drop Shadow 5")
		tef = 21;
	else if (effect == "Medium Drop Shadow 6")
		tef = 22;
	else if (effect == "Medium Drop Shadow 7")
		tef = 23;
	else if (effect == "Medium Drop Shadow 8")
		tef = 24;
	else if (effect == "Hard Drop Shadow 1")
		tef = 25;
	else if (effect == "Hard Drop Shadow 2")
		tef = 26;
	else if (effect == "Hard Drop Shadow 3")
		tef = 27;
	else if (effect == "Hard Drop Shadow 4")
		tef = 28;
	else if (effect == "Hard Drop Shadow 5")
		tef = 29;
	else if (effect == "Hard Drop Shadow 6")
		tef = 30;
	else if (effect == "Hard Drop Shadow 7")
		tef = 31;
	else if (effect == "Hard Drop Shadow 8")
		tef = 32;
	else if (effect == "Soft Drop Shadow 1 with outline")
		tef = 33;
	else if (effect == "Soft Drop Shadow 2 with outline")
		tef = 34;
	else if (effect == "Soft Drop Shadow 3 with outline")
		tef = 35;
	else if (effect == "Soft Drop Shadow 4 with outline")
		tef = 36;
	else if (effect == "Soft Drop Shadow 5 with outline")
		tef = 37;
	else if (effect == "Soft Drop Shadow 6 with outline")
		tef = 38;
	else if (effect == "Soft Drop Shadow 7 with outline")
		tef = 39;
	else if (effect == "Soft Drop Shadow 8 with outline")
		tef = 40;
	else if (effect == "Medium Drop Shadow 1 with outline")
		tef = 41;
	else if (effect == "Medium Drop Shadow 2 with outline")
		tef = 42;
	else if (effect == "Medium Drop Shadow 3 with outline")
		tef = 43;
	else if (effect == "Medium Drop Shadow 4 with outline")
		tef = 44;
	else if (effect == "Medium Drop Shadow 5 with outline")
		tef = 45;
	else if (effect == "Medium Drop Shadow 6 with outline")
		tef = 46;
	else if (effect == "Medium Drop Shadow 7 with outline")
		tef = 47;
	else if (effect == "Medium Drop Shadow 8 with outline")
		tef = 48;
	else if (effect == "Hard Drop Shadow 1 with outline")
		tef = 49;
	else if (effect == "Hard Drop Shadow 2 with outline")
		tef = 50;
	else if (effect == "Hard Drop Shadow 3 with outline")
		tef = 51;
	else if (effect == "Hard Drop Shadow 4 with outline")
		tef = 52;
	else if (effect == "Hard Drop Shadow 5 with outline")
		tef = 53;
	else if (effect == "Hard Drop Shadow 6 with outline")
		tef = 54;
	else if (effect == "Hard Drop Shadow 7 with outline")
		tef = 55;
	else if (effect == "Hard Drop Shadow 8 with outline")
		tef = 56;

	iterateButtonStates(addr, bts, cbTEF, tef);
}
function cbTEF(name, button, bt, idx, effect)
{
	button.sr[idx].et = effect;
	var tef = "";

	switch(effect)
	{
		case 0: tef = ""; break;
		// Outline
		case 1: tef = "-1px -1px "+getWebColor(button.sr[idx].ec)+",1px -1px "+getWebColor(button.sr[idx].ec)+",1px -1px "+getWebColor(button.sr[idx].ec)+",1px 1px "+getWebColor(button.sr[idx].ec); break;
		case 2: tef = "-2px -2px "+getWebColor(button.sr[idx].ec)+",2px -2px "+getWebColor(button.sr[idx].ec)+",2px -2px "+getWebColor(button.sr[idx].ec)+",2px 2px "+getWebColor(button.sr[idx].ec); break;
		case 3: tef = "-3px -3px "+getWebColor(button.sr[idx].ec)+",3px -3px "+getWebColor(button.sr[idx].ec)+",3px -3px "+getWebColor(button.sr[idx].ec)+",3px 3px "+getWebColor(button.sr[idx].ec); break;
		case 4: tef = "-4px -4px "+getWebColor(button.sr[idx].ec)+",4px -4px "+getWebColor(button.sr[idx].ec)+",4px -4px "+getWebColor(button.sr[idx].ec)+",4px 4px "+getWebColor(button.sr[idx].ec); break;
		// Glow
		case 5: tef = "0px 0px 1px "+getWebColor(button.sr[idx].ec); break;
		case 6: tef = "0px 0px 2px "+getWebColor(button.sr[idx].ec); break;
		case 7: tef = "0px 0px 3px "+getWebColor(button.sr[idx].ec); break;
		case 8: tef = "0px 0px 4px "+getWebColor(button.sr[idx].ec); break;
		// Soft Drop Shadow
		case 9: tef = "1px 1px 1px "+getWebColor(button.sr[idx].ec); break;
		case 10: tef = "1px 1px 2px "+getWebColor(button.sr[idx].ec); break;
		case 11: tef = "1px 1px 3px "+getWebColor(button.sr[idx].ec); break;
		case 12: tef = "1px 1px 4px "+getWebColor(button.sr[idx].ec); break;
		case 13: tef = "1px 1px 5px "+getWebColor(button.sr[idx].ec); break;
		case 14: tef = "1px 1px 6px "+getWebColor(button.sr[idx].ec); break;
		case 15: tef = "1px 1px 7px "+getWebColor(button.sr[idx].ec); break;
		case 16: tef = "1px 1px 8px "+getWebColor(button.sr[idx].ec); break;
		// Medium Drop Shadow
		case 17: tef = "2px 2px 1px "+getWebColor(button.sr[idx].ec); break;
		case 18: tef = "2px 2px 2px "+getWebColor(button.sr[idx].ec); break;
		case 19: tef = "2px 2px 3px "+getWebColor(button.sr[idx].ec); break;
		case 20: tef = "2px 2px 4px "+getWebColor(button.sr[idx].ec); break;
		case 21: tef = "2px 2px 5px "+getWebColor(button.sr[idx].ec); break;
		case 22: tef = "2px 2px 6px "+getWebColor(button.sr[idx].ec); break;
		case 23: tef = "2px 2px 7px "+getWebColor(button.sr[idx].ec); break;
		case 24: tef = "2px 2px 8px "+getWebColor(button.sr[idx].ec); break;
		// Hard Drop Shadow
		case 25: tef = "3px 3px 1px "+getWebColor(button.sr[idx].ec); break;
		case 26: tef = "3px 3px 2px "+getWebColor(button.sr[idx].ec); break;
		case 27: tef = "3px 3px 3px "+getWebColor(button.sr[idx].ec); break;
		case 28: tef = "3px 3px 4px "+getWebColor(button.sr[idx].ec); break;
		case 29: tef = "3px 3px 5px "+getWebColor(button.sr[idx].ec); break;
		case 30: tef = "3px 3px 6px "+getWebColor(button.sr[idx].ec); break;
		case 31: tef = "3px 3px 7px "+getWebColor(button.sr[idx].ec); break;
		case 32: tef = "3px 3px 8px "+getWebColor(button.sr[idx].ec); break;
		// Soft Drop Shadow with outline
		case 33: tef = "-1px -1px 1px "+getWebColor(button.sr[idx].ec)+",1px 1px 1px "+getWebColor(button.sr[idx].ec); break;
		case 34: tef = "-1px -1px 2px "+getWebColor(button.sr[idx].ec)+",1px 1px 2px "+getWebColor(button.sr[idx].ec); break;
		case 35: tef = "-1px -1px 3px "+getWebColor(button.sr[idx].ec)+",1px 1px 3px "+getWebColor(button.sr[idx].ec); break;
		case 36: tef = "-1px -1px 4px "+getWebColor(button.sr[idx].ec)+",1px 1px 4px "+getWebColor(button.sr[idx].ec); break;
		case 37: tef = "-1px -1px 5px "+getWebColor(button.sr[idx].ec)+",1px 1px 5px "+getWebColor(button.sr[idx].ec); break;
		case 38: tef = "-1px -1px 6px "+getWebColor(button.sr[idx].ec)+",1px 1px 6px "+getWebColor(button.sr[idx].ec); break;
		case 39: tef = "-1px -1px 7px "+getWebColor(button.sr[idx].ec)+",1px 1px 7px "+getWebColor(button.sr[idx].ec); break;
		case 40: tef = "-1px -1px 8px "+getWebColor(button.sr[idx].ec)+",1px 1px 8px "+getWebColor(button.sr[idx].ec); break;
		// Medium Drop Shadow with outline
		case 41: tef = "-2px -2px 1px "+getWebColor(button.sr[idx].ec)+",2px 2px 1px "+getWebColor(button.sr[idx].ec); break;
		case 42: tef = "-2px -2px 2px "+getWebColor(button.sr[idx].ec)+",2px 2px 2px "+getWebColor(button.sr[idx].ec); break;
		case 43: tef = "-2px -2px 3px "+getWebColor(button.sr[idx].ec)+",2px 2px 3px "+getWebColor(button.sr[idx].ec); break;
		case 44: tef = "-2px -2px 4px "+getWebColor(button.sr[idx].ec)+",2px 2px 4px "+getWebColor(button.sr[idx].ec); break;
		case 45: tef = "-2px -2px 5px "+getWebColor(button.sr[idx].ec)+",2px 2px 5px "+getWebColor(button.sr[idx].ec); break;
		case 46: tef = "-2px -2px 6px "+getWebColor(button.sr[idx].ec)+",2px 2px 6px "+getWebColor(button.sr[idx].ec); break;
		case 47: tef = "-2px -2px 7px "+getWebColor(button.sr[idx].ec)+",2px 2px 7px "+getWebColor(button.sr[idx].ec); break;
		case 48: tef = "-2px -2px 8px "+getWebColor(button.sr[idx].ec)+",2px 2px 8px "+getWebColor(button.sr[idx].ec); break;
		// Hard Drop Shadow with outline
		case 49: tef = "-3px -3px 1px "+getWebColor(button.sr[idx].ec)+",3px 3px 1px "+getWebColor(button.sr[idx].ec); break;
		case 50: tef = "-3px -3px 2px "+getWebColor(button.sr[idx].ec)+",3px 3px 2px "+getWebColor(button.sr[idx].ec); break;
		case 51: tef = "-3px -3px 3px "+getWebColor(button.sr[idx].ec)+",3px 3px 3px "+getWebColor(button.sr[idx].ec); break;
		case 52: tef = "-3px -3px 4px "+getWebColor(button.sr[idx].ec)+",3px 3px 4px "+getWebColor(button.sr[idx].ec); break;
		case 53: tef = "-3px -3px 5px "+getWebColor(button.sr[idx].ec)+",3px 3px 5px "+getWebColor(button.sr[idx].ec); break;
		case 54: tef = "-3px -3px 6px "+getWebColor(button.sr[idx].ec)+",3px 3px 6px "+getWebColor(button.sr[idx].ec); break;
		case 55: tef = "-3px -3px 7px "+getWebColor(button.sr[idx].ec)+",3px 3px 7px "+getWebColor(button.sr[idx].ec); break;
		case 56: tef = "-3px -3px 8px "+getWebColor(button.sr[idx].ec)+",3px 3px 8px "+getWebColor(button.sr[idx].ec); break;
	}

	try
	{
		if (button.btype == BUTTONTYPE.TEXT_INPUT)
			document.getElementById('"'+button.bname+'"').style.textShadow = tef;
		else
        	document.getElementById(name + '_font').style.textShadow = tef;
	}
    catch (e)
    {
        errlog("cbTEF: No parent of name " + name + " found!");
    }
}
/*
 * Assign a text string to those buttons with a defined address range.
 */
function doTXT(msg)
{
    var addr = getField(msg, 0, ',');
    var bts = getField(msg, 1, ',');
    var text = getField(msg, 2, ',');

    text = textToWeb(text);
    iterateButtonStates(addr, bts, cbTXT, text);
}
function cbTXT(name, button, bt, idx, text)
{
    button.sr[idx].te = text;

    try
    {
		if (button.btype == BUTTONTYPE.TEXT_INPUT)
			document.getElementById('"'+button.bname+'"').value = text;
		else
        	document.getElementById(name + '_font').innerHTML = text;
    }
    catch (e)
    {
        errlog("cbTXT: No parent of name " + name + " found!");
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
/*
 * Play a sound file.
 */
function doSOU(msg)
{
	var sound = getField(msg, 0, ',');

	if (!soundExist(sound))
	{
		errlog("doSOU: Sound file "+sound+" does not exist!");
		return;
	}

	try
	{
		var snd = new Audio("sounds/"+sound);
		snd.play();
	}
	catch(e)
	{
		errlog("doSOU: Error: "+e);
	}
}
/*
 * Get the current border color.
 */
function doGetBCB(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetBCB, 0, false);
}
function cbGetBCB(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1011:"+button.sr[idx].number+":"+button.sr[idx].cb.length+":0:1:"+button.sr[idx].cb);
}
/*
 * Get the current fill color.
 */
function doGetBCF(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetBCF, 0, false);
}
function cbGetBCF(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1012:"+button.sr[idx].number+":"+button.sr[idx].cf.length+":0:1:"+button.sr[idx].cf);
}
/*
 * Get the current text color.
 */
function doGetBCT(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetBCT, 0, false);
}
function cbGetBCT(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1013:"+button.sr[idx].number+":"+button.sr[idx].ct.length+":0:1:"+button.sr[idx].ct);
}
/*
 * Get the current bitmap name.
 */
function doGetBMP(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetBMP, 0, false);
}
function cbGetBMP(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1002:"+button.sr[idx].number+":"+button.sr[idx].bm.length+":0:1:"+button.sr[idx].bm);
}
/*
 * Get the overall button opacity.
 */
function doGetBOP(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetBOP, 0, false);
}
function cbGetBOP(name, button, bt, idx, par)
{
	var page = getPage(bt.pnum);
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1015:"+button.sr[idx].number+":"+page.sr[0].oo+":0:1:");
}
/*
 * Get the current border name.
 */
function doGetBRD(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetBRD, 0, false);
}
function cbGetBRD(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1014:"+button.sr[idx].number+":"+button.sr[idx].bs.length+":0:1:"+button.sr[idx].bs);
}
/*
 * Get the current word wrap flag status.
 */
function doGetBWW(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetBWW, 0, false);
}
function cbGetBWW(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1010:"+button.sr[idx].number+":"+button.sr[idx].ww+":0:1:");
}
/*
 * Get the current font index.
 */
function doGetFON(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetFON, 0, false);
}
function cbGetFON(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1007:"+button.sr[idx].number+":"+button.sr[idx].fi+":0:1:");
}
/*
 * Get the current icon index.
 */
function doGetICO(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetICO, 0, false);
}
function cbGetICO(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1003:"+button.sr[idx].number+":"+button.sr[idx].ii+":0:1:");
}
/*
 * Get the current bitmap justification.
 */
function doGetJSB(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetJSB, 0, false);
}
function cbGetJSB(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1005:"+button.sr[idx].number+":"+button.sr[idx].jb+":0:1:");
}
/*
 * Get the current icon justification.
 */
function doGetJSI(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetJSI, 0, false);
}
function cbGetJSI(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1006:"+button.sr[idx].number+":"+button.sr[idx].ji+":0:1:");
}
/*
 * Get the current text justification.
 */
function doGetJST(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetJST, 0, false);
}
function cbGetJST(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1004:"+button.sr[idx].number+":"+button.sr[idx].jt+":0:1:");
}
/*
 * Get the current text effect color.
 */
function doGetTEC(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetTEC, 0, false);
}
function cbGetTEC(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1009:"+button.sr[idx].number+":"+button.sr[idx].ec.length+":0:1:"+button.sr[idx].ec);
}
/*
 * Get the current text effect name.
 */
function doGetTEF(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetTEF, 0, false);
}
function cbGetTEF(name, button, bt, idx, par)
{
	var tef = "";

	switch (button.sr[idx].et)
	{
		case 1:  tef = "Outline-S"; break;
		case 2:  tef = "Outline-M"; break;
		case 3:  tef = "Outline-L"; break;
		case 4:  tef = "Outline-X"; break;
		case 5:  tef = "Glow-S"; break;
		case 6:  tef = "Glow-M"; break;
		case 7:  tef = "Glow-L"; break;
		case 8:  tef = "Glow-X"; break;
		case 9:  tef = "Soft Drop Shadow 1"; break;
		case 10: tef = "Soft Drop Shadow 2"; break;
		case 11: tef = "Soft Drop Shadow 3"; break;
		case 12: tef = "Soft Drop Shadow 4"; break;
		case 13: tef = "Soft Drop Shadow 5"; break;
		case 14: tef = "Soft Drop Shadow 6"; break;
		case 15: tef = "Soft Drop Shadow 7"; break;
		case 16: tef = "Soft Drop Shadow 8"; break;
		case 17: tef = "Medium Drop Shadow 1"; break;
		case 18: tef = "Medium Drop Shadow 2"; break;
		case 19: tef = "Medium Drop Shadow 3"; break;
		case 20: tef = "Medium Drop Shadow 4"; break;
		case 21: tef = "Medium Drop Shadow 5"; break;
		case 22: tef = "Medium Drop Shadow 6"; break;
		case 23: tef = "Medium Drop Shadow 7"; break;
		case 24: tef = "Medium Drop Shadow 8"; break;
		case 25: tef = "Hard Drop Shadow 1"; break;
		case 26: tef = "Hard Drop Shadow 2"; break;
		case 27: tef = "Hard Drop Shadow 3"; break;
		case 28: tef = "Hard Drop Shadow 4"; break;
		case 29: tef = "Hard Drop Shadow 5"; break;
		case 30: tef = "Hard Drop Shadow 6"; break;
		case 31: tef = "Hard Drop Shadow 7"; break;
		case 32: tef = "Hard Drop Shadow 8"; break;
		case 33: tef = "Soft Drop Shadow 1 with outline"; break;
		case 34: tef = "Soft Drop Shadow 2 with outline"; break;
		case 35: tef = "Soft Drop Shadow 3 with outline"; break;
		case 36: tef = "Soft Drop Shadow 4 with outline"; break;
		case 37: tef = "Soft Drop Shadow 5 with outline"; break;
		case 38: tef = "Soft Drop Shadow 6 with outline"; break;
		case 39: tef = "Soft Drop Shadow 7 with outline"; break;
		case 40: tef = "Soft Drop Shadow 8 with outline"; break;
		case 41: tef = "Medium Drop Shadow 1 with outline"; break;
		case 42: tef = "Medium Drop Shadow 2 with outline"; break;
		case 43: tef = "Medium Drop Shadow 3 with outline"; break;
		case 44: tef = "Medium Drop Shadow 4 with outline"; break;
		case 45: tef = "Medium Drop Shadow 5 with outline"; break;
		case 46: tef = "Medium Drop Shadow 6 with outline"; break;
		case 47: tef = "Medium Drop Shadow 7 with outline"; break;
		case 48: tef = "Medium Drop Shadow 8 with outline"; break;
		case 49: tef = "Hard Drop Shadow 1 with outline"; break;
		case 50: tef = "Hard Drop Shadow 2 with outline"; break;
		case 51: tef = "Hard Drop Shadow 3 with outline"; break;
		case 52: tef = "Hard Drop Shadow 4 with outline"; break;
		case 53: tef = "Hard Drop Shadow 5 with outline"; break;
		case 54: tef = "Hard Drop Shadow 6 with outline"; break;
		case 55: tef = "Hard Drop Shadow 7 with outline"; break;
		case 56: tef = "Hard Drop Shadow 8 with outline"; break;
	}

	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1008:"+button.sr[idx].number+":"+tef.length+":0:1:"+tef);
}
/*
 * Get the current text information.
 */
function doGetTXT(msg)
{
	var addr = getField(msg, 0, ',');
	var bts = getField(msg, 1, ',');

	iterateButtonStates(addr, bts, cbGetTXT, 0, false);
}
function cbGetTXT(name, button, bt, idx, par)
{
	writeTextOut("CUSTOM:"+panelID+":"+button.cp+":"+button.ch+":0:1001:"+button.sr[idx].number+":"+button.sr[idx].te.length+":0:1:"+button.sr[idx].te);
}
function doERR(msg)
{
    var emg = getField(msg, 0, ',');
    alert(emg);
    wsocket.close();
}

function doREG(msg)
{
    var stat = getField(msg, 0, ',');
    var pID = getField(msg, 1, ',');
    var regID = getField(msg, 2, ',');

    if (regID != registrationID)
        return;

    panelID = pID;

    if (stat == 'OK')
    {
        regStatus = true;
        writeTextOut("PANEL:"+panelID+":"+regID);
    }
    else
        regStatus = false;
}

function parseMessage(msg)
{
    TRACE("parseMessage: " + msg);
    var pID = splittCmd(msg);

    if (pID != panelID)
        return;

    for (var i in cmdArray.commands)
    {
        try
        {
            if (curCommand.indexOf(cmdArray.commands[i].cmd) >= 0)
            {
                cmdArray.commands[i].call(curCommand);
                break;
            }
        }
        catch (e)
        {
            errlog("parseMessage WARNING: Position: " + i + ": Error: " + e);
        }
    }
}

function calcImageSize(imWidth, imHeight, btWidth, btHeight, btFrame)
{
    var spX = btWidth - (btFrame * 2);
    var spY = btHeight - (btFrame * 2);

    if (imWidth <= spX && imHeight <= spY)
        return [imWidth + 'px', imHeight + 'px', imWidth, imHeight];

    var oversizeX = 0,
        oversizeY = 0;

    if (imWidth > spX)
        oversizeX = imWidth - spX;

    if (imHeight > spY)
        oversizeY = imHeight - spY;

    var percent = 0;
    var realX, realY;

    if (oversizeX > oversizeY)
        percent = 100 / imWidth * spX;
    else
        percent = 100 / imHeight * spY;

    realX = percent / 100 * imWidth;
    realY = percent / 100 * imHeight;
    return [percent + '%', percent + '%', realX, realY];
}

function calcImagePosition(width, height, button, cc, inst = 0)
{
    var sr, code, css, sz, border;

    if (button === null)
        return "";

    css = "position: absolute;";
    var idx = parseInt(inst);

    if (idx == 0)
        sr = button.sr[0];
    else if (idx <= button.sr.length)
        sr = button.sr[idx - 1];
    else
        return "";

    border = getBorderSize(sr.bs);

    switch (cc)
    {
        case CENTER_CODE.SC_ICON:
            code = sr.ji;
            break;
        case CENTER_CODE.SC_BITMAP:
            code = sr.jb;
            break;
        case CENTER_CODE.SC_TEXT:
            code = sr.jt;
            break;
    }

    switch (code)
    {
        case 0:
        case 1: // absolute position
            css += "left:" + sr.ix + 'px;';
            css += "top:" + sr.iy + 'px;';
            sz = calcImageSize(width, height, button.wt - sr.ix + border, button.ht - sr.iy + border, border);
            css += "width:" + sz[0] + ";";
            css += "height" + sz[1] + ";";
        break;

        case 1: // top, left
            css += "left:" + border + "px;";
            css += "top:" + border + "px;";
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            css += "width:" + sz[0] + ";";
            css += "height" + sz[1] + ";";
        break;

        case 2: // center, top
            css += "top:" + border + "px;";
            sz = calcImageSize(width, height, button.wt, button.ht, border);

            css += "left:" + ((button.wt - sz[2]) / 2) + 'px;';
            css += "width:" + sz[0] + ";";
            css += "height:" + sz[1] + ";";
        break;

        case 3: // right, top
            css += "top:" + border + "px;";
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            css += "left:" + (button.wt - sz[2]) + 'px;';
            css += "width:" + sz[0] + ';';
            css += "height:" + sz[1] + ';';
        break;

        case 4: // left, middle
            css += "left:" + border + "px;";
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            css += "width:" + sz[0] + ';';
            css += "top:" + ((button.ht - sz[3]) / 2) + 'px;';
            css += "height:" + sz[1] + ';';
        break;

        case 6: // right, middle
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            css += "left:" + (button.wt - (sz[2] + border)) + 'px;';
            css += "width:" + sz[0] + ';';
            css += "top:" + ((button.ht - sz[3]) / 2) + 'px;';
            css += "height:" + sz[1] + ';';
        break;

        case 7: // left, bottom
            css += "left:" + border + "px;";
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            css += "width:" + sz[0] + ';';
            css += "top:" + (button.ht - (sz[3] + border)) + 'px;';
            css += "height:" + sz[1] + ';';
        break;

        case 8: // center, bottom
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            css += "left:" + ((button.wt - (sz[2] + border)) / 2) + 'px;';
            css += "width:" + sz[0] + ';';
            css += "top:" + (button.ht - (sz[3] + border)) + 'px;';
            css += "height:" + sz[1] + ';';
        break;

        case 9: // right, bottom
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            css += "left:" + (button.wt - (sz[2] + border)) + 'px;';
            css += "width:" + sz[0] + ';';
            css += "top:" + (button.ht - (sz[3] + border)) + 'px;';
            css += "height:" + sz[1] + ';';
        break;

        default: // center, middle
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            css += "left:" + ((button.with - (sz[2] + border)) / 2) + 'px;';
            css += "width:" + sz[0] + ';';
            css += "top:" + ((button.ht - (sz[3] + border)) / 2) + 'px;';
            css += "height:" + sz[1] + ';';
    }

    return css;
}

function justifyImage(img, button, cc, inst = 0)
{
    var sr, code, sz, border, posx, posy;

    if (img === null || button === null)
        return;

    img.style.position = 'absolute';
    var idx = parseInt(inst);

    if (idx == 0)
        sr = button.sr[0];
    else if (idx <= button.sr.length)
        sr = button.sr[idx - 1];
    else
        return;

    border = getBorderSize(sr.bs);

    if (img.width <= 0)
        img.width = sr.bm_width;

    if (img.height <= 0)
        img.height = sr.bm_height;

    var width = img.width;
    var height = img.height;

    switch (cc) {
        case CENTER_CODE.SC_ICON:
            code = sr.ji;
            posx = sr.ix;
            posy = sr.iy;
            break;

        case CENTER_CODE.SC_BITMAP:
            code = sr.jb;
            posx = sr.ix;
            posy = sr.iy;
            break;

        case CENTER_CODE.SC_TEXT:
            code = sr.jt;
            posx = sr.tx;
            posy = sr.ty;
            break;
    }

    switch (code)
    {
        case 0: // absolute position
            sz = calcImageSize(width, height, button.wt - posx, button.ht - posy, border);
            img.style.left = posx + 'px';
            img.style.top = posy + 'px';
            img.style.width = sz[0];
            img.style.height = sz[1];
        break;

        case 1: // top, left
            img.style.left = border + 'px';
            img.style.top = border + 'px';
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            img.style.width = sz[0];
            img.style.height = sz[1];
        break;

        case 2: // center, top
            img.style.top = border + "px";
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            img.style.left = ((button.wt - (sz[2] + border)) / 2) + 'px';
            img.style.width = sz[0];
            img.style.height = sz[1];
        break;

        case 3: // right, top
            img.style.top = border + "px";
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            img.style.left = (button.wt - (sz[2] + border)) + 'px';
            img.style.width = sz[0];
            img.style.height = sz[1];
        break;

        case 4: // left, middle
            img.style.left = "0px";
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            img.style.width = sz[0];
            img.style.top = ((button.ht - (sz[3] + border)) / 2) + 'px';
            img.style.height = sz[1];
        break;

        case 6: // right, middle
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            img.style.left = (button.wt - (st[2] + border)) + 'px';
            img.style.width = sz[0];
            img.style.top = ((button.ht - (sz[3] + border)) / 2) + 'px';
            img.style.height = sz[1];
        break;

        case 7: // left, bottom
            img.style.left = border + "px";
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            img.style.width = sz[0];
            img.style.top = (button.ht - (sz[3] + border)) + 'px';
            img.style.height = sz[1];
        break;

        case 8: // center, bottom
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            img.style.left = ((button.wt - (sz[2] + border)) / 2) + 'px';
            img.style.width = sz[0];
            img.style.top = (button.ht - (sz[3] + border)) + 'px';
            img.style.height = sz[1];
        break;

        case 9: // right, bottom
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            img.style.left = (button.wt - (sz[2] + border)) + 'px';
            img.style.width = sz[0];
            img.style.top = (button.ht - (sz[3] + border)) + 'px';
            img.style.height = sz[1];
        break;

        default: // center, middle
            sz = calcImageSize(width, height, button.wt, button.ht, border);
            img.style.left = ((button.wt - (sz[2] + border)) / 2) + 'px';
            img.style.width = sz[0];
            img.style.top = ((button.ht - (sz[3] + border)) / 2) + 'px';
            img.style.height = sz[1];
    }
}

function beep()
{
    var snd = new Audio("data:audio/wav;base64,//uQRAAAAWMSLwUIYAAsYkXgoQwAEaYLWfkWgAI0wWs/ItAAAGDgYtAgAyN+QWaAAihwMWm4G8QQRDiMcCBcH3Cc+CDv/7xA4Tvh9Rz/y8QADBwMWgQAZG/ILNAARQ4GLTcDeIIIhxGOBAuD7hOfBB3/94gcJ3w+o5/5eIAIAAAVwWgQAVQ2ORaIQwEMAJiDg95G4nQL7mQVWI6GwRcfsZAcsKkJvxgxEjzFUgfHoSQ9Qq7KNwqHwuB13MA4a1q/DmBrHgPcmjiGoh//EwC5nGPEmS4RcfkVKOhJf+WOgoxJclFz3kgn//dBA+ya1GhurNn8zb//9NNutNuhz31f////9vt///z+IdAEAAAK4LQIAKobHItEIYCGAExBwe8jcToF9zIKrEdDYIuP2MgOWFSE34wYiR5iqQPj0JIeoVdlG4VD4XA67mAcNa1fhzA1jwHuTRxDUQ//iYBczjHiTJcIuPyKlHQkv/LHQUYkuSi57yQT//uggfZNajQ3Vmz+Zt//+mm3Wm3Q576v////+32///5/EOgAAADVghQAAAAA//uQZAUAB1WI0PZugAAAAAoQwAAAEk3nRd2qAAAAACiDgAAAAAAABCqEEQRLCgwpBGMlJkIz8jKhGvj4k6jzRnqasNKIeoh5gI7BJaC1A1AoNBjJgbyApVS4IDlZgDU5WUAxEKDNmmALHzZp0Fkz1FMTmGFl1FMEyodIavcCAUHDWrKAIA4aa2oCgILEBupZgHvAhEBcZ6joQBxS76AgccrFlczBvKLC0QI2cBoCFvfTDAo7eoOQInqDPBtvrDEZBNYN5xwNwxQRfw8ZQ5wQVLvO8OYU+mHvFLlDh05Mdg7BT6YrRPpCBznMB2r//xKJjyyOh+cImr2/4doscwD6neZjuZR4AgAABYAAAABy1xcdQtxYBYYZdifkUDgzzXaXn98Z0oi9ILU5mBjFANmRwlVJ3/6jYDAmxaiDG3/6xjQQCCKkRb/6kg/wW+kSJ5//rLobkLSiKmqP/0ikJuDaSaSf/6JiLYLEYnW/+kXg1WRVJL/9EmQ1YZIsv/6Qzwy5qk7/+tEU0nkls3/zIUMPKNX/6yZLf+kFgAfgGyLFAUwY//uQZAUABcd5UiNPVXAAAApAAAAAE0VZQKw9ISAAACgAAAAAVQIygIElVrFkBS+Jhi+EAuu+lKAkYUEIsmEAEoMeDmCETMvfSHTGkF5RWH7kz/ESHWPAq/kcCRhqBtMdokPdM7vil7RG98A2sc7zO6ZvTdM7pmOUAZTnJW+NXxqmd41dqJ6mLTXxrPpnV8avaIf5SvL7pndPvPpndJR9Kuu8fePvuiuhorgWjp7Mf/PRjxcFCPDkW31srioCExivv9lcwKEaHsf/7ow2Fl1T/9RkXgEhYElAoCLFtMArxwivDJJ+bR1HTKJdlEoTELCIqgEwVGSQ+hIm0NbK8WXcTEI0UPoa2NbG4y2K00JEWbZavJXkYaqo9CRHS55FcZTjKEk3NKoCYUnSQ0rWxrZbFKbKIhOKPZe1cJKzZSaQrIyULHDZmV5K4xySsDRKWOruanGtjLJXFEmwaIbDLX0hIPBUQPVFVkQkDoUNfSoDgQGKPekoxeGzA4DUvnn4bxzcZrtJyipKfPNy5w+9lnXwgqsiyHNeSVpemw4bWb9psYeq//uQZBoABQt4yMVxYAIAAAkQoAAAHvYpL5m6AAgAACXDAAAAD59jblTirQe9upFsmZbpMudy7Lz1X1DYsxOOSWpfPqNX2WqktK0DMvuGwlbNj44TleLPQ+Gsfb+GOWOKJoIrWb3cIMeeON6lz2umTqMXV8Mj30yWPpjoSa9ujK8SyeJP5y5mOW1D6hvLepeveEAEDo0mgCRClOEgANv3B9a6fikgUSu/DmAMATrGx7nng5p5iimPNZsfQLYB2sDLIkzRKZOHGAaUyDcpFBSLG9MCQALgAIgQs2YunOszLSAyQYPVC2YdGGeHD2dTdJk1pAHGAWDjnkcLKFymS3RQZTInzySoBwMG0QueC3gMsCEYxUqlrcxK6k1LQQcsmyYeQPdC2YfuGPASCBkcVMQQqpVJshui1tkXQJQV0OXGAZMXSOEEBRirXbVRQW7ugq7IM7rPWSZyDlM3IuNEkxzCOJ0ny2ThNkyRai1b6ev//3dzNGzNb//4uAvHT5sURcZCFcuKLhOFs8mLAAEAt4UWAAIABAAAAAB4qbHo0tIjVkUU//uQZAwABfSFz3ZqQAAAAAngwAAAE1HjMp2qAAAAACZDgAAAD5UkTE1UgZEUExqYynN1qZvqIOREEFmBcJQkwdxiFtw0qEOkGYfRDifBui9MQg4QAHAqWtAWHoCxu1Yf4VfWLPIM2mHDFsbQEVGwyqQoQcwnfHeIkNt9YnkiaS1oizycqJrx4KOQjahZxWbcZgztj2c49nKmkId44S71j0c8eV9yDK6uPRzx5X18eDvjvQ6yKo9ZSS6l//8elePK/Lf//IInrOF/FvDoADYAGBMGb7FtErm5MXMlmPAJQVgWta7Zx2go+8xJ0UiCb8LHHdftWyLJE0QIAIsI+UbXu67dZMjmgDGCGl1H+vpF4NSDckSIkk7Vd+sxEhBQMRU8j/12UIRhzSaUdQ+rQU5kGeFxm+hb1oh6pWWmv3uvmReDl0UnvtapVaIzo1jZbf/pD6ElLqSX+rUmOQNpJFa/r+sa4e/pBlAABoAAAAA3CUgShLdGIxsY7AUABPRrgCABdDuQ5GC7DqPQCgbbJUAoRSUj+NIEig0YfyWUho1VBBBA//uQZB4ABZx5zfMakeAAAAmwAAAAF5F3P0w9GtAAACfAAAAAwLhMDmAYWMgVEG1U0FIGCBgXBXAtfMH10000EEEEEECUBYln03TTTdNBDZopopYvrTTdNa325mImNg3TTPV9q3pmY0xoO6bv3r00y+IDGid/9aaaZTGMuj9mpu9Mpio1dXrr5HERTZSmqU36A3CumzN/9Robv/Xx4v9ijkSRSNLQhAWumap82WRSBUqXStV/YcS+XVLnSS+WLDroqArFkMEsAS+eWmrUzrO0oEmE40RlMZ5+ODIkAyKAGUwZ3mVKmcamcJnMW26MRPgUw6j+LkhyHGVGYjSUUKNpuJUQoOIAyDvEyG8S5yfK6dhZc0Tx1KI/gviKL6qvvFs1+bWtaz58uUNnryq6kt5RzOCkPWlVqVX2a/EEBUdU1KrXLf40GoiiFXK///qpoiDXrOgqDR38JB0bw7SoL+ZB9o1RCkQjQ2CBYZKd/+VJxZRRZlqSkKiws0WFxUyCwsKiMy7hUVFhIaCrNQsKkTIsLivwKKigsj8XYlwt/WKi2N4d//uQRCSAAjURNIHpMZBGYiaQPSYyAAABLAAAAAAAACWAAAAApUF/Mg+0aohSIRobBAsMlO//Kk4soosy1JSFRYWaLC4qZBYWFRGZdwqKiwkNBVmoWFSJkWFxX4FFRQWR+LsS4W/rFRb/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////VEFHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAU291bmRib3kuZGUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjAwNGh0dHA6Ly93d3cuc291bmRib3kuZGUAAAAAAAAAACU=");
    snd.play();
}

function getRegistrationID()
{
    if (regStatus == true || (registrationID !== null && typeof registrationID == "string" && registrationID.length > 0))
        return registrationID;

    try
    {
        var ID = 'T' + Math.random().toString(36).substr(2, 9);
        registrationID = ID;
    }
    catch (e)
    {
        errlog("getRegistrationID: Error: " + e);

        try
        {
            var d = new Date();

            for (var i = 0; i < d.length; i++)
            {
                var ID = "T";
                var c = d.charCodeAt(i);
                var str = Number(c).toString(16);
                ID = ID + ((str.length == 1) ? "0" + str : str);
            }

            registrationID = ID;
        }
        catch (e)
        {
            errlog("getRegistrationID: Error: " + e);
            registrationID = "";
            return registrationID;
        }
    }

    if (wsocket === null || wsocket.readyState == WebSocket.CLOSED)
    {
        connect();
        debug("getRegistrationID: regID: " + registrationID);
        return registrationID;
    }

    writeTextOut("REGISTER:" + registrationID);
    return registrationID;
}
function setOnlineStatus(stat)
{
    if (stat < 0 || stat > 11 || stat == wsStatus)
        return;

    curPort = 0;
    var bt = findButtonPort(8);

    if (bt.length == 0)
    {
        errlog('setOnlineStatus: Error button ' + addr + ' not found!');
        return;
    }

    for (var b = 0; b < bt.length; b++)
    {
        try
        {
            var name1 = 'Page_' + bt[b].pnum + '_Button_' + bt[b].bi + '_' + (wsStatus + 1);
            var name2 = 'Page_' + bt[b].pnum + '_Button_' + bt[b].bi + '_' + (stat + 1);
            document.getElementById(name1).style.display = 'none';
            document.getElementById(name2).style.display = 'inline';
            debug("setOnlineStatus: stat=" + stat + ", wsStatus=" + wsStatus);
        }
        catch (e)
        {
            errlog("setOnlineStatus: [Page_" + bt[b].pnum + "_Button_" + bt[b].bi + "_?] Error: " + e);
        }
    }

    wsStatus = stat;
}

function writeTextOut(msg)
{
    TRACE("--> " + msg);

    if (wsocket.readyState != WebSocket.OPEN)
    {
        errlog("WARNING: Socket not ready!");
        return;
    }

    wsocket.send(msg + ";");
}

(function() {
    var timestamp = new Date().getTime();

    function checkResume() {
        var current = new Date().getTime();

        if (current - timestamp > 4000)
        {
            var event = document.createEvent("Events");
            event.initEvent("resume", true, true);
            document.dispatchEvent(event);
        }

        timestamp = current;
    }

    window.setInterval(checkResume, 10000);
})();

function handleStandby()
{
    window.addEventListener('focus', function() {
        TRACE("handleStandby: Got focus");
        isBackground = false;
        isStandby = false;

        if (hdOffTimer !== null)
        {
            clearTimeout(hdOffTimer);
            hdOffTimer = null;
        }

        if (wsocket.readyState == WebSocket.CLOSED)
            connect();
    }, false);

    window.addEventListener('blur', function() {
        TRACE("handleStandby: Lost focus");
        isBackground = true;

		if (hdOffTimer === null && (isAndroid() || isIOS()))
			hdOffTimer = setTimeout(setOffline, 20000);
    }, false);

    window.addEventListener('online', function() {
        TRACE("handleStandby: We're online");
        isStandby = false;

        if (hdOffTimer !== null)
        {
            clearTimeout(hdOffTimer);
            hdOffTimer = null;
        }

        if (wsocket.readyState == WebSocket.CLOSED)
            connect();
    }, false);

    window.addEventListener('offline', function() {
        TRACE("handleStandby: We're offline");
        setOffline();
    }, false);

    window.addEventListener('resume', function() {
        TRACE("handleStandby: We resume");
        if (wsocket.readyState == WebSocket.CLOSED && !isStandby)
        {
            if (hdOffTimer !== null)
            {
                clearTimeout(hdOffTimer);
                hdOffTimer = null;
            }

            connect();
        }
    }, false);
}

function setOffline()
{
    if (wsocket.readyState == WebSocket.OPEN)
    {
        setOnlineStatus(0);
        isStandby = true;
        wsocket.close();

        if (hdOffTimer !== null)
        {
            clearTimeout(hdOffTimer);
            hdOffTimer = null;
        }
    }
}

function checkTime(i)
{
    if (i < 10)
        i = "0" + i;

    return i;
}

function setSystemBattery(hook = false)
{
    var battery = navigator.battery || navigator.webkitBattery || navigator.mozBattery || navigator.msBattery;

    if (battery === null || typeof battery == "undefined")
    {
        errlog("setSystemBattery: The browser doesn't support battery status!");
        return false;
    }

    try
    {
        var level = battery.level * 100;
        curPort = 0;
        var batStatus = findButtonPort(SYSTEMS.OBJ_BAT_LEVEL);
        var batCharge = findButtonPort(SYSTEMS.OBJ_BAT_CHARGE);

        if (!battery.charging)
        {
            for (var i in batStatus)
            {
                setBargraphLevel(batStatus[i].pnum, batStatus[i].bi, level);
                var button = getButton(batStatus[i].pnum, batStatus[i].bi);
                var name = "";

                if (button.ap == 0 && isSystemReserved(button.ad))
                    name = getSystemReservedName(button.ad);
                else
                    name = "Page_" + batStatus[i].pnum + "_Button_" + batStatus[i].bi + "_1";

                document.getElementById(name).style.display = 'inline';
                var dir = true;

                if (button.dr == "horizontal")
                    dir = false;

                drawBargraph2Graph(makeURL('image/' + button.sr[1].bm), makeURL('image/' + button.sr[0].bm), name, level, button.wt, button.ht, dir);
            }
        }
        else
        {
            for (var i in batCharge)
            {
                setBargraphLevel(batCharge[i].pnum, batCharge[i].bi, level);
                var button = getButton(batStatus[i].pnum, batStatus[i].bi);
                var name = "";

                if (button.ap == 0 && isSystemReserved(button.ad))
                    name = getSystemReservedName(button.ad);
                else
                    name = "Page_" + batStatus[i].pnum + "_Button_" + batStatus[i].bi + "_1";

                document.getElementById(name).style.display = 'inline';
                var dir = true;

                if (button.dr == "horizontal")
                    dir = false;

                drawBargraph2Graph(makeURL('image/' + button.sr[1].bm), makeURL('image/' + button.sr[0].bm), name, level, button.wt, button.ht, dir);
            }
        }

        if (battery.charging)
        {
            for (var i in batStatus)
            {
                var name = "Page_" + batStatus[i].pnum + "_Button_" + batStatus[i].bi + "_1";
                document.getElementById(name).style.display = 'none';
            }
        }
        else
        {
            for (var i in batCharge)
            {
                var name = "Page_" + batCharge[i].pnum + "_Button_" + batCharge[i].bi + "_1";
                document.getElementById(name).style.display = 'none';
            }
        }

        if (hook)
        {
            battery.onchargingchange = function() { setSystemBattery(); };
            battery.onlevelchange = function() { setSystemBattery(); };
        }
    }
    catch (e)
    {
        errlog("setSystemBattery: Browser doesn't support a battery!");
        return false;
    }

    return true;
}

function setWiFi()
{
    var connection = window.navigator.connection ||
        window.navigator.mozConnection ||
        null;

    if (connection === null)
    {
        errlog("setWiFi: WiFi API not supported!");
        return;
    }

    if ('metered' in connection) // Old style?
    {
        document.getElementById('nio-supported').classList.remove('hidden');
        [].slice.call(document.getElementsByClassName('old-api')).forEach(function(element) {
            element.classList.remove('hidden');
        });

//		var bandwidthValue = document.getElementById('b-value');
//		var meteredValue = document.getElementById('m-value');

        connection.addEventListener('change', function(event) {
            debug("Band width value: " + connection.bandwidth);
            debug("Metered value:" + (connection.metered ? '' : 'not ') + 'metered');
        }, false);

        connection.dispatchEvent(new Event('change'));
    }
    else
    {
//		var typeValue = document.getElementById('t-value');

        [].slice.call(document.getElementsByClassName('new-api')).forEach(function(element) {
            //			element.classList.remove('hidden');
        });

        connection.addEventListener('typechange', function(event) {
            debug("Connection type: " + connection.type);
        }, false);

        connection.dispatchEvent(new Event('typechange'));
    }
}

function debug(text)
{
    if (!__debug)
        return;

    console.log(text);
}

function errlog(text)
{
    if (!__errlog)
        return;

    console.log(text);
}

function TRACE(text)
{
    if (!__TRACE)
        return;

    console.log(text);
}
