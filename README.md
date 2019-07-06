# amxpanel
This software is an emulator for any G4 AMX panel (https://www.amx.com).
In case you don't know what AMX is, you can stop reading.

The software consists of two parts. A daemon written for Linux or any other Unix
operating system, and a couple of javascript files. All the emulation is done in
javascript, while the daemon on a server makes the connection to an AMX controller.
The daemon on a server connects to an AMX controller and identifies itself as a
legal AMX panel. Than it opens a websocket channel and waits for the connection of
a normal browser.

To make all work you need the following stuff:

* A server to run the daemon (currently Linux or MacOS is supported)
* A WEB server
* A browser (chrome works best)

The daemon is written in C++. There are cmake makefiles. To compile the daemon,
you've to create a directory named **build**. Enter this directory and type:

    cmake ..
    make
    make install

After the daemon was installed, you need to create a config file. For details about this,
look at the Wiki. Then you can start the daemon.

Copy the files of the panel into a directory of your WEB server. You can create a
surface with the application **TPDesign4** from AMX (G5 panels are currently not
supported!). To get the files, install **G4 Panelpreview**. There will be a menu
point in TPDesign (Panel->Send to G4 Panelpreview). Select this. While the Panelpreview
is running, you'll find the unpacked files in a temporary directory. Copy all the files
there into a directory of your WEB server. That's it!

Currently only a couple of panel commands are supported. This is what works until now:

* Buttons
* Bargraphs
* Colors
* Images
* Chameleon images
* Animated images (multistate buttons)
* Keyboard commands
* Page commands
* Popup pages
* Simple frames around buttons
* Display text

Dynamic images are broken. Currently the password of a dynamic image can't be
decrypted and the authentication to a WEB server doesn't work.

In the file **amxpanel.js** you'll find a table on the beginning. There you can
see which commands are supported, and which not.

For more details on compiling and configuration look at the Wiki.

