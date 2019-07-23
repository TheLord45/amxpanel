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
        { "cmd": "@APG-", "call": doAPG }, // Add a popup to a popup group
        { "cmd": "@CPG-", "call": doCPG }, // Clear all popups from a group
        { "cmd": "@DPG-", "call": doDPG }, // Delete a specific popup page from specified popup group if it exists.
        { "cmd": "@PDR-", "call": unsupported },
        { "cmd": "@PHE-", "call": unsupported },
        { "cmd": "@PHP-", "call": unsupported },
        { "cmd": "@PHT-", "call": unsupported },
        { "cmd": "@PPA-", "call": unsupported },
        { "cmd": "@PPF-", "call": doPPF }, // Popup off
        { "cmd": "@PPG-", "call": doPPG }, // Toggle a popup
        { "cmd": "@PPK-", "call": doPPK }, // Close popup on all pages
        { "cmd": "@PPM-", "call": unsupported },
        { "cmd": "@PPN-", "call": doPPN }, // Popup on
        { "cmd": "@PPT-", "call": unsupported },
        { "cmd": "@PPX", "call": doPPX }, // close all popups on all pages
        { "cmd": "@PSE-", "call": unsupported },
        { "cmd": "@PSP-", "call": unsupported },
        { "cmd": "@PST-", "call": unsupported },
        { "cmd": "PAGE-", "call": doPAGE }, // Flip to page
        { "cmd": "PPOF-", "call": doPPF }, // Popup off
        { "cmd": "PPOG-", "call": unsupported },
        { "cmd": "PPON-", "call": doPPN }, // Popup on
        { "cmd": "^ANI-", "call": doANI }, // Run a button animation        
        { "cmd": "^APF-", "call": doAPF }, // Add page flip action to button
        { "cmd": "^BAT-", "call": doBAT }, // Append non-unicode text.
        { "cmd": "^BAU-", "call": doBAU }, // Append unicode text
        { "cmd": "^BCB-", "call": doBCB }, // Set the border color to the specified color.
        { "cmd": "^BCF-", "call": doBCF }, // Set the fill color to the specified color.
        { "cmd": "^BCT-", "call": doBCT }, // Set the text color to the specified color.
        { "cmd": "^BDO-", "call": unsupported }, // Set the button draw order.
        { "cmd": "^BFB-", "call": doBFB }, // Set the feedback type of the button.
        { "cmd": "^BIM-", "call": unsupported }, // Set the input mask for the specified address.
        { "cmd": "^BLN-", "call": unsupported }, // Set the number of lines removed equally from the top and bottom of a composite video signal. --> will never be implemented!
        { "cmd": "^BMC-", "call": unsupported }, // Button copy command.
        { "cmd": "^BMF-", "call": doBMF }, // Set any/all button parameters by sending embedded codes and data.
        { "cmd": "^BMI-", "call": doBMI }, // Set the button mask image.
        { "cmd": "^BML-", "call": unsupported }, // Set the maximum length of the text area button.
        { "cmd": "^BMP-", "call": doBMP }, // Assign a picture to those buttons with a defined addressrange.
        { "cmd": "^BNC-", "call": unsupported },
        { "cmd": "^BNN-", "call": unsupported },
        { "cmd": "^BNT-", "call": unsupported },
        { "cmd": "^BOP-", "call": doBOP }, // Set the button opacity.        
        { "cmd": "^BOR-", "call": doBOR }, // Set a border to a specific border style.
        { "cmd": "^BOS-", "call": unsupported },
        { "cmd": "^BPP-", "call": unsupported },
        { "cmd": "^BRD-", "call": doBRD },
        { "cmd": "^BSF-", "call": unsupported },
        { "cmd": "^BSP-", "call": doBSP }, // Set the button size and position.
        { "cmd": "^BSM-", "call": unsupported },
        { "cmd": "^BSO-", "call": unsupported },
        { "cmd": "^BVL-", "call": unsupported },
        { "cmd": "^BVN-", "call": unsupported },
        { "cmd": "^BVP-", "call": unsupported },
        { "cmd": "^BVT-", "call": unsupported },
        { "cmd": "^BWW-", "call": unsupported },
        { "cmd": "^CPF-", "call": doCPF }, // Clear all page flips from a button.
        { "cmd": "^DLD-", "call": unsupported },
        { "cmd": "^DPF-", "call": unsupported },
        { "cmd": "^ENA-", "call": doENA }, // Enable or disable buttons with a set variable text range.
        { "cmd": "^FON-", "call": doFON },
        { "cmd": "^GDI-", "call": unsupported },
        { "cmd": "^GIV-", "call": unsupported },
        { "cmd": "^GLH-", "call": unsupported },
        { "cmd": "^GLL-", "call": unsupported },
        { "cmd": "^GRD-", "call": unsupported },
        { "cmd": "^GRU-", "call": unsupported },
        { "cmd": "^GSC-", "call": unsupported },
        { "cmd": "^GSN-", "call": unsupported },
        { "cmd": "^ICO-", "call": doICO }, // Set the icon to a button.
        { "cmd": "^IRM-", "call": unsupported },
        { "cmd": "^JSB-", "call": doJSB },
        { "cmd": "^JSI-", "call": doJSI },
        { "cmd": "^JST-", "call": doJST },
        { "cmd": "^MBT-", "call": unsupported },
        { "cmd": "^MDC-", "call": unsupported },
        { "cmd": "^SHO-", "call": doSHO },
        { "cmd": "^TEC-", "call": unsupported },
        { "cmd": "^TEF-", "call": unsupported },
        { "cmd": "^TOP-", "call": unsupported },
        { "cmd": "^TXT-", "call": doTXT },
        { "cmd": "^UNI-", "call": unsupported },
        { "cmd": "^LPC-", "call": unsupported },
        { "cmd": "^LPR-", "call": unsupported },
        { "cmd": "^LPS-", "call": unsupported },
        { "cmd": "?BCP-", "call": unsupported },
        { "cmd": "?BCF-", "call": unsupported },
        { "cmd": "?BCT-", "call": unsupported },
        { "cmd": "?BMP-", "call": unsupported },
        { "cmd": "?BOP-", "call": unsupported },
        { "cmd": "?BRD-", "call": unsupported },
        { "cmd": "?BWW-", "call": unsupported },
        { "cmd": "?FON-", "call": unsupported },
        { "cmd": "?ICO-", "call": unsupported },
        { "cmd": "?JSB-", "call": unsupported },
        { "cmd": "?JSI-", "call": unsupported },
        { "cmd": "?JST-", "call": unsupported },
        { "cmd": "?TEC-", "call": unsupported },
        { "cmd": "?TEF-", "call": unsupported },
        { "cmd": "?TXT-", "call": unsupported },
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
        { "cmd": "@SOU-", "call": unsupported },
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

