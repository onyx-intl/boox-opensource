#include <stdio.h>
#include <QtGui/QFileDialog>
#include <QtGui/QStyleFactory>
#include <QClipboard>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QKeyEvent>

#include "mainwindow.h"

#include "settings.h"
#include "crqtutil.h"
#include "../crengine/include/lvtinydom.h"

#include "onyx/screen/screen_update_watcher.h"
#include "onyx/data/bookmark.h"
#include "onyx/ui/menu.h"
#include "onyx/ui/screen_rotation_dialog.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/sys/sys_status.h"
#include "onyx/sys/platform.h"
#include "onyx/ui/number_dialog.h"
#include "onyx/ui/glow_light_control_dialog.h"
#include "onyx/cms/content_manager.h"
#include "onyx/cms/content_thumbnail.h"

#include "../lcl_ui/settings_dialog.h"
#include "../lcl_ui/info_dialog.h"
#include "../lcl_ui/recent_books.h"

#include <cr3version.h>

#define DOC_CACHE_SIZE 128 * 0x100000

#ifndef ENABLE_BOOKMARKS_DIR
#define ENABLE_BOOKMARKS_DIR 1
#endif

using namespace ui;

lString16 getDocText( ldomDocument * doc, const char * path, const char * delim )
{
    lString16 res;
    for ( int i=0; i<100; i++ ) {
        lString8 p = lString8(path) + "[" + lString8::itoa(i+1) + "]";
        //CRLog::trace("checking doc path %s", p.c_str() );
        lString16 p16 = Utf8ToUnicode(p);
        ldomXPointer ptr = doc->createXPointer( p16 );
        if ( ptr.isNull() )
            break;
        lString16 s = ptr.getText( L' ' );
        if ( s.empty() )
            continue;
        if ( !res.empty() && delim!=NULL )
            res << Utf8ToUnicode( lString8( delim ) );
        res << s;
    }
    return res;
}

lString16 getDocAuthors( ldomDocument * doc, const char * path, const char * delim )
{
    lString16 res;
    for ( int i=0; i<100; i++ ) {
        lString8 p = lString8(path) + "[" + lString8::itoa(i+1) + "]";
        //CRLog::trace("checking doc path %s", p.c_str() );
        lString16 firstName = getDocText( doc, (p + "/first-name").c_str(), " " );
        lString16 lastName = getDocText( doc, (p + "/last-name").c_str(), " " );
        lString16 middleName = getDocText( doc, (p + "/middle-name").c_str(), " " );
        lString16 nickName = getDocText( doc, (p + "/nickname").c_str(), " " );
        lString16 homePage = getDocText( doc, (p + "/homepage").c_str(), " " );
        lString16 email = getDocText( doc, (p + "/email").c_str(), " " );
        lString16 s = firstName;
        if ( !middleName.empty() )
            s << L" " << middleName;
        if ( !lastName.empty() ) {
            if ( !s.empty() )
                s << L" ";
            s << lastName;
        }
        if ( !nickName.empty() ) {
            if ( !s.empty() )
                s << L" ";
            s << nickName;
        }
        if ( !homePage.empty() ) {
            if ( !s.empty() )
                s << L" ";
            s << homePage;
        }
        if ( !email.empty() ) {
            if ( !s.empty() )
                s << L" ";
            s << email;
        }
        if ( s.empty() )
            continue;
        if ( !res.empty() && delim!=NULL )
            res << Utf8ToUnicode( lString8( delim ) );
        res << s;
    }
    return res;
}

OnyxMainWindow::OnyxMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , able_go_to_page_(true)
{
    resize(600, 800);

    view_ = new CR3View;
    setCentralWidget(view_);

    status_bar_ = new StatusBar(this, MENU|PROGRESS|MESSAGE|CLOCK|BATTERY|SCREEN_REFRESH);
    setStatusBar(status_bar_);

    sys::SystemConfig conf;
    onyx::screen::watcher().addWatcher(this, conf.screenUpdateGCInterval());

    QString homeDir = QDir::toNativeSeparators("/media/flash/cr3/");
    QString exeDir = QString(CR3_DATA_DIR);

    QString cacheDir = homeDir + "cache";
    QString bookmarksDir = homeDir + "bookmarks";
    QString histFile = exeDir + "cr3hist.bmk";
    QString histFile2 = homeDir + "cr3hist.bmk";
    QString iniFile = exeDir + "cr3.ini";
    QString iniFile2 = homeDir + "cr3.ini";
    QString cssFile = homeDir + "fb2.css";
    QString cssFile2 = exeDir + "fb2.css";

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
            if ( file_name_to_open_.length()==0 )
                file_name_to_open_ = args[i];
        }
    }

    select_font_ = currentFont();
    font_family_actions_.loadExternalFonts();

