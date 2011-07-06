#include "mainwindow.h"
#include <QtGui/QFileDialog>
#include <QtGui/QStyleFactory>
#include <QClipboard>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include "settings.h"
#include "crqtutil.h"
#include "../crengine/include/lvtinydom.h"

#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/menu.h"
#include "onyx/sys/sys_status.h"
#include <QKeyEvent>

#define DOC_CACHE_SIZE 128 * 0x100000

#ifndef ENABLE_BOOKMARKS_DIR
#define ENABLE_BOOKMARKS_DIR 1
#endif

OnyxMainWindow::OnyxMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    view_ = new CR3View;
    view_->setFocusPolicy(Qt::NoFocus);
    this->setCentralWidget(view_);

    statusbar_ = new StatusBar(this);
    this->setStatusBar(statusbar_);

    onyx::screen::watcher().addWatcher(this);

#ifdef _LINUX
    QString homeDir = QDir::toNativeSeparators(QDir::homePath() + "/.cr3/");
#else
    QString homeDir = QDir::toNativeSeparators(QDir::homePath() + "/cr3/");
#endif

#ifdef _LINUX
    QString exeDir = QString(CR3_DATA_DIR);
#else
    QString exeDir = QDir::toNativeSeparators(qApp->applicationDirPath() + "/"); //QDir::separator();
#endif

    QString cacheDir = homeDir + "cache";
    QString bookmarksDir = homeDir + "bookmarks";
    QString histFile = exeDir + "cr3hist.bmk";
    QString histFile2 = homeDir + "cr3hist.bmk";
    QString iniFile = exeDir + "cr3.ini";
    QString iniFile2 = homeDir + "cr3.ini";
    QString cssFile = homeDir + "fb2.css";
    QString cssFile2 = exeDir + "fb2.css";
    //QString translations = exeDir + "i18n";
    //CRLog::info("Translations directory: %s", LCSTR(qt2cr(translations)) );
    QString hyphDir = exeDir + "hyph" + QDir::separator();
    ldomDocCache::init( qt2cr( cacheDir ), DOC_CACHE_SIZE );
    view_->setPropsChangeCallback( this );
    if ( !view_->loadSettings( iniFile ) )
        view_->loadSettings( iniFile2 );
    if ( !view_->loadHistory( histFile ) )
        view_->loadHistory( histFile2 );
    view_->setHyphDir( hyphDir );
    if ( !view_->loadCSS( cssFile ) )
        view_->loadCSS( cssFile2 );
#if ENABLE_BOOKMARKS_DIR==1
        view_->setBookmarksDir( bookmarksDir );
#endif

    QStringList args( qApp->arguments() );
    for ( int i=1; i<args.length(); i++ ) {
        if ( args[i].startsWith("--") ) {
            // option
        } else {
            // filename to open
            if ( _filenameToOpen.length()==0 )
                _filenameToOpen = args[i];
        }
    }

    view_->loadDocument(_filenameToOpen);
    statusbar_->setProgress(1, view_->getDocView()->getPageCount());

//     QTranslator qtTranslator;
//     if (qtTranslator.load("qt_" + QLocale::system().name(),
//             QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
//        QApplication::installTranslator(&qtTranslator);
//
//     QTranslator myappTranslator;
//     QString trname = "cr3_" + QLocale::system().name();
//     CRLog::info("Using translation file %s from dir %s", UnicodeToUtf8(qt2cr(trname)).c_str(), UnicodeToUtf8(qt2cr(translations)).c_str() );
//     if ( myappTranslator.load(trname, translations) )
//         QApplication::installTranslator(&myappTranslator);

    props_ref = view_->getOptions();
    QString family;
    QString size;
    props_ref->getString(PROP_FONT_FACE, family);
    props_ref->getString(PROP_FONT_SIZE, size);
    select_font = QFont(family, size.toInt());
    font_family_actions_.loadExternalFonts();

    connect(statusbar_, SIGNAL(menuClicked()), this, SLOT(showContextMenu()));
}

void OnyxMainWindow::closeEvent ( QCloseEvent * event )
{
}

OnyxMainWindow::~OnyxMainWindow()
{
}

