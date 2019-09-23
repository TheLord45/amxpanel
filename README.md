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

* A server to run the daemon (currently Linux is supported)
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
supported!). To get the files, download the source of **FSFReader** from https://github.com/TheLord45/fsfreader.git.

    git clone https://github.com/TheLord45/fsfreader.git

You **FSFReader** is able to read a TPDesign4 file and extract the contents into
a directory. Then move all the files with the extensions `.xma` and `.xml` into the main directory
of the directory of your WEB server. In this directory create the subdirectories `images`,
`sounds` and `fonts`. Move all graphic files with the extensions `.png` and `.jpg` into
the subdirectory `images`.  The sound files with the extenstions `.mp3` and `.wav` into
the subdirectory `sounds` and the fonts with the extensions `.ttf` into the directory
`fonts`. That's it!
Currently the filetransfer is implemented but still marked _experimental_. You can try to
transfer the surface directly with **TPDesign4** as you would with a real panel.

Currently only a couple of panel commands are supported. This is what works until now:

* Buttons
* Bargraphs
* Colors
* Images, bitmaps, icons
* Chameleon images
* Animated images (multistate buttons)
* Keyboard commands
* Page commands
* Popup pages
* Simple frames around buttons
* Display text
* Custom messages

Dynamic images are broken. Currently the password of a dynamic image can't be
decrypted and the authentication to a WEB server doesn't work.

In the file **amxpanel.js** you'll find a table on the beginning. There you can
see which commands are supported, and which not.

For more details on compiling and configuration look at the Wiki.