#ifdef Q_WS_QWS
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif
    connect( &(SysStatus::instance()), SIGNAL(aboutToShutdown()), this, SLOT(close()) );
    connect( &(SysStatus::instance()), SIGNAL(forceQuit()), this, SLOT(close()) );

    connect(status_bar_, SIGNAL(menuClicked()), this, SLOT(popupMenu()));
    connect(&SysStatus::instance(), SIGNAL(mouseLongPress(QPoint, QSize)), this, SLOT(popupMenu()));

    connect(view_, SIGNAL(updateProgress(int,int)), status_bar_, SLOT(setProgress(int,int)));
    connect(status_bar_, SIGNAL(progressClicked(int,int)), this ,SLOT(onProgressClicked(const int, const int)));
    connect(view_, SIGNAL(requestUpdateAll()), this,SLOT(updateScreen()));
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);

    view_->restoreWindowPos( this, "main.", true );

    loadDocumentOptions(file_name_to_open_);

    view_->loadDocument(file_name_to_open_);

    view_->paintCitation();
}

void OnyxMainWindow::closeEvent ( QCloseEvent * event )
{
    view_->saveWindowPos( this, "main." );
}

OnyxMainWindow::~OnyxMainWindow()
{
//    storeThumbnail();
    saveDocumentOptions(file_name_to_open_);
    delete status_bar_;
    delete view_;
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
}

void OnyxMainWindow::keyPressEvent(QKeyEvent *ke)
 {
     ke->accept();
 }

 void OnyxMainWindow::keyReleaseEvent(QKeyEvent *ke)
 {
     switch (ke->key())
     {
     case Qt::Key_PageDown:
     case Qt::Key_Right:
         {
             view_->nextPageWithTTSChecking();
             return;
         }
         break;
     case Qt::Key_PageUp:
     case Qt::Key_Left:
         {
             view_->prevPageWithTTSChecking();
             return;
         }
         break;
     case Qt::Key_Up:
         {
             view_->zoomIn();
             onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
             return;
         }
     case Qt::Key_Down:
         {
             view_->zoomOut();
             onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
             return;
         }
     case Qt::Key_Enter: // fall through
     case Qt::Key_Return:
         {
             this->gotoPage();
             return;
         }
         break;
     case Qt::Key_Menu:
     case Qt::Key_F10:
         {
             popupMenu();
         }
         break;
     case Qt::Key_Escape:
         {
             this->close();
         }
         break;
     }

     QMainWindow::keyReleaseEvent(ke);
 }