void OnyxMainWindow::onPropsChange( PropsRef props )
{
    for ( int i=0; i<props->count(); i++ ) {
        QString name = props->name( i );
        QString value = props->value( i );
        int v = (value != "0");
        CRLog::debug("OnyxMainWindow::onPropsChange [%d] '%s'=%s ", i, props->name(i), props->value(i).toUtf8().data() );
        if ( name == PROP_WINDOW_FULLSCREEN ) {
            bool state = windowState().testFlag(Qt::WindowFullScreen);
            bool vv = v ? true : false;
            if ( state != vv )
                setWindowState( windowState() ^ Qt::WindowFullScreen );
        }
        if ( name == PROP_WINDOW_SHOW_MENU ) {
            //ui->menuBar->setVisible( v );
        }
        if ( name == PROP_WINDOW_SHOW_SCROLLBAR ) {
            //ui->scroll->setVisible( v );
        }
        if ( name == PROP_BACKGROUND_IMAGE ) {
            lString16 fn = qt2cr(value);
            LVImageSourceRef img;
            if ( !fn.empty() && fn[0]!='[' ) {
                CRLog::debug("Background image file: %s", LCSTR(fn));
                LVStreamRef stream = LVOpenFileStream(fn.c_str(), LVOM_READ);
                if ( !stream.isNull() ) {
                    img = LVCreateStreamImageSource(stream);
                }
            }
            fn.lowercase();
            bool tiled = ( fn.pos(lString16("\\textures\\"))>=0 || fn.pos(lString16("/textures/"))>=0);
        }
        if ( name == PROP_WINDOW_STYLE ) {
            QApplication::setStyle( value );
        }
    }
}

void OnyxMainWindow::contextMenu( QPoint pos )
{
}


static bool firstShow = true;

void OnyxMainWindow::showEvent ( QShowEvent * event )
{
    if ( !firstShow )
        return;
    CRLog::debug("first showEvent()");
    firstShow = false;
}

static bool firstFocus = true;

void OnyxMainWindow::focusInEvent ( QFocusEvent * event )
{
    if ( !firstFocus )
        return;
    CRLog::debug("first focusInEvent()");
//    int n = view_->getOptions()->getIntDef( PROP_APP_START_ACTION, 0 );
//    if ( n==1 ) {
//        // show recent books dialog
//        CRLog::info("Startup Action: Show recent books dialog");
//        RecentBooksDlg::showDlg( view_ );
//    }

    firstFocus = false;
}


void OnyxMainWindow::on_actionFindText_triggered()
{
    QMessageBox * mb = new QMessageBox( QMessageBox::Information, tr("Not implemented"), tr("Search is not implemented yet"), QMessageBox::Close, this );
    mb->exec();
}

void OnyxMainWindow::keyPressEvent(QKeyEvent *ke)
 {
     ke->accept();
 }

 void OnyxMainWindow::keyReleaseEvent(QKeyEvent *ke)
 {
     static int n=0;

     switch (ke->key())
     {
     case Qt::Key_PageDown:
     case Qt::Key_Right:
         {
             view_->nextPage();
             statusbar_->setProgress(view_->getDocView()->getCurPage()+1, 100);
             update();
             if(++n%=5)
             {
                 onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW,
                         onyx::screen::ScreenCommand::WAIT_NONE);
                 return;
             }
             else
             {
                 onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC,
                         onyx::screen::ScreenCommand::WAIT_NONE);
                 return;
             }
         }
         break;
     case Qt::Key_PageUp:
     case Qt::Key_Left:
         {
             view_->prevPage();
             statusbar_->setProgress(view_->getDocView()->getCurPage(), 100);
             update();
             if(++n%=5)
             {
                 onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW,
                         onyx::screen::ScreenCommand::WAIT_NONE);
                 return;
             }
             else
             {
                 onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC,
                         onyx::screen::ScreenCommand::WAIT_NONE);
                 return;
             }
         }
         break;
     case Qt::Key_Up:
         {
             view_->prevChapter();
             statusbar_->setProgress(view_->getDocView()->getCurPage(), 100);
             update();
             onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW,
                     onyx::screen::ScreenCommand::WAIT_NONE);
             return;
         }
     case Qt::Key_Down:
         {
             view_->nextChapter();
             statusbar_->setProgress(view_->getDocView()->getCurPage(), 100);
             update();
             onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW,
                     onyx::screen::ScreenCommand::WAIT_NONE);
             return;
         }
     case Qt::Key_Return:
         {
             //gotoPage();
         }
         break;
     case Qt::Key_Menu:
         {
             showContextMenu();
         }
         break;
     case Qt::Key_Escape:
         {
             if (this->isFullScreenByWidgetSize())
             {
                 statusbar_->show();
                 onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW,
                         onyx::screen::ScreenCommand::WAIT_NONE);
             }
             else
             {
                 this->close();
             }
         }
         break;
     }

     QMainWindow::keyReleaseEvent(ke);
 }

