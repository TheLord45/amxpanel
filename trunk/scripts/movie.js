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
'use strict';

 var refreshing = {"ref":[]};

function findRessource(name)
{
	for (var i in ressources.ressources)
	{
		var ressource = ressources.ressources[i];

		for (var j in ressource.ressource)
		{
			var res = ressource.ressource[j];

			if (res.name == name)
				return res;
		}
	}

	return null;
}
function getRessourceURL(name)
{
	var res = findRessource(name);

	if (res === null)
		return "";

	var url = res.protocol.toLowerCase() + "://"+res.host+"/";

	if (res.path.length > 0)
		url = url+res.path+"/";

	if (res.file.length > 0)
		url = url+res.file;

	return url;
}
function addRessource(name, protocol="HTTP", host="", path="", file="", user="", pass="", refresh=0)
{
	var res = {"name":name, "protocol":protocol, "host":host, "path":path, "file":file, "user":user, "password":pass, "refresh":refresh, "encrypted":false};

	if (findRessource(name) !== null)
	{
		errlog("addRessource: Resource "+name+" already exists!");
		return;
	}

	ressources.ressources[0].ressource.push(res);
}
function updateResource(res)
{
	if (res === null || typeof res != "object")
	{
		errlog("updateResource: Invalid or no object found!");
		return;
	}

	try
	{
		for (var i in ressources.ressources)
		{
			var ressource = ressources.ressources[i];

			for (var j in ressource.ressource)
			{
				var r = ressource.ressource[j];

				if (r.name == res.name)
				{
					if (res.protocol.length > 0)
						r.protocol = res.protocol;

					if (res.user.length > 0)
						r.user = res.user;

					if (res.password.length > 0)
					{
						r.password = res.password;
						r.encrypted = res.encrypted;
					}

					if (res.host.length > 0)
						r.host = res.host;

					if (res.path.length > 0)
						r.path = res.path;

					if (res.file.length > 0)
						r.file = res.file;

					r.refresh = res.refresh;
				}
			}
		}
	}
	catch(e)
	{
		errlog("updateResource: Error: "+e);
	}
}
function refreshResource(name)
{
	if (name === null || typeof name != "string" || name.length == 0)
		return;

	var uri = getRessourceURL(name);
	debug ("refreshResource: Trying to get URI: "+uri);

	if (uri.length == 0)
	{
		errlog("refreshResource: Invalid url for resource '"+name+"'.");
		return;
	}

	// Find all buttons using this resource
	for (var i in buttonArray.buttons)
	{
		var bt = buttonArray.buttons[i];
		var button = getButton(bt.pnum, bt.bi);

		if (button === null)
			continue;

		for (var j in button.sr)
		{
			var sr = button.sr[j];

			if (sr.sb == 1 && sr.bm == name)
			{
				var nm = "";

				if (button.ap == 0 && isSystemReserved(button.ad))
					nm = getSystemReservedName(button.ad)+sr.number;
				else
					nm = "Page_"+bt.pnum+"_Button_"+bt.bi+"_"+sr.number

				try
				{
					var div = document.getElementById(nm);

					if (uri.indexOf("random=") < 0)
					{
						if (uri.indexOf('?') >= 0 || uri.indexOf("%3f") >= 0)
							uri = uri + "&random="+Math.random();
						else
							uri = uri + "?random="+Math.random();
					}

                    uri = uri.replace("?", "%3f");
                    uri = uri.replace("&", "%26");
                    var proxy = makeURL("scripts/proxy.php?csurl="+uri);
					debug("refreshResource: Image URI: "+proxy);
					div.style.backgroundImage = "url('"+proxy+"')";
					div.style.backgroundRepeat = "no-repeat";

					switch (sr.jb)
					{
						case 0:
							div.style.backgroundPositionX = sr.ix+'px';
							div.style.backgroundPositionY = sr.iy+'px';
						break;

						case 1: div.style.backgroundPosition = "left top"; break;
						case 2: div.style.backgroundPosition = "center top"; break;
						case 3: div.style.backgroundPosition = "right top"; break;
						case 4: div.style.backgroundPosition = "left center"; break;
						case 6: div.style.backgroundPosition = "right center"; break;
						case 7: div.style.backgroundPosition = "left bottom"; break;
						case 8: div.style.backgroundPosition = "center bottom"; break;
						case 9: div.style.backgroundPosition = "right bottom"; break;
						default:
							div.style.backgroundPosition = "center center";
					}
				}
				catch(e)
				{
					errlog("refreshResource: Error on image "+name+" at button "+nm+" with URL: "+uri);
					errlog("refreshResource: Error: "+e);
					// If this is a refreshing image, try to stop the interval
					for (var x in refreshing.ref)
					{
						if (refreshing.ref[x].name == name)
						{
							clearInterval(refreshing.ref[x].handle);
							delete refreshing.ref[x];
							break;
						}
					}
				}
			}
		}
	}
}
function drawButtonRemote(button, nm, idx)
{
	if (button === null || typeof button != "object" || nm === null || nm.length == 0)
	{
		errlog("drawButtonRemote: Invalid parameter was given to this function!");
		return;
	}

	var sr = button.sr[idx];

	if (sr.sb != 1)
	{
		errlog("drawButtonRemote: Button "+nm+" is not a remote image!");
		return;
	}

	var res = findRessource(sr.bm);

	if (res === null)
	{
		errlog("drawButtonRemote: Could not find resource "+sr.bm);
		return;
	}

	if (!sr.dynamic && res.refresh > 0)		// periodicaly refreshig image?
	{
		debug("drawButtonRemote: Refreshing image "+sr.bm+" every "+res.refresh+" seconds.");

		for (var i in refreshing.ref)
		{
			if (refreshing.ref[i].name == sr.bm)
			{
				debug("drawButtonRemote: Image "+sr.bm+" is already refreshing.");
				return;
			}
		}

		var handle = setInterval(refreshResource.bind(null, res.name), res.refresh * 1000);
		var r = {"name":sr.bm,"handle":handle};
		refreshing.ref.push(r);
	}
	else if (!sr.dynamic)
		refreshResource(res.name);
	else
		errlog("drawButtonRemote: Dynamic display of moving pictures is currently not implemented!");
}