void OnyxMainWindow::popupMenu()
{
    if(menu_ && menu_->isVisible())
    {
        return;
    }

    menu_.reset(new PopupMenu(this));

    updateActions();

    menu_->addGroup(&font_family_actions_);
    menu_->addGroup(&font_actions_);
    menu_->addGroup(&reading_style_actions_);
    //menu.addGroup(&zoom_setting_actions_);
    menu_->addGroup(&reading_tool_actions_);
    menu_->addGroup(&advanced_actions_);
    menu_->setSystemAction(&system_actions_);

    if (menu_->popup() != QDialog::Accepted)
    {
        //onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        QApplication::processEvents();
        return;
    }

    QAction * group = menu_->selectedCategory();
    if (group == zoom_setting_actions_.category())
    {
        //processZoomingActions();
    }
    else if (group == font_family_actions_.category())
    {
        select_font_.setFamily(font_family_actions_.selectedFont());
        props_ref_->setString( PROP_FONT_FACE, font_family_actions_.selectedFont() );
        view_->setOptions(props_ref_);
    }
    else if (group == font_actions_.category())
    {
        select_font_ = font_actions_.selectedFont();
        props_ref_->setString( PROP_FONT_SIZE, QString().setNum(select_font_.pointSize()));
        if (select_font_.bold()) {
            view_->toggleFontEmbolden();
        }
        view_->setOptions(props_ref_);
    }
    else if (group == reading_tool_actions_.category())
    {
        processToolActions();
        update();
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
        return;
    }
    else if (group == advanced_actions_.category())
    {
        processAdvancedActions();
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
            view_->setFullScreen(true);
            view_->update();
            status_bar_->hide();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU,
                    onyx::screen::ScreenCommand::WAIT_NONE);
        }
        else if (system == EXIT_FULL_SCREEN)
        {
            view_->setFullScreen(false);
            view_->update();
            status_bar_->show();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU,
                    onyx::screen::ScreenCommand::WAIT_NONE);
        }
        else if (system == MUSIC)
        {
            // Start or show music player.
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
            this->update();
            sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
        }
        else if (system == SYSTEM_VOLUME)
        {
            status_bar_->onVolumeClicked();
        }
        else if (system == GLOW_LIGHT_CONTROL)
        {
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
            ui::GlowLightControlDialog dialog(this);
            dialog.exec();
            QApplication::processEvents();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
        }
        else if (system == ROTATE_SCREEN)
        {
            ui::ScreenRotationDialog dialog;
            dialog.popup();
//            view_->update();
//            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
//            this->update();
            return;
        }
    }
    else if(group == reading_style_actions_.category())
    {
        ReadingStyleType s = reading_style_actions_.selected();
        if (s >= STYLE_LINE_SPACING_8 && s <= STYLE_LINE_SPACING_20)
        {
            const unsigned int Percentage_Compensation = 80;
            // since (int)STYLE_LINE_SPACING_8's value is 0
            int line_height_percentage = ((int)s * 10) + Percentage_Compensation;
            this->setLineHeight(line_height_percentage);

            status_bar_->update();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
            return;
        }
    }
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
    this->update();
}

void OnyxMainWindow::setLineHeight(const unsigned int lineHeightPercentage)
{
    props_ref_->setInt(PROP_INTERLINE_SPACE, lineHeightPercentage);
    view_->setOptions(props_ref_);
    return;
}

void OnyxMainWindow::updateReadingStyleActions()
{
    int index;
    view_->getOptions()->getInt(PROP_INTERLINE_SPACE, index);
    index = STYLE_LINE_SPACING_8+((index-80)/10);
    reading_style_actions_.generateActions(static_cast<ReadingStyleType>(index));
}

void OnyxMainWindow::updateToolActions()
{
    // Reading tools of go to page and clock.
    reading_tool_actions_.clear();
    std::vector<ReadingToolsType> tools;

    tools.clear();
    tools.push_back(::ui::SEARCH_TOOL);
    if (SysStatus::instance().hasTouchScreen() ||
        sys::SysStatus::instance().isDictionaryEnabled())
    {
        tools.push_back(DICTIONARY_TOOL);
    }
    tools.push_back(::ui::TOC_VIEW_TOOL);
    reading_tool_actions_.generateActions(tools, true);

    if (SysStatus::instance().hasTouchScreen() ||
        SysStatus::instance().isTTSEnabled())
    {
        tools.clear();
        tools.push_back(::ui::TEXT_TO_SPEECH);
        reading_tool_actions_.generateActions(tools, true);
    }

    tools.clear();
    if( !view_->hasBookmark() )
    {
        tools.push_back(::ui::ADD_BOOKMARK);
    }
    else
    {
        tools.push_back(::ui::DELETE_BOOKMARK);
    }

    tools.push_back(::ui::SHOW_ALL_BOOKMARKS);
    reading_tool_actions_.generateActions(tools, true);

    tools.clear();
    tools.push_back(::ui::PREVIOUS_VIEW);
    tools.push_back(::ui::NEXT_VIEW);
    tools.push_back(::ui::GOTO_PAGE);
    tools.push_back(::ui::CLOCK_TOOL);

    reading_tool_actions_.generateActions(tools, true);
}

