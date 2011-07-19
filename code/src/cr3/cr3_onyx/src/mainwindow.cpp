#include "mainwindow.h"
#include <QtGui/QFileDialog>
#include <QtGui/QStyleFactory>
#include <QClipboard>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include "settings.h"
#include "crqtutil.h"
#include "search_tool.h"
#include "../crengine/include/lvtinydom.h"

#include "onyx/screen/screen_update_watcher.h"
#include "onyx/data/bookmark.h"
#include "onyx/ui/menu.h"
#include "onyx/sys/sys_status.h"
#include "onyx/ui/number_dialog.h"
#include <QKeyEvent>

#define DOC_CACHE_SIZE 128 * 0x100000

#ifndef ENABLE_BOOKMARKS_DIR
#define ENABLE_BOOKMARKS_DIR 1
#endif

static const int BEFORE_SEARCH = 0;
static const int IN_SEARCHING  = 1;

OnyxMainWindow::OnyxMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    view_ = new CR3View;
    view_->setFocusPolicy(Qt::StrongFocus);
    this->setCentralWidget(view_);

    statusbar_ = new StatusBar(this, MENU|PROGRESS|MESSAGE|CLOCK|BATTERY|SCREEN_REFRESH);
    this->setStatusBar(statusbar_);

    sys::SystemConfig conf;
    onyx::screen::instance().setGCInterval(conf.screenUpdateGCInterval());
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

    search_tool_ = new SearchTool(this, view_);

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
    connect(view_, SIGNAL(updateProgress(int,int)), statusbar_, SLOT(setProgress(int,int)));
    connect(statusbar_, SIGNAL(progressClicked(int,int)), this ,SLOT(onProgressClicked(const int, const int)));
    connect(view_, SIGNAL(requestTranslate()), this, SLOT(lookup()));
    updateScreen();
}

void OnyxMainWindow::closeEvent ( QCloseEvent * event )
{
}

OnyxMainWindow::~OnyxMainWindow()
{
    dict_widget_.release();
    search_widget_.release();
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
             view_->nextPage();
             updateScreen();
             return;
         }
         break;
     case Qt::Key_PageUp:
     case Qt::Key_Left:
         {
             view_->prevPage();
             updateScreen();
             return;
         }
         break;
     case Qt::Key_Up:
         {
             view_->prevChapter();
             updateScreen();
             return;
         }
     case Qt::Key_Down:
         {
             view_->nextChapter();
             updateScreen();
             return;
         }
     case Qt::Key_Return:
         {
             //gotoPage();
             ldomXPointer p=view_->getDocView()->getPageBookmark(view_->getDocView()->getCurPage());
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
                 onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU,
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
    //menu.addGroup(&reading_style_actions_);
    //menu.addGroup(&zoom_setting_actions_);
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
        return;
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
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU,
                    onyx::screen::ScreenCommand::WAIT_NONE);
        }
        else if (system == EXIT_FULL_SCREEN)
        {
            statusbar_->show();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU,
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
    tools.push_back(::ui::GOTO_PAGE);
    tools.push_back(::ui::CLOCK_TOOL);

    reading_tool_actions_.generateActions(tools, true);
}

bool OnyxMainWindow::updateActions()
{
    updateZoomingActions();
    updateToolActions();


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
    case ::ui::TOC_VIEW_TOOL:
        {
            showTableOfContents();
        }
        break;
    case ::ui::DICTIONARY_TOOL:
        {
            startDictLookup();
        }
        break;
    case ::ui::SEARCH_TOOL:
        {
            showSearchWidget();
        }
        break;

    case ::ui::TEXT_TO_SPEECH:
        {
            view_->startTTS();
        }
        break;

    case ::ui::ADD_BOOKMARK:
        addBookmark();
        view_->restoreWindowPos(this, "MyBookmark");
        updateScreen();
        break;
    case ::ui::DELETE_BOOKMARK:
        view_->deleteBookmark();
        updateScreen();
        break;
    case ::ui::SHOW_ALL_BOOKMARKS:
        showAllBookmarks();
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
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
        return;
    }

    int current = dialog.value();
    this->onProgressClicked(1, current);
}

void OnyxMainWindow::showClock()
{
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    statusbar_->onClockClicked();
}

