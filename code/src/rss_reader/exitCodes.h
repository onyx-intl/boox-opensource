/*	NewsFlash
		Copyright 2010 Daniel Goﬂ (Flash Systems)

		This file is part of NewsFlash

    NewsFlash is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NewsFlash is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with NewsFlash.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma one

//Sync CSyncHttp and CRSSLoaderThread use the exit command to leave a temporary
//message loop. This message loop is used to make an ansynchronous process
//synchronous to ease programming. Therefore this header file defines different
//exit codes so a warning ca be issued if an exit-statement with the wrong Id is
//encountered.

#define EXIT_NEWLISTITEM	1
#define EXIT_SYNCHTTP			2