bool OnyxMainWindow::updateActions()
{   
    updateReadingStyleActions();
    updateToolActions();

    advanced_actions_.clear();
    std::vector<AdvancedType> advanced;
    advanced.push_back(SETTINGS);
    advanced.push_back(INFO);
    advanced.push_back(RECENT_BOOKS);
    advanced_actions_.generateActions(advanced, true);

    advanced.clear();
    advanced.push_back(ADD_CITATION);
    advanced.push_back(DELETE_CITE);
    advanced.push_back(SHOW_ALL_CITES);
    advanced_actions_.generateActions(advanced, true);

    // Font family.
    QFont font = currentFont();
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
    if (sys::isIRTouch() || ui::isLandscapeVolumeMapping())
    {
        all.push_back(SYSTEM_VOLUME);
    }
    all.push_back(GLOW_LIGHT_CONTROL);
    all.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(all);
    return true;
}

const QFont & OnyxMainWindow::currentFont()
{
    props_ref_ = view_->getOptions();
    QString family;
    QString size;
    props_ref_->getString(PROP_FONT_FACE, family);
    props_ref_->getString(PROP_FONT_SIZE, size);
    select_font_ = QFont(family, size.toInt());
    return select_font_;
}

bool OnyxMainWindow::isFullScreenByWidgetSize()
{
    return !status_bar_->isVisible();
}

