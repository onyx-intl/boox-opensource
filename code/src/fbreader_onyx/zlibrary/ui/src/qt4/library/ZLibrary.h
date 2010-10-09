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

#include <QObject>

#include <ZLApplication.h>
#include <ZLibrary.h>
#include <ZLLanguageUtil.h>

#include "../../../../core/src/unix/library/ZLibraryImplementation.h"

#include "../filesystem/ZLQtFSManager.h"
#include "../time/ZLQtTime.h"
#include "../dialogs/ZLQtDialogManager.h"
#include "../image/ZLQtImageManager.h"
#include "../view/ZLQtPaintContext.h"
#include "../../unix/message/ZLUnixMessage.h"
#include "../../../../core/src/util/ZLKeyUtil.h"
#include "../../../../core/src/unix/xmlconfig/XMLConfig.h"
#include "../../../../core/src/unix/iconv/IConvEncodingConverter.h"

#include "onyx/base/base.h"
#include "onyx/base/dbus.h"

class OnyxReaderApplicationAdaptor;

class ZLQtLibraryImplementation : public QObject
                                , public ZLibraryImplementation
{
    Q_OBJECT

public Q_SLOTS:
    bool flip(int);

private:
    void init(int &argc, char **&argv);
    ZLPaintContext *createContext();
    void run(ZLApplication *application);

private:
    scoped_ptr<OnyxReaderApplicationAdaptor> adaptor_;
};


class OnyxReaderApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.onyx_reader");

public:
    explicit OnyxReaderApplicationAdaptor(ZLQtLibraryImplementation *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().
            registerService("com.onyx.service.onyx_reader");
        QDBusConnection::systemBus().
            registerObject("/com/onyx/object/onyx_reader", app_);
    }

    public Q_SLOTS:
        bool open(const QString & path) { return false; }
        bool close(const QString & path) { return false; }
        bool flip(int value) { return app_->flip(value); }

private:
    ZLQtLibraryImplementation *app_;
    NO_COPY_AND_ASSIGN(OnyxReaderApplicationAdaptor);
};

