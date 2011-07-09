Sourcecode Readme for NewsFlash
같같같같같같같같같같같같같같같�

Revision: 64

This is the developer howto for NewsFlash. It is intended to give you a rough starting point. I've you can't get NewsFlash to compile, simply use the binary version from www.flashystems.de


Installing Qt build envrionment for windows
===========================================

This chapter shows you a way to install all neccessary tools for QT development for the boox on windows. This makes it possible to create and test your apps in windows and then crosscompile with little (or even no) changes on linux.

To be able to do this, some things have to be done:
1. Install QT 4.5.2
   We use this version as it's the same as the one on the boox.
   This way we can't use methods not supported by the QT version
   on the boox.
2. Install CMake
   Onyx uses CMake to generate the platform dependend makefiles.
   It really is a great too. This tutorial provides you with a
   modified CMakeCache.txt that automatically switches between
   building for Win32 and for ARM.
3. Modify qtvars within the Qt 4.5.2 directory
   Add the CMake bin directory to the PATH additons in qtvars.bat.


Compiling on windows
====================

To compile NewsFlash in Windows first open a Command Prompt via the "Qt 4.5.2 Command Prompt" link. Change into the directory containing the NewsFlash source and type:
cmake -DMINGW:BOOL=ON -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles"
This creates the neccessary files. Then you can use make to build the application.


Testing the program on windows
==============================

The program will be compiled to the bin directory. Put a NewsFlash.ini file (the format is described within readme.txt) in this directory and run it from a Qt command prompt opened via the "Qt 4.5.2 Command Prompt" link.


Compiling on linux
==================

To compile this application on linux install the onyx sdk and toolchain. Both can be downloaded from http://dev.onyxcommunity.com/sdk/. NewsFlash was build with the SDK released at 25.03.2010.

For details on how to install the SDK see http://booxusers.com/viewtopic.php?f=15&t=90.

After the SDK is ready and you executed onyx_sdk_setup.sh (don't forget to use ". onyx_sdk_setup.sh" to apply the environment changes to the current shell) change into the directory where you extracted the source and type:
cmake -DBUILD_FOR_ARM:BOOL=ON -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF

After that you can use make to build the .oar file for your boox.


Downloads
=========

ftp://ftp.qt.nokia.com/pub/qt/source/qt-win-opensource-4.5.2-mingw.exe
http://www.cmake.org/files/v2.8/cmake-2.8.0-win32-x86.zip
http://prdownloads.sf.net/mingw/gdb-6.3-2.exe


Contact
=======

To contact the developer Daniel Goss use developer@flashsystems.de.