function iterateButtonStates(addr, bts, callback, pars)
{
    var addrRange = getRange(addr);
    var btRange = getRange(bts);

    for (var i = 0; i < addrRange.length; i++)
    {
        var bt = findButtonPort(addrRange[i]);

        if (bt.length == 0)
        {
            errlog('iterateButtons: Error button ' + addrRange[i] + ' not found!');
            continue;
        }

        for (var b = 0; b < bt.length; b++)
        {
            for (var z = 1; z <= bt[b].instances; z++)
            {
                for (var j = 0; j < btRange.length; j++)
                {
                    if ((btRange.length == 1 && btRange[0] == 0) || btRange[j] == z)
                    {
                        var name = 'Page_' + bt[b].pnum + "_Button_" + bt[b].bi + "_" + z;
                        var button = getButton(bt[b].pnum, bt[b].bi);
                        var idx = 0;

                        for (var i in button.sr)
                        {
                            if (button.sr[i].number == z)
                                idx = parseInt(i);
                        }

                        callback(name, button, bt[b], idx, pars);
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
    var i;

    for (i in bargraphs.bargraphs)
    {
        var bg = bargraphs.bargraphs[i];

        if (bg.pnum == pnum && bg.bi == id)
            return bg.level;
    }

    return 0;
}

function getBargraphPC(pnum, id)
{
    var i;

    for (i in bargraphs.bargraphs)
    {
        var bg = bargraphs.bargraphs[i];

        if (bg.pnum == pnum && bg.bi == id)
            return [bg.lp, bg.lv];
    }

    return -1;
}

function getBargraphPars(pnum, id)
{
    var i;

    for (i in bargraphs.bargraphs)
    {
        var bg = bargraphs.bargraphs[i];

        if (bg.pnum == pnum && bg.bi == id)
            return bg;
    }

    return -1;
}

function setBargraphLevel(pnum, id, level)
{
    var i;
    var PC = getBargraphPC(pnum, id);

    if (PC === -1)
        return;

    for (i in bargraphs.bargraphs)
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
    var rest;
    var pos;
    var i;
    var bStr = false;
    var part = "";

    pos = msg.indexOf('-'); // Check for the command part preceding the parameters

    if (pos >= 0)
        rest = msg.substr(pos + 1); // Cut off the command part
    else
        rest = msg; // No command part, so take the whole string

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

function hideGroup(name)
{
    var nm;
    var group;
    var i;
    group = popupGroups[name];

    if (group === null || typeof group == "undefined" || name === null || typeof name == "undefined" || name.length == 0)
    {
        errlog("hideGroup: Invalid group name '" + name + "'!");
        return;
    }

    for (i in group)
    {
        var pg;
        pg = findPopupNumber(group[i]);
        nm = 'Page_' + pg;

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
            Popups.pages[idx].lnpage = getActivePageName();
        }
    }
    catch (e)
    {
        errlog('showPopup: Error on name <' + name + '> and page ' + pname + ': ' + e);
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
    pname = "Page_" + pID;
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
    catch (e)
    {
        errlog('showPopupOnPage: Error on name <' + name + '> and page ' + pname + ': ' + e);
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
    pname = "Page_" + pID;
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
    catch (e)
    {
        errlog('hidePopupOnPage: Error on name <' + name + '> and page ' + pname + ': ' + e);
    }
}

function hidePopup(name)
{
    var pname;
    var pID;
    var idx;

    pID = findPopupNumber(name);
    pname = "Page_" + pID;

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
    catch (e)
    {
        errlog('hidePopup: Error on name <' + name + '> and page ' + pname + ': ' + e);
    }
}

function showPage(name)
{
    var pname;
    var pID;

    pID = findPageNumber(name);

    if (pID > 0)
        pname = "Page_" + pID;
    else
        pname = name;

    try
    {
        var ID;
        ID = getActivePage();

        dropPage();
        drawPage(name);

        for (var i in Popups.pages)
        {
            pname = "Page_" + Popups.pages[i].ID;

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
    catch (e)
    {
        errlog('showPage: Error on name <' + name + '> and page ' + pname + ': ' + e);
    }
}

function hidePage(name)
{
    var pname;
    var pID;

    pID = findPageNumber(name);

    if (pID >= 0)
        pname = "Page_" + pID;
    else
        pname = name;

    try
    {
        dropPage();

        for (var i in Popups.pages)
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
    catch (e)
    {
        errlog('hidePage: Error on name <' + name + '> and page ' + pname + ': ' + e);
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
 * Attention: The animation is currently sot implemented. This works
 * the same way like doPPN()!
 */
function doAFP(msg)
{
    var pg;
    var animation;
    var origin;
    var time;

    pg = getField(msg, 0, ',');
    animation = getField(msg, 1, ',');
    origin = getField(msg, 2, ',');
    time = getField(msg, 3, ',');

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
    var pos;
    var name;
    var pg;

    pg = "";
    pos = msg.indexOf(";"); // Do we have a page name?
    // FIXME: Page names are not supported currently!
    if (pos < 0)
        pos = msg.length - 5;
    else {
        pg = msg.substr(pos + 1);
        pos = msg.length - pos - 5;
    }

    name = msg.substr(5, pos); // Extract the popup name

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
    var pos;
    var name;

    pos = msg.indexOf(";"); // Do we have a page name?
    // FIXME: Page names are not supported currently!
    if (pos < 0)
        pos = msg.length;

    name = msg.substr(5, pos); // Extract the popup name

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
    var name;
    var group;

    name = msg.substr(5); // Extract the popup name
    group = findPageGroup(name);

    if (group.length > 0)
        hideGroup(group);
    else
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
 * Flip to a specified page.
 */
function doPAGE(msg)
{
    var name;

    name = msg.substr(5);
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

    iterateButtonStates(addr, bts, cbBau, unescape(encodeURIComponent(utext)));
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
function cbBBR(name, button, bt, idx, par)
{
    button.sr[idx].sb = 1;
    button.sr[idx].bm = name;
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
 * Set the feedbacktype of the button.
 */
function doBFB(msg)
{
    var addr = getField(msg, 0, ',');
    var fb = getField(msg, 1, ',');

    addrRange = getRange(addr);

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
    var sr = button.sr[idx];
    sr.mi = img;

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
    if (button !== null)
    {
        button.sr[idx].bm = img;

        if (button.sr[idx].bm_width == 0)
            button.sr[idx].bm_width = button.wt;

        if (button.sr[idx].bm_height == 0)
            button.sr[idx].bm_height = button.ht;
    }

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
    var sr = button.sr[idx];
    sr.fi = fID;

    try
    {
        var font = findFont(fID);
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
            justifyImage(img, getButton(bt.pnum, bt.bi), CENTER_CODE.SC_ICON, button.sr[i].number);
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
                justifyImage(img, getButton(bt.pnum, bt.bi), CENTER_CODE.SC_ICON, button.sr[i].number);
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

    var regID = store.get("regID");

    if (regID == null || regID.length == 0)
    {
        try
        {
            var ID = 'T' + Math.random().toString(36).substr(2, 9);
            store.set("regID", ID);
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
                    var ID = "";
                    var c = d.charCodeAt(i);
                    var str = Number(c).toString(16);
                    ID = ID + ((str.length == 1) ? "0" + str : str);
                }

                store.set("regID", ID);
                registrationID = ID;
            }
            catch (e)
            {
                errlog("getRegistrationID: Error: " + e);
                registrationID = "";
                return registrationID;
            }
        }
    }
    else
        registrationID = regID;

    if (wsocket === null || wsocket.readyState == WebSocket.CLOSED)
    {
        connect();
        debug("getRegistrationID: regID: " + registrationID);
        return registrationID;
    }

    writeTextOut("REGISTER:" + registrationID);
    return registrationID;
}
/*
function onInitFs(name, root)
{
    var ID = 'T' + Math.random().toString(36).substr(2, 9);
    debug("onInitFs: Storage: " + name + " / " + root + ", regID: " + ID);

    try
    {
        window.localStorage.setItem("regID", ID);
        registrationID = ID;
    }
    catch (e)
    {
        errlog("onInitFs: Error: " + e);
        registrationID = "";
        return;
    }

    debug("onInitFs: " + registrationID);
    writeTextOut("REGISTER:" + registrationID);
}

function errorHandler(err)
{
    errlog("errorHandler: Error: " + err);
    registrationID = "";
}
*/
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