void OnyxMainWindow::processAdvancedActions()
{
    AdvancedType atype = advanced_actions_.selectedTool();
    switch (atype) {
        case SETTINGS: {
            SettingsDialog sdialog(0);

            QString prop_str;

            props_ref_->getString(PROP_STATUS_LINE, prop_str);
            sdialog.status_line = (prop_str == "0");

            props_ref_->getString(PROP_SHOW_TIME, prop_str);
            sdialog.show_time = (prop_str == "1");

            props_ref_->getString(PROP_LANDSCAPE_PAGES, prop_str);
            sdialog.two_pages_landscape = (prop_str == "2");

            props_ref_->getString(PROP_FONT_SIZE, prop_str);
            sdialog.font_size = prop_str;

            props_ref_->getString(PROP_FONT_WEIGHT_EMBOLDEN, prop_str);
            sdialog.font_bold = (prop_str == "1");

            props_ref_->getString(PROP_FONT_ANTIALIASING, prop_str);
            sdialog.font_aa = (prop_str == "1" || prop_str == "2");

            props_ref_->getString(PROP_DISPLAY_INVERSE, prop_str);
            sdialog.display_inverse = (prop_str == "1");

            props_ref_->getString(PROP_PAGE_MARGIN_LEFT, prop_str);
            sdialog.l_margin = prop_str;
            props_ref_->getString(PROP_PAGE_MARGIN_RIGHT, prop_str);
            sdialog.r_margin = prop_str;
            props_ref_->getString(PROP_PAGE_MARGIN_TOP, prop_str);
            sdialog.t_margin = prop_str;
            props_ref_->getString(PROP_PAGE_MARGIN_BOTTOM, prop_str);
            sdialog.b_margin = prop_str;

            sdialog.exec();
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);

            if (!sdialog.save)
                break;

            prop_str = sdialog.status_line ? "0" : "2";
            props_ref_->setString(PROP_STATUS_LINE, prop_str);

            prop_str = sdialog.show_time ? "1" : "0";
            props_ref_->setString(PROP_SHOW_TIME, prop_str);

            prop_str = sdialog.two_pages_landscape ? "2" : "1";
            props_ref_->setString(PROP_LANDSCAPE_PAGES, prop_str);

            props_ref_->setString(PROP_FONT_SIZE, sdialog.font_size);

            prop_str = sdialog.font_bold ? "1" : "0";
            props_ref_->setString(PROP_FONT_WEIGHT_EMBOLDEN, prop_str);

            prop_str = sdialog.font_aa ? "2" : "0";
            props_ref_->setString(PROP_FONT_ANTIALIASING, prop_str);

            prop_str = sdialog.display_inverse ? "1" : "0";
            props_ref_->setString(PROP_DISPLAY_INVERSE, prop_str);

            props_ref_->setString(PROP_PAGE_MARGIN_LEFT, sdialog.l_margin);
            props_ref_->setString(PROP_PAGE_MARGIN_RIGHT, sdialog.r_margin);
            props_ref_->setString(PROP_PAGE_MARGIN_TOP, sdialog.t_margin);
            props_ref_->setString(PROP_PAGE_MARGIN_BOTTOM, sdialog.b_margin);

            view_->setOptions(props_ref_);

            break;
                       }
        case INFO: {
            InfoDialog id(0);

            view_->getDocView()->savePosition();
            CRPropRef props = view_->getDocView()->getDocProps();

            QString x;

            id.addLine(QApplication::tr("Status"), true);
            id.addLine(QApplication::tr("Cool Reader version: ") + QString(CR_ENGINE_VERSION));
            id.addLine(QApplication::tr("Current page: ") + QString::number(view_->getDocView()->getCurPage()+1));
            id.addLine(QApplication::tr("Total pages: ") + QString::number(view_->getDocView()->getPageCount()));
            id.addLine(QApplication::tr("Current Time: ") + cr2qt(view_->getDocView()->getTimeString()));

            id.addLine(QApplication::tr("File info"), true);
            id.addLine(QApplication::tr("Archive name: ") + cr2qt(props->getStringDef(DOC_PROP_ARC_NAME)));
            id.addLine(QApplication::tr("Archive path: ") + cr2qt(props->getStringDef(DOC_PROP_ARC_PATH)));
            id.addLine(QApplication::tr("Archive size: ") + cr2qt(props->getStringDef(DOC_PROP_ARC_SIZE)));
            id.addLine(QApplication::tr("File name: ") + cr2qt(props->getStringDef(DOC_PROP_FILE_NAME)));
            id.addLine(QApplication::tr("File path: ") + cr2qt(props->getStringDef(DOC_PROP_FILE_PATH)));
            id.addLine(QApplication::tr("File size: ") + cr2qt(props->getStringDef(DOC_PROP_FILE_SIZE)));
            id.addLine(QApplication::tr("File format: ") + cr2qt(props->getStringDef(DOC_PROP_FILE_FORMAT)));

            id.addLine(QApplication::tr("Book info"), true);
            id.addLine(QApplication::tr("Title: ") + cr2qt(props->getStringDef(DOC_PROP_TITLE) ));
            id.addLine(QApplication::tr("Author(s): ") + cr2qt(props->getStringDef(DOC_PROP_AUTHORS) ));
            id.addLine(QApplication::tr("Series name: ") + cr2qt(props->getStringDef(DOC_PROP_SERIES_NAME) ));
            id.addLine(QApplication::tr("Series number: ") + cr2qt(props->getStringDef(DOC_PROP_SERIES_NUMBER) ));
            id.addLine(QApplication::tr("Date: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/title-info/date", ", " ) ));
            id.addLine(QApplication::tr("Genres: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/title-info/genre", ", " ) ));
            id.addLine(QApplication::tr("Translator: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/title-info/translator", ", " ) ));

            id.addLine(QApplication::tr("Document info"), true);
            id.addLine(QApplication::tr("Document author: ") + cr2qt(getDocAuthors( view_->getDocView()->getDocument(), "/FictionBook/description/document-info/author", " " ) ));
            id.addLine(QApplication::tr("Document date: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/document-info/date", " " ) ));
            id.addLine(QApplication::tr("Document source URL: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/document-info/src-url", " " ) ));
            id.addLine(QApplication::tr("OCR by: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/document-info/src-ocr", " " ) ));
            id.addLine(QApplication::tr("Document version: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/document-info/version", " " ) ));
            id.addLine(QApplication::tr("Change history: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/document-info/history", " " ) ));

            id.addLine(QApplication::tr("Publication info"), true);
            id.addLine(QApplication::tr("Publication name: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/publish-info/book-name", " " ) ));
            id.addLine(QApplication::tr("Publisher: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/publish-info/publisher", " " ) ));
            id.addLine(QApplication::tr("Publisher city: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/publish-info/city", " " ) ));
            id.addLine(QApplication::tr("Publication year: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/publish-info/year", " " ) ));
            id.addLine(QApplication::tr("ISBN: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/publish-info/isbn", " " ) ));

            id.addLine(QApplication::tr("Information"), true);
            id.addLine(QApplication::tr("Custom info: ") + cr2qt(getDocText( view_->getDocView()->getDocument(), "/FictionBook/description/custom-info", " " ) ));

            id.popup(tr("Information"));
            break;
                   }
        case RECENT_BOOKS: {
            RecentBooks rb(0);
            rb.books = view_->getRecentBooks();
            if (rb.popup(tr("Recent Books")) != QDialog::Accepted)
               break;

            view_->openRecentBook(rb.selectedInfo());
            break;
                           }
        case ADD_CITATION:
            {
                view_->enableAddCitation(true);
                break;
            }
        case DELETE_CITE:
            view_->enableDeleteCitation(true);
            break;

        case SHOW_ALL_CITES:
            showAllCites();
            break;

        default:
            break;
    }
}

