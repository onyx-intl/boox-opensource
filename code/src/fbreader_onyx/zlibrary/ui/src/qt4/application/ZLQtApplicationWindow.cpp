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

#include <QtGui/QApplication>
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtGui/QIcon>
#include <QtGui/QToolBar>
#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QToolButton>
#include <QtGui/QLayout>
#include <QtGui/QWheelEvent>
#include <QtGui/QDockWidget>
#include <QtCore/QObjectList>

#include <ZLibrary.h>
#include <ZLPopupData.h>

#include "ZLQtApplicationWindow.h"
#include "../dialogs/ZLQtDialogManager.h"
#include "../view/ZLQtViewWidget.h"
#include "../util/ZLQtKeyUtil.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/platform.h"

static bool support16GrayScale()
{
    static int gray = 0;
    if (gray <= 0)
    {
        gray = sys::SysStatus::instance().grayScale();
    }
    return (gray == 16);
}

void ZLQtDialogManager::createApplicationWindow(ZLApplication *application) const {
	new ZLQtApplicationWindow(application);
}

ZLQtApplicationWindow::ZLQtApplicationWindow(ZLApplication *application) :
	ZLDesktopApplicationWindow(application),
    view_widget_(0),
	myFullScreen(false),
	myWasMaximized(true),
	myCursorIsHyperlink(false) {

#ifdef Q_WS_QWS
    setWindowFlags(Qt::FramelessWindowHint);
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif

    menuBar()->hide();
    showMaximized();

}

void ZLQtApplicationWindow::init() {
	ZLDesktopApplicationWindow::init();
    return;

	switch (myWindowStateOption.value()) {
		case NORMAL:
			break;
		case FULLSCREEN:
			setFullscreen(true);
			break;
		case MAXIMIZED:
			showMaximized();
			break;
	}
}

ZLQtApplicationWindow::~ZLQtApplicationWindow() {
}

void ZLQtApplicationWindow::setFullscreen(bool fullscreen) {
	if (fullscreen == myFullScreen) {
		return;
	}
	myFullScreen = fullscreen;
}

bool ZLQtApplicationWindow::isFullscreen() const {
	return myFullScreen;
}

void ZLQtApplicationWindow::keyPressEvent(QKeyEvent *event)
{
    event->accept();
}

void ZLQtApplicationWindow::keyReleaseEvent(QKeyEvent *event) {

    if (!application().isTextSelectionEnabled())
    {
        switch (event->key())
        {
        case Qt::Key_Up:
            if (view_widget_->isHyperlinkSelected())
            {
                view_widget_->processKeyReleaseEvent(event->key());
            }
            else
            {
                application().doAction("increaseFont");
            }
            break;
        case Qt::Key_Down:
            if (view_widget_->isHyperlinkSelected())
            {
                view_widget_->processKeyReleaseEvent(event->key());
            }
            else
            {
                application().doAction("decreaseFont");
            }
            break;
        //case Qt::Key_Right:
        //    application().doAction("redo");
        //    break;
        //case Qt::Key_Left:
        //    application().doAction("undo");
        //    break;
        case ui::Device_Menu_Key:
            if (view_widget_)
            {
                view_widget_->popupMenu();
            }
            break;
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_PageDown:
        case Qt::Key_PageUp:
        case Qt::Key_Right:
        case Qt::Key_Left:
            if (view_widget_)
            {
                view_widget_->processKeyReleaseEvent(event->key());
            }
            break;
        default:
            application().doActionByKey(ZLQtKeyUtil::keyName(event));
            break;
        }
    }
    else
    {
        view_widget_->processKeyReleaseEvent(event->key());
    }
}

void ZLQtApplicationWindow::wheelEvent(QWheelEvent *event) {
	if (event->orientation() == Qt::Vertical) {
		if (event->delta() > 0) {
			application().doActionByKey(ZLApplication::MouseScrollUpKey);
		} else {
			application().doActionByKey(ZLApplication::MouseScrollDownKey);
		}
	}
}

void ZLQtApplicationWindow::closeEvent(QCloseEvent *event) {
	if (application().closeView()) {
		event->accept();
	} else {
		event->ignore();
	}
}

bool ZLQtApplicationWindow::event(QEvent *e)
{
    return QMainWindow::event(e);
}

void ZLQtApplicationWindow::processAllEvents() {
    qApp->processEvents();
}

void ZLQtApplicationWindow::updateScreen()
{
    if (onyx::screen::instance().userData() < 2)
    {
        ++onyx::screen::instance().userData();
        if (onyx::screen::instance().userData() == 2)
        {
            sys::SysStatus::instance().setSystemBusy(false);
            onyx::screen::instance().updateWidget(
                this,
                onyx::screen::ScreenProxy::GC,
                true,
                onyx::screen::ScreenCommand::WAIT_ALL);
        }
        return;
    }

    if (onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW)
    {
        onyx::screen::instance().updateWidget(
            this,
            onyx::screen::ScreenProxy::DW,
            true,
            onyx::screen::ScreenCommand::WAIT_ALL);
    }
    else
    {
        onyx::screen::ScreenProxy::Waveform w = onyx::screen::ScreenProxy::GU;
        if (application().document_path.endsWith(".txt", Qt::CaseInsensitive) && sys::isImx508())
        {
            w = onyx::screen::ScreenProxy::GC4;
        }

        onyx::screen::instance().updateWidgetWithGCInterval(
            this,
            NULL,
            w,
            true,
            onyx::screen::ScreenCommand::WAIT_ALL);
    }
}

ZLViewWidget *ZLQtApplicationWindow::createViewWidget() {
	ZLQtViewWidget *wnd = new ZLQtViewWidget(this, &application());
	setCentralWidget(wnd->widget());
	wnd->widget()->show();
    view_widget_ = wnd;
	return view_widget_;
}

bool ZLQtApplicationWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (key_event->key() == Qt::Key_Escape)
        {
            return true;
        }
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (key_event->key() == Qt::Key_PageDown ||
            key_event->key() == Qt::Key_PageUp ||
            key_event->key() == Qt::Key_Escape)
        {
            keyReleaseEvent(key_event);
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

void ZLQtApplicationWindow::close() {
    QMainWindow::close();
    if (view_widget_)
    {
        view_widget_->closeDocument();
    }
}

void ZLQtApplicationWindow::grabAllKeys(bool) {
}

void ZLQtApplicationWindow::setCaption(const std::string &caption) {
    // QMainWindow::setWindowTitle(QString::fromUtf8(caption.c_str()));
}

void ZLQtApplicationWindow::setHyperlinkCursor(bool hyperlink) {
	if (hyperlink == myCursorIsHyperlink) {
		return;
	}
	myCursorIsHyperlink = hyperlink;

    // John: No hyperlink cursor.
    // if (hyperlink) {
    // myStoredCursor = cursor();
    // setCursor(Qt::PointingHandCursor);
    // } else {
    // setCursor(myStoredCursor);
    // }

}

void ZLQtApplicationWindow::setFocusToMainWidget() {
	centralWidget()->setFocus();
}

void ZLQtApplicationWindow::onScreenSizeChanged(int)
{
    onyx::screen::instance().enableUpdate(false);
    resize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC);
}

