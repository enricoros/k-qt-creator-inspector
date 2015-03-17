# Inspector Plugin

This repository contains the sourcecode (in the inspector-plugin branch) of the Inspector Plugin for QtCreator. **click here!** ->

The Inspector Plugin adds the following functionality to QtCreator:

 * Inspection Mode (Ctrl+6) - this allows to inspect a Target (either a new run of the code you're working, or any Qt executable already running on your system)

 * slightly changes the source code of other plugins for better integration

## Quick Manual Building instructions

Requirements:

 * system Qt >= 4.6 (I'm testing with 4.6.2 right now)
 * [optional] VTK development libraries 5.4 or later (tested with 5.4.2)

Checking out the sourcecode:

 * the "git clone.." stuff as written below

How to build & run:

 * export QTCREATOR_WITH_INSPECTOR=yes
 * export QTCREATOR_WITH_INSPECTOR_VTK=yes
 * qmake -r
 * make -j2
 * bin/qtcreator.app

## Gentoo Building

Inspector can be found in Gentoo's _qting-edge_ portage overlay! To install it, follow these steps:

 * emerge layman
 * layman -a qting-edge
 * echo "dev-util/qt-creator **" >> /etc/portage/package.keywords
 * USE="inspector" emerge qt-creator

## Usage

It is really important that you use the *modified debugging helpers* that come with the sourcecode. If they don't build from the QtCreator options menu, build them by hand inside their directory: cd share/qtcreator/gdbmacros; qmake; make; and then point creator to the library you just build (from the options menu > debugger > debugging helper).

## Feedback...

... is much appreciated!

 * please tell me if your build succeeded or failed and on which platform (linux, max, win) and with which compiler.
 * please tell me if you're able to attach to the target and perform some basic tests like the temperature one.