void OnyxMainWindow::processToolActions()
{
    ReadingToolsType tool = reading_tool_actions_.selectedTool();
    switch (tool)
    {
    case ::ui::TOC_VIEW_TOOL:
        {
            showTableOfContents();
        }
        break;
    case ::ui::DICTIONARY_TOOL:
        {
            view_->startDictLookup();
        }
        break;
    case ::ui::SEARCH_TOOL:
        {
            view_->showSearchWidget();
        }
        break;

    case ::ui::TEXT_TO_SPEECH:
        {
            if (sys::SysStatus::instance().isMusicPlayerRunning())
            {
                MessageDialog dialog(QMessageBox::Information,
                                     tr("Cool Reader"),
                                     tr("The Music player is running. "
                                        "To use TTS, you must exit Music Player."),
                                     QMessageBox::Ok);
                dialog.exec();
                repaint();
                onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
            }
            else
            {
                view_->startTTS();
            }
        }
        break;

    case ::ui::ADD_BOOKMARK:
        addBookmark();
        view_->restoreWindowPos(this, "MyBookmark");
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
        break;
    case ::ui::DELETE_BOOKMARK:
        view_->deleteBookmark();
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
        break;
    case ::ui::SHOW_ALL_BOOKMARKS:
        showAllBookmarks();
        break;

    case ::ui::PREVIOUS_VIEW:
        {
            view_->historyBack();
            updateScreen();
        }
        break;
    case ::ui::NEXT_VIEW:
        {
            view_->historyForward();
            updateScreen();
        }
        break;
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
    if (view_->getDocView()->getPageCount() <= 1)
        return;

    //onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    //emit requestGotoPageDialog();
    //statusbar_->onMessageAreaClicked();
    //resetScrollBar();
    // Popup page dialog.
    NumberDialog dialog(0);
    if (dialog.popup(view_->getDocView()->getCurPage(), view_->getDocView()->getPageCount()) != QDialog::Accepted)
    {
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
        return;
    }

    int current = dialog.value();
    this->onProgressClicked(1, current);
}

void OnyxMainWindow::showClock()
{
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
    status_bar_->onClockClicked();
}

void OnyxMainWindow::updateScreen()
{
    view_->update();

    if (onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW)
    {
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW);
    }
    else
    {
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
    }
}

void OnyxMainWindow::onProgressClicked(const int percentage, const int value)
{
    if(view_->ttsWidget().isVisible())
    {
        if(!able_go_to_page_)
        {
            return;
        }
        able_go_to_page_ = false;
        QTimer::singleShot(1200, this, SLOT(ableGoToPage()));
        sys::SysStatus::instance().setSystemBusy(true);
    }

    view_->gotoPageWithTTSChecking(value);
    updateScreen();
}

void OnyxMainWindow::ableGoToPage()
{
    able_go_to_page_ = true;
    sys::SysStatus::instance().setSystemBusy(false);
    onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GU);
}