void OnyxMainWindow::updateScreen()
{
    view_->repaint();

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
        onyx::screen::instance().updateWidgetWithGCInterval(
            this,
            NULL,
            w,
            true,
            onyx::screen::ScreenCommand::WAIT_ALL);
    }
}

void OnyxMainWindow::onProgressClicked(const int percentage, const int value)
{
    view_->getDocView()->goToPage(value-1);
    updateScreen();
}

void OnyxMainWindow::showSearchWidget()
{
    if (!search_widget_)
    {
        search_widget_.reset(new OnyxSearchDialog(this, search_context_));
        connect(search_widget_.get(), SIGNAL(search(OnyxSearchContext &)),
            this, SLOT(onSearch(OnyxSearchContext &)));
        connect(search_widget_.get(), SIGNAL(closeClicked()), this, SLOT(onSearchClosed()));
        onyx::screen::watcher().addWatcher(search_widget_.get());
    }

    search_context_.userData() = BEFORE_SEARCH;
    hideHelperWidget(dict_widget_.get());
    search_widget_->showNormal();
}

void OnyxMainWindow::onSearch(OnyxSearchContext& context)
{
    if (search_context_.userData() <= BEFORE_SEARCH)
    {
        search_tool_->setSearchPattern(context.pattern());
        search_tool_->setReverse(!context.forward());
        search_context_.userData() = IN_SEARCHING;

        // TODO
        updateSearchWidget();
    }
    else
    {
        updateSearchWidget();
    }
}

void OnyxMainWindow::onSearchClosed()
{
    //OnyxMainWindow->doAction("clearSearchResult");
    search_tool_->onCloseSearch();
    updateScreen();
}

bool OnyxMainWindow::updateSearchWidget()
{
    search_tool_->setReverse(!search_context_.forward());
    if (search_context_.forward())
    {
        if (!search_tool_->FindNext())
        {
            search_widget_->noMoreMatches();
            return false;
        }
    }
    else
    {
        if (!search_tool_->FindNext())
        {
            search_widget_->noMoreMatches();
            return false;
        }
    }
    updateScreen();
    return true;
}

void OnyxMainWindow::startDictLookup()
{
    if (!dicts_)
    {
        dicts_.reset(new DictionaryManager);
    }

    if (!dict_widget_)
    {
        dict_widget_.reset(new DictWidget(this, *dicts_, &(view_->tts())) );
        connect(dict_widget_.get(), SIGNAL(keyReleaseSignal(int)), this, SLOT(processKeyReleaseEvent(int)));
        connect(dict_widget_.get(), SIGNAL(closeClicked()), this, SLOT(onDictClosed()));
    }

    hideHelperWidget(search_widget_.get());
    hideHelperWidget(&(view_->ttsWidget()));

    // When dictionary widget is not visible, it's necessary to update the text view.
    dict_widget_->lookup(view_->getSelectionText());
    dict_widget_->ensureVisible(selected_rect_, true);
}

void OnyxMainWindow::hideHelperWidget(QWidget * wnd)
{
    if (wnd)
    {
        wnd->hide();
    }
}

void OnyxMainWindow::processKeyReleaseEvent(int key)
{
    //TODO:
    switch (key)
    {
    case Qt::Key_Escape:
        hideHelperWidget(dict_widget_.get());
        onDictClosed();
        break;
    case Qt::Key_PageDown:
        view_->nextPage();
        updateScreen();
        break;
    case Qt::Key_PageUp:
        view_->prevPage();
        updateScreen();
        break;
    }
}

void OnyxMainWindow::onDictClosed()
{
    dict_widget_.reset(0);
    search_tool_->onCloseSearch();
}

void OnyxMainWindow::lookup()
{
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    if (!dict_widget_)
    {
        startDictLookup();
    }

    adjustDictWidget();
    dict_widget_->lookup(view_->getSelectionText());
}

bool OnyxMainWindow::adjustDictWidget()
{
    if(dict_widget_.get()->isVisible())
    {
        QPoint point = view_->getSelectWordPoint();
        selected_rect_.setCoords(point.x(), point.y(), point.x()+1, point.y()+1);
    }

    return dict_widget_->ensureVisible(selected_rect_);
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
        onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
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
    onyx::screen::instance().flush();

    if (ret != QDialog::Accepted)
    {
        onyx::screen::instance().updateWidget(0);
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
    for(int i  = 0; i < list.length(); ++i, ++row)
    {
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
    }
}
