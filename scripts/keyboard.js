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

var akbText = "";
var akpText = "";

function getAkbText()
{
	return akbText;
}

function setAkbText(text)
{
	akbText = text;
}

function getAkpText()
{
	return akpText;
}

function setAkpText(text)
{
	akpText = text;
}

function doKeyboard(message, defMessage)
{
	return prompt(message, defMessage);
}

function sendKeyboardText(text)
{
	if (text === null || typeof text != 'string')
		return;

	writeTextOut('KEY:'+panelID+':1:1:KEYB-'+text);
}

function sendKeypadText(text)
{
	if (text === null || typeof text != 'string')
		return;

	writeTextOut('KEY:'+panelID+':1:1:KEYP-'+text);
}

function passwordPrompt(pw, text)
{
	/*creates a password-prompt instead of a normal prompt*/
	/* first the styling - could be made here or in a css-file. looks very silly now but its just a proof of concept so who cares */
	var width = 200;
	var height = 100;
	var pwprompt = document.createElement("div"); 	//creates the div to be used as a prompt
	pwprompt.id= "password_prompt"; 				//gives the prompt an id - not used in my example but good for styling with css-file
	pwprompt.style.position = "fixed"; 				//make it fixed as we do not want to move it around
	pwprompt.style.left = ((window.innerWidth / 2) - (width / 2)) + "px"; 	//let it apear in the middle of the page
	pwprompt.style.top = ((window.innerWidth / 2) - (width / 2)) + "px"; 	//let it apear in the middle of the page
	pwprompt.style.border = "1px solid black"; 		//give it a border
	pwprompt.style.padding = "16px"; 				//give it some space
	pwprompt.style.background = "white"; 			//give it some background so its not transparent
	pwprompt.style.zIndex = 99999; 					//put it above everything else - just in case

	var pwtext = document.createElement("div"); 	//create the div for the password-text
	pwtext.innerHTML = text; 						//put inside the text
	pwprompt.appendChild(pwtext); 					//append the text-div to the password-prompt

	var pwinput = document.createElement("input"); 	//creates the password-input
	pwinput.id = "password_id"; 					//give it some id - not really used in this example...
	pwinput.type="password"; 						// makes the input of type password to not show plain-text

	if (pw !== null && typeof pw == "string" && pw.length > 0)
		pwinput.value = pw;							// Assign default password, if any

	pwprompt.appendChild(pwinput); 					//append it to password-prompt

	var pwokbutton = document.createElement("button"); 	//the ok button
	pwokbutton.innerHTML = "ok";

	var pwcancelb = document.createElement("button"); 	//the cancel-button
	pwcancelb.innerHTML = "cancel";
	pwprompt.appendChild(pwcancelb); 				//append cancel-button first
	pwprompt.appendChild(pwokbutton); 				//append the ok-button
	document.body.appendChild(pwprompt); 			//append the password-prompt so it gets visible
	pwinput.focus(); 								//focus on the password-input-field so user does not need to click

	/* now comes the magic: create and return a promise */
	return new Promise(function(resolve, reject)
	{
		pwprompt.addEventListener('click', function handleButtonClicks(e)	//lets handle the buttons
		{
			if (e.target.tagName !== 'BUTTON')		//nothing to do - user clicked somewhere else
				return;

		  	pwprompt.removeEventListener('click', handleButtonClicks); //removes eventhandler on cancel or ok

			if (e.target === pwokbutton)			//click on ok-button
				resolve(pwinput.value);				//return the value of the password
			else
				reject(new Error('User cancelled'));//return an error

			document.body.removeChild(pwprompt);	//as we are done clean up by removing the password-prompt
		});

		pwinput.addEventListener('keyup',function handleEnter(e) //users dont like to click on buttons
		{
			if(e.keyCode == 13) 					//if user enters "enter"-key on password-field
			{
				resolve(pwinput.value); 			//return password-value
				document.body.removeChild(pwprompt);//clean up by removing the password-prompt
			}
			else if (e.keyCode == 27)				//user enters "Escape" on password-field
			{
				document.body.removeChild(pwprompt);//clean up the password-prompt
				reject(new Error("User cancelled"));//return an error
			}
		});
	});
}