void OnyxMainWindow::showTableOfContents()
{
    std::vector<int> paragraphs;
    std::vector<int> pages;
    std::vector<QString> titles;
    std::vector<QString> paths;
    LVTocItem * root = this->view_->getToc();

    for ( int i=0; i<root->getChildCount(); i++ )
    {
        LVTocItem *n = root->getChild(i);
        paragraphs.push_back(n->getLevel());
        titles.push_back( cr2qt(n->getName()));
        paths.push_back(cr2qt(n->getPath()));
        pages.push_back(n->getPage());
        for ( int j=0; j<n->getChildCount(); j++ )
        {
            LVTocItem *m = n->getChild(j);
            paragraphs.push_back(m->getLevel());
            titles.push_back( cr2qt(m->getName()));
            paths.push_back(cr2qt(m->getPath()));
            pages.push_back(m->getPage());
        }
    }

    std::vector<QStandardItem *> ptrs;
    QStandardItemModel model;

    QStandardItem *parent = model.invisibleRootItem();
    for (size_t i = 0;i < paragraphs.size();++i)
    {
        QStandardItem *item= new QStandardItem(titles[i]);
        item->setData(paths[i],Qt::UserRole+100);
        item->setEditable(false);
        ptrs.push_back(item);

        // Get parent.
        parent = searchParent(i, paragraphs, ptrs, model);
        parent->appendRow(item);
    }

    TreeViewDialog dialog( this );
    dialog.setModel( &model);

    int ret = dialog.popup( tr("Table of Contents") );
    if (ret != QDialog::Accepted)
    {
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
        return;
    }

    QModelIndex index = dialog.selectedItem();
    if ( !index.isValid() )
    {
        return;
    }
    view_->goToXPointer(index.data(Qt::UserRole+100).toString());
}

QStandardItem * OnyxMainWindow::searchParent(const int index,
                                           std::vector<int> & entries,
                                           std::vector<QStandardItem *> & ptrs,
                                           QStandardItemModel &model)
{
    int indent = entries[index];
    for(int i = index - 1; i >= 0; --i)
    {
        if (entries[i] < indent)
        {
            return ptrs[i];
        }
    }
    return model.invisibleRootItem();
}

bool OnyxMainWindow::addBookmark()
{
    view_->createBookmark();
    return true;
}

bool OnyxMainWindow::addCite()
{
    view_->createCitation();
    return true;
}

void OnyxMainWindow::showAllCites()
{
    QStandardItemModel model;
    QModelIndex selected = model.invisibleRootItem()->index();
    citeModel(model, selected);

    TreeViewDialog bookmark_view(this);
    bookmark_view.setModel(&model);
    QVector<int> percentages;
    percentages.push_back(80);
    percentages.push_back(20);
    bookmark_view.tree().setColumnWidth(percentages);

    int ret = bookmark_view.popup(QCoreApplication::tr("Citations"));
    // Returned from the bookmark view.
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
    this->update();

    if (ret != QDialog::Accepted)
    {
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
        return;
    }

    CRBookmark *pos = (CRBookmark *)(model.data(bookmark_view.selectedItem(), Qt::UserRole + 1).toInt());
    view_->goToBookmark(pos);
}

void OnyxMainWindow::citeModel(QStandardItemModel & model,
                                   QModelIndex & selected)
{
    CRFileHistRecord * rec = view_->getDocView()->getCurrentFileHistRecord();
    if ( !rec )
        return;
    LVPtrVector<CRBookmark> & list( rec->getBookmarks() );
    model.setColumnCount(2);
    int row = 0;
    for(int i  = 0; i < list.length(); ++i)
    {
        // skip bookmarks
        CRBookmark * bmk = list[i];
        if (!bmk || (bmk->getType() != bmkt_comment && bmk->getType() != bmkt_correction))
            continue;

        //QString t =cr2qt(view_->getDocView()->getPageText(true, list[i]->getBookmarkPage()));
        QString t = cr2qt(list[i]->getPosText());
        t.truncate(100);
        QStandardItem *title = new QStandardItem(t);
        title->setData((int)list[i]);
        title->setEditable(false);
        model.setItem(row, 0, title);

        int pg = 1 + view_->getDocView()->getBookmarkPage(view_->getDocView()->getDocument()->createXPointer( list[i]->getStartPos() ));
        QString str(tr("Page %1"));
        str = str.arg(pg);
        /*
        double pos = list[i]->getPercent() / 100.0;
        QString str(tr("Page %1 (%2%)"));
        str = str.arg(pg);
        str = str.arg(pos);
        */
        QStandardItem *page = new QStandardItem(str);
        page->setTextAlignment(Qt::AlignCenter);
        page->setEditable(false);
        model.setItem(row, 1, page);

        row++;
    }
}

