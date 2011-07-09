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

#include "onyx/screen/screen_update_watcher.h"
#include "onyx/sys/sys_status.h"
#include "cancelWindow.h"

///<summary>Constructor</summary>
///<param name="parent">The parent window owning this widget. This is also used for centering this widget.</param>
///<param name="message">Reference to a QString instance containing the message to display</param>
CCancelWindow::CCancelWindow(QWidget* parent, const QString &message) :
  QLabel(message, parent, Qt::WindowStaysOnTopHint)
{
  Font.setBold(true);
  Font.setPointSize(18);

  setAutoFillBackground(true);
  setBackgroundRole(QPalette::Base);
  setFont(Font);

  setFixedWidth(500);
  setAlignment(Qt::AlignCenter);
}

///<summary>Destructor</summary>
CCancelWindow::~CCancelWindow()
{
}

///<summary>Handels the show event to center the window within the parent widget</summary>
///<param name="event">Pointer to a QShowEvent instance defining the details for this event.</param>
void CCancelWindow::showEvent(QShowEvent * event)
{
  move((((QWidget*)parent())->width()-this->width())/2, (((QWidget*)parent())->height()-this->height())/2);
  
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC, false, onyx::screen::ScreenCommand::WAIT_ALL);
}
