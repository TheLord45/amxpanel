const BROWSER = Object.freeze({
    BR_UNDEFINED:   0,
    BR_OPERA:       1,
    BR_CHROME:      2,
    BR_SAFARI:      3,
    BR_FIREFOX:     4,
    BR_IE:          5
});

const OPSYS = Object.freeze({
    OS_UNDEFINED:   0,
    OS_LINUX:       1,
    OS_UNIX:        2,
    OS_MAC:         3,
    OS_WINDOWS:     4,
    OS_ANDROID:     5,
    OS_IOS:         6
});

var BR_TYPE = BROWSER.BR_UNDEFINED;
var OSTYPE = OPSYS.OS_UNDEFINED;

function __detectBrowser() 
{
	if((navigator.userAgent.indexOf("Opera") || navigator.userAgent.indexOf('OPR')) != -1 )
		BR_TYPE = BROWSER.BR_OPERA;
	else if(navigator.userAgent.indexOf("Chrome") != -1 )
		BR_TYPE = BROWSER.BR_CHROME;
	else if(navigator.userAgent.indexOf("Safari") != -1)
		BR_TYPE = BROWSER.BR_SAFARI;
	else if(navigator.userAgent.indexOf("Firefox") != -1 )
        BR_TYPE = BROWSER.BR_FIREFOX;
	else if((navigator.userAgent.indexOf("MSIE") != -1 ) || (!!document.documentMode == true )) //IF IE > 10
		BR_TYPE = BROWSER.BR_IE
	else
        BR_TYPE = BROWSER.BR_UNDEFINED;
    
    if (navigator.userAgent.indexOf("Android"))
        OSTYPE = OPSYS.OS_ANDROID;
    else if (navigator.userAgent.indexOf("Linux"))
        OSTYPE = OPSYS.OS_LINUX;
    else if (navigator.userAgent.indexOf("macOS") || navigator.userAgent.indexOf("OSX"))
        OSTYPE = OPSYS.OS_MAC;
    else if (navigator.userAgent.indexOf("iOS"))
        OSTYPE = OPSYS.OS_IOS;
    else if (navigator.userAgent.indexOf("UNIX"))
        OSTYPE = OPSYS.OS_UNIX;
    else if (navigator.userAgent.indexOf("Windows"))
        OSTYPE = OPSYS.OS_WINDOWS;
    else
        OSTYPE = OPSYS.OS_UNDEFINED;
}

function isOpera()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (BR_TYPE == BROWSER.BR_OPERA)
        return true;

    return false;
}
function isChrome()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (BR_TYPE == BROWSER.BR_OPERA)
        return true;

    return false;
}
function isSafari()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (BR_TYPE == BROWSER.BR_OPERA)
        return true;

    return false;
}
function isFirefox()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (BR_TYPE == BROWSER.BR_OPERA)
        return true;

    return false;
}
function isIE()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (BR_TYPE == BROWSER.BR_OPERA)
        return true;

    return false;
}

function isLinux()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (OSTYPE == OPSYS.OS_LINUX)
        return true;

    return false;
}
function isMacOS()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (OSTYPE == OPSYS.OS_MAC)
        return true;

    return false;
}
function isUnix()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (OSTYPE == OPSYS.OS_UNIX)
        return true;

    return false;
}
function isIOS()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (OSTYPE == OPSYS.OS_IOS)
        return true;

    return false;
}
function isAndroid()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (OSTYPE == OPSYS.OS_ANDROID)
        return true;

    return false;
}
function isWindows()
{
    if (BR_TYPE == BROWSER.BR_UNDEFINED && OSTYPE == OPSYS.OS_UNDEFINED)
        __detectBrowser();

    if (OSTYPE == OPSYS.OS_WINDOWS)
        return true;

    return false;
}