void OnyxMainWindow::showAllBookmarks()
{
    QStandardItemModel model;
    QModelIndex selected = model.invisibleRootItem()->index();
    bookmarkModel(model, selected);

    TreeViewDialog bookmark_view(this);
    bookmark_view.setModel(&model);
    QVector<int> percentages;
    percentages.push_back(80);
    percentages.push_back(20);
    bookmark_view.tree().setColumnWidth(percentages);

    int ret = bookmark_view.popup(QCoreApplication::tr("Bookmarks"));
    // Returned from the bookmark view.
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
    this->update();

    if (ret != QDialog::Accepted)
    {
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
        return;
    }

    CRBookmark *pos = (CRBookmark *)(model.data(bookmark_view.selectedItem(), Qt::UserRole + 1).toInt());
    view_->goToBookmark(pos);
}

void OnyxMainWindow::bookmarkModel(QStandardItemModel & model,
                                   QModelIndex & selected)
{
    CRFileHistRecord * rec = view_->getDocView()->getCurrentFileHistRecord();
    if ( !rec )
        return;
    LVPtrVector<CRBookmark> & list( rec->getBookmarks() );
    model.setColumnCount(2);
    int row = 0;
    for(int i  = 0; i < list.length(); ++i)
    {
        // skip cites
        CRBookmark * bmk = list[i];
        if (!bmk || (bmk->getType() == bmkt_comment || bmk->getType() == bmkt_correction))
            continue;

        QString t =cr2qt(view_->getDocView()->getPageText(true, list[i]->getBookmarkPage()));
        t.truncate(100);
        QStandardItem *title = new QStandardItem(t);
        title->setData((int)list[i]);
        title->setEditable(false);
        model.setItem(row, 0, title);

        int pos = list[i]->getPercent();
        QString str(tr("%1"));
        str = str.arg(pos);
        QStandardItem *page = new QStandardItem(str);
        page->setTextAlignment(Qt::AlignCenter);
        page->setEditable(false);
        model.setItem(row, 1, page);

        row++;
    }
}

void OnyxMainWindow::updateScreenManually()
{
    sys::SysStatus::instance().setSystemBusy(false);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void OnyxMainWindow::onScreenSizeChanged(int)
{
    // Preserve updatability.
    setFixedSize(qApp->desktop()->screenGeometry().size());
    this->resize(qApp->desktop()->screenGeometry().size());

    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

bool OnyxMainWindow::loadDocumentOptions(const QString &path)
{
    ContentManager database;
    if (!vbf::openDatabase(path, database))
    {
        return false;
    }
    if(!vbf::loadDocumentOptions(database, path, conf_))
    {
        return false;
    }

    bool full = qgetenv("COOL_READER_FULL_SCREEN").toInt();
    if (conf_.options.contains(vbf::CONFIG_FULLSCREEN))
    {
        full = conf_.options[vbf::CONFIG_FULLSCREEN].toBool();
    }
    view_->setFullScreen(full);
    status_bar_->setVisible(!full);
    return true;
}

bool OnyxMainWindow::saveDocumentOptions(const QString &path)
{
    QString authors = cr2qt(view_->getDocView()->getAuthors());
    QString title = cr2qt(view_->getDocView()->getTitle());
    cms::ContentManager database;
    if (!vbf::openDatabase(path, database) && !vbf::loadDocumentOptions(database, path, conf_))
    {
        return false;
    }

    conf_.info.mutable_authors() = authors;
    conf_.info.mutable_title() = title;
    conf_.options[vbf::CONFIG_FULLSCREEN] = !status_bar_->isVisibleTo(this);
    return vbf::saveDocumentOptions(database, path, conf_);
}

void OnyxMainWindow::storeThumbnail()
{
    QFileInfo info(file_name_to_open_);
    QImage img=view_->getPageImage();
    cms::ContentThumbnail thumbdb(info.absolutePath());
    thumbdb.storeThumbnail(info.fileName(), cms::THUMBNAIL_LARGE, img.scaled(thumbnailSize(), Qt::KeepAspectRatio));
}
