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

#pragma once

#include <QtGui>

///<summary>Provides a window that is centered within the parent window and show a message to the user.</summary>
class CCancelWindow : public QLabel
{
	Q_OBJECT
	
//-- Private methods
private:
  QFont Font;
	
//-- Protected overriedes
protected:
	void showEvent(QShowEvent * event);
  
//-- c'tor and d'tor
public:
  CCancelWindow(QWidget* parent, const QString &message);
  ~CCancelWindow();
};
