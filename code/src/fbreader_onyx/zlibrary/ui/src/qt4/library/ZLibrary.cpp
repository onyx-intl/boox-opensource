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

#include <QApplication>
#include "ZLibrary.h"
#include "onyx/ui/ui.h"
#include "onyx/ui/languages.h"

static std::string DOCUMENT_PATH;

void initLibrary() {
    new ZLQtLibraryImplementation();
}

void ZLQtLibraryImplementation::init(int &argc, char **&argv) {
    new QApplication(argc, argv);
    qApp->setDoubleClickInterval(1000);
    adaptor_.reset(new OnyxReaderApplicationAdaptor(this));
    ui::loadTranslator(QLocale::system().name());
    Q_INIT_RESOURCE(vbf_icons);
    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(dictionary_images);
    Q_INIT_RESOURCE(tts_images);

    QVector<int> fonts;
    ui::FontFamilyActions::loadExternalFonts(&fonts);

    // Zlibrary.
    ZLibrary::parseArguments(argc, argv);

    if (argc > 1)
    {
        DOCUMENT_PATH = argv[1];
    }

    // to customize. create our own manager.
    XMLConfigManager::createInstance();
    ZLQtTimeManager::createInstance();
    ZLQtFSManager::createInstance();
    ZLQtDialogManager::createInstance();
    ZLQtImageManager::createInstance();
    ZLEncodingCollection::instance().registerProvider(
        shared_ptr<IConvEncodingConverterProvider>(new IConvEncodingConverterProvider()));

    // ZLKeyUtil::setKeyNamesFileName("keynames-qt4.xml");
    // Before we open the document, make sure the external fonts
    // have been installed. It's necessary as user may use external
    // fonts, but by default, these fonts are not loaded.
    // ui::FontFamilyActions::loadExternalFonts();
}

ZLPaintContext *ZLQtLibraryImplementation::createContext() {
    return new ZLQtPaintContext();
}

bool ZLQtLibraryImplementation::flip(int value)
{
    return true;
}

void ZLQtLibraryImplementation::run(ZLApplication *application) {
    if (ZLLanguageUtil::isRTLLanguage(ZLibrary::Language())) {
        qApp->setLayoutDirection(Qt::RightToLeft);
    }

    application->document_path = QString::fromLocal8Bit(DOCUMENT_PATH.c_str());

    ZLDialogManager::instance().createApplicationWindow(application);
    if (application->initWindow())
    {
        qApp->exec();
    }

    adaptor_.reset(0);
    delete application;
}
