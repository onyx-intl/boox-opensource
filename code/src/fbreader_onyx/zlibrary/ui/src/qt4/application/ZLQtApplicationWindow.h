/*
 * Copyright (C) 2004-2009 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef __ZLQTAPPLICATIONWINDOW_H__
#define __ZLQTAPPLICATIONWINDOW_H__

#include <map>

#include <QtGui/QMainWindow>
#include <QtGui/QAction>
#include <QtGui/QCursor>

class QDockWidget;

class ZLPopupData;

#include "../../../../core/src/desktop/application/ZLDesktopApplicationWindow.h"

class ZLQtApplicationWindow : public QMainWindow, public ZLDesktopApplicationWindow {
	Q_OBJECT

public:
	ZLQtApplicationWindow(ZLApplication *application);
	~ZLQtApplicationWindow();

	void setFocusToMainWidget();

private:
	ZLViewWidget *createViewWidget();
    void addToolbarItem(ZLToolbar::ItemPtr item) {}
	void init();
	void processAllEvents();
	void close();

    bool eventFilter(QObject *obj, QEvent *event);

	void grabAllKeys(bool grab);

	void setCaption(const std::string &caption);

	void setHyperlinkCursor(bool hyperlink);

	bool isFullscreen() const;
	void setFullscreen(bool fullscreen);

    void setToggleButtonState(const ZLToolbar::ToggleButtonItem &button){}
    void setToolbarItemState(ZLToolbar::ItemPtr item, bool visible, bool enabled){}

	void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
    bool event(QEvent *e);
	void wheelEvent(QWheelEvent *event);

private Q_SLOTS:
    void updateScreen();
    void onScreenSizeChanged(int);
    void onTaskActivated(const QStringList &);
    void onReceivedTaskCloseRequest(const QStringList &);
private:
	QDockWidget *myDocWidget;

    ZLViewWidget * view_widget_;

	bool myFullScreen;
	bool myWasMaximized;

	bool myCursorIsHyperlink;
	//QCursor myStoredCursor;
};


#endif /* __ZLQTAPPLICATIONWINDOW_H__ */