void OnyxMainWindow::showContextMenu()
{
    PopupMenu menu(this);
    updateActions();

    menu.addGroup(&font_family_actions_);
    menu.addGroup(&font_actions_);
    menu.addGroup(&reading_style_actions_);
    menu.addGroup(&zoom_setting_actions_);
    menu.addGroup(&reading_tool_actions_);
    menu.setSystemAction(&system_actions_);

    if (menu.popup() != QDialog::Accepted)
    {
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        QApplication::processEvents();
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == zoom_setting_actions_.category())
    {
        //processZoomingActions();
    }
    else if (group == font_family_actions_.category())
    {
        select_font.setFamily(font_family_actions_.selectedFont());
        props_ref->setString( PROP_FONT_FACE, font_family_actions_.selectedFont() );
        view_->setOptions(props_ref);
    }
    else if (group == font_actions_.category())
    {
        select_font = font_actions_.selectedFont();
        props_ref->setString( PROP_FONT_SIZE, QString().setNum(select_font.pointSize()));
        view_->setOptions(props_ref);
    }
    else if (group == reading_tool_actions_.category())
    {
        processToolActions();
    }
    else if (group == system_actions_.category())
    {
        SystemAction system = system_actions_.selected();
        if (system == RETURN_TO_LIBRARY)
        {
            qApp->quit();
        }
        else if (system == FULL_SCREEN)
        {
            statusbar_->hide();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW,
                    onyx::screen::ScreenCommand::WAIT_NONE);
        }
        else if (system == EXIT_FULL_SCREEN)
        {
            statusbar_->show();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW,
                    onyx::screen::ScreenCommand::WAIT_NONE);
        }
        else if (system == MUSIC)
        {
            // Start or show music player.
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
            sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
        }
        else if (system == ROTATE_SCREEN)
        {
            SysStatus::instance().rotateScreen();
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        }
    }
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
}

void OnyxMainWindow::updateZoomingActions()
{
    if (zoom_setting_actions_.actions().size() <= 0)
    {
         std::vector<ZoomFactor> zoom_settings;
         zoom_settings.clear();
         zoom_settings.push_back(ZOOM_TO_PAGE);
         zoom_settings.push_back(ZOOM_TO_WIDTH);
         zoom_settings.push_back(ZOOM_TO_HEIGHT);
         zoom_settings.push_back(75.0f);
         zoom_settings.push_back(100.0f);
         zoom_settings.push_back(125.0f);
         zoom_settings.push_back(150.0f);
         zoom_settings.push_back(175.0f);
         zoom_settings.push_back(200.0f);
         zoom_settings.push_back(300.0f);
         zoom_settings.push_back(400.0f);
         zoom_setting_actions_.generateActions(zoom_settings);
     }
     //zoom_setting_actions_.setCurrentZoomValue(model_->currentZoomRatio());
}

void OnyxMainWindow::updateToolActions()
{
    // Reading tools of go to page and clock.
    std::vector<ReadingToolsType> tools;
    tools.push_back(::ui::GOTO_PAGE);
    tools.push_back(::ui::CLOCK_TOOL);
    reading_tool_actions_.generateActions(tools, false);
}

bool OnyxMainWindow::updateActions()
{
    updateZoomingActions();
    updateToolActions();


    // Font family.
    QFont font = currentFont();
    qDebug()<<"font :"<<font.toString();
    font_family_actions_.generateActions(font.family(), true);

    // size
    std::vector<int> size;
    font_actions_.generateActions(font, size, font.pointSize());

    std::vector<int> all;
    all.push_back(ROTATE_SCREEN);

    if (isFullScreenByWidgetSize())
    {
        all.push_back(EXIT_FULL_SCREEN);
    } else
    {
        all.push_back(FULL_SCREEN);
    }

    all.push_back(MUSIC);
    all.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(all);
    return true;
}

const QFont & OnyxMainWindow::currentFont()
{
    return select_font;
}

bool OnyxMainWindow::isFullScreenByWidgetSize()
{
    return !statusbar_->isVisible();
}

void OnyxMainWindow::processToolActions()
{
    ReadingToolsType tool = reading_tool_actions_.selectedTool();
    switch (tool)
    {
    case ::ui::GOTO_PAGE:
        {
            gotoPage();
        }
        break;
    case ::ui::CLOCK_TOOL:
        {
            showClock();
        }
        break;
    default:
        break;
    }
}

void OnyxMainWindow::gotoPage()
{
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    //emit requestGotoPageDialog();
    statusbar_->onMessageAreaClicked();
    //resetScrollBar();
}

void OnyxMainWindow::showClock()
{

}
