#include <QDesktopServices>

#include "rss_reader_view.h"
#include "rss_view.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/data/data_tags.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/sys/sys_utils.h"
#include "onyx/sys/sys_status.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/wireless/connecting_dialog.h"

#include "rssLoaderThread.h"
#include "rssFeedInfo.h"
#include "feedStore.h"

#include "settings_view.h"

using namespace ui;

namespace rss_reader
{

static const QString BUTTON_STYLE =   "\
QPushButton                             \
{                                       \
    background-color: QLinearGradient(  \
    x1: 0, y1: 0, x2: 0, y2: 1,         \
    stop: 0 #ffffff, stop: 0.1 #ffffff, \
    stop: 0.49 #ffffff, stop: 0.5 #ffffff,   \
    stop: 1 #ffffff);                   \
    font-size: 20px, bold;              \
    border-width: 1px;                  \
    border-color: black;                \
    border-style: solid;                \
    color: black;                       \
    padding: 0px;                       \
    min-width: 130px;                   \
    min-height: 32px;                   \
}                                       \
QPushButton:pressed                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    color: white;                       \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:checked                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    color: white;                       \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:focus {                     \
    border:3px solid black;             \
}                                       \
QPushButton:disabled                    \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: dark;                 \
    color: dark;                        \
    background-color: white;            \
}";

QString TAG_ROW = "row";
QString TAG_POINTER = "pointer";
static const int SPACING = 2;
extern const int ROWS = 7;

enum {NONE, UPDATE_RSS, SETTINGS};

int defaultItemHeight()
{   
    return 36;
}

RssFactory factory;

RssReaderView::RssReaderView(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , vbox_(this)
    , title_layout_(0)
    , list_view_(&factory, this)
    , button_view_(&factory, this)
    , status_bar_(this, MENU | PROGRESS  | MESSAGE | CONNECTION | BATTERY)
    , AppDir(MyConf::getConfDir())
    , CancelWindow(0)
    , ExitOnDone(false)
{
    setWindowTitle(tr("News Feeds"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    if (!AppDir.exists())
    {
        AppDir.mkpath(MyConf::getConfDir());
    }

    loadFeedList();

    connect(&status_bar_,  SIGNAL(progressClicked(const int, const int)),
            this, SLOT(onProgressClicked(const int, const int)));

    wifi_dialog = new WifiDialog(this, sys::SysStatus::instance());
    connect(&sys::SysStatus::instance().connectionManager(), SIGNAL(wpaStateChanged(bool)), this,SLOT(onWpaStateChanged(bool)));
    connect(&sys::SysStatus::instance().connectionManager(), SIGNAL(passwordRequired(WifiProfile)), this, SLOT(onPasswordRequired(WifiProfile)));
    connect(&sys::SysStatus::instance().connectionManager(), SIGNAL(noMatchedAP()), this, SLOT(onNoMatchedAP()));
    connect(&sys::SysStatus::instance().connectionManager(),
            SIGNAL(connectionChanged(WifiProfile, WpaConnection::ConnectionState)),
            this,
            SLOT(onConnectionChanged(WifiProfile, WpaConnection::ConnectionState)));


    createLayout();
    createListView();
    createButtonView();

    Loader=new CRSSLoaderThread();
    Loader->moveToThread(Loader);
    Loader->start();

    connectWithChildren();
    onyx::screen::watcher().addWatcher(this);

#ifdef BUILD_FOR_ARM
    QDesktopServices::setUrlHandler("file", this, "OnOpenUrl");
    QDesktopServices::setUrlHandler("http", this, "OnOpenUrl");
    QDesktopServices::setUrlHandler("https", this, "OnOpenUrl");
#endif

}

RssReaderView::~RssReaderView()
{
    Loader->quit();
    Loader->wait();
    delete Loader;

    saveFeedList();
}

void RssReaderView::updateOnStart()
{
    NotifyDialog dialog(this);
    if (dialog.exec() == QMessageBox::Yes)
    {
        onStartUpdate();
    }
}

void RssReaderView::OnOpenUrl(QUrl url)
{
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU, onyx::screen::ScreenCommand::WAIT_ALL);

    sys::SysStatus::instance().setSystemBusy(true);
    QProcess::execute(QString("web_browser \"")+url.toString()+"\"");
    sys::SysStatus::instance().setSystemBusy(false);

    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU, onyx::screen::ScreenCommand::WAIT_ALL);
}

void RssReaderView::onReloadFeedList()
{
    clearDatas(list_data_);
    for (int i = 0; i < rss_feeds_.size(); i++)
    {
            delete rss_feeds_.at(i);
    }
    rss_feeds_.clear();

    QDir myDir(MyConf::getStoreDir());

    QStringList arguments;
    arguments << "-rf" <<  myDir.absoluteFilePath(DATASTOREDIR);
    sys::runScriptBlock("rm", arguments);

#ifdef BUILD_FOR_ARM
    QFile file("/etc/NewsFlash.ini");
    if (file.exists())
    {
        QFile::remove(AppDir.absoluteFilePath(CONFIGFILE));
        file.copy(AppDir.absoluteFilePath(CONFIGFILE));
    }
#else
    QFile::remove(AppDir.absoluteFilePath(CONFIGFILE));
    QFile::copy("build_sys/rootfs_patch/etc/NewsFlash.ini", AppDir.absoluteFilePath(CONFIGFILE));
#endif

    loadFeedList();
    createListView();
}

void RssReaderView::onAddFeed(const CRSSFeedInfo & feedInfo)
{
    rss_feeds_.push_back(new CRSSFeedInfo(feedInfo));
    clearDatas(list_data_); 
    createListView();
}

void RssReaderView::onRemoveFeed(int i)
{
    if (i >=0 &&  i < rss_feeds_.size())
    {
        CRSSFeedInfo * feedInfo = rss_feeds_.at(i);

        QUrl feedURL(feedInfo->Url);

        rss_feeds_.remove(i);
        delete feedInfo;

        clearDatas(list_data_); 
        createListView();

        CFeedStore feedStore;
        if (feedStore.Initialize(feedURL)==CFeedStore::ssSucceeded)
        {
            feedStore.ClearItems();
        }
    }
}

bool RssReaderView::loadFeedList()
{
    CRSSFeedInfo* newFeedInfo;

    QSettings settings(AppDir.absoluteFilePath(CONFIGFILE), QSettings::IniFormat, this);
    settings.setIniCodec("UTF-8");
    QStringList groups = settings.childGroups();

    if (groups.length()==0)
    {
            qDebug() << "Error reading config file";
            return false;
    }

    QStringListIterator groupsI(groups);
    while (groupsI.hasNext())
    {
        newFeedInfo=new CRSSFeedInfo();
        newFeedInfo->Load(&settings, groupsI.next());

        rss_feeds_.push_back(newFeedInfo);
    }

    return true;
}

void RssReaderView::saveFeedList()
{
    //-- Save the settings if this doesn't work, no problem
    //TODO
    QFile::remove(AppDir.absoluteFilePath(CONFIGFILE));

    QSettings settings(AppDir.absoluteFilePath(CONFIGFILE), QSettings::IniFormat, this);
    settings.setIniCodec("UTF-8");

    for (int i = 0; i < rss_feeds_.size(); i++)
    {
        rss_feeds_.at(i)->Save(&settings);
    }
    settings.sync();
}

void RssReaderView::onStatus(CRSSFeedInfo feed, CStatus status)
{
    OData * dd = feed.GetAssociatedListItem();
    if(dd)
    {
        setStatus(dd, status);
    }
}

void RssReaderView::setStatus(OData * dd, const CStatus & status)
{
    CRSSFeedInfo* feedInfo = (CRSSFeedInfo *) dd->value(TAG_POINTER).toInt();
    //-- Update the count properties of the FeedInfo if these have been transmitted with the status update
    if (status.GetFlags() & CStatus::sfCountUpdate)
    {
        feedInfo->NewItems=status.GetNewItems();
        feedInfo->ItemCount=status.GetItemCount();

        dd->insert(RssView::TAG_NEW_ITEMS, status.GetNewItems());
        dd->insert(RssView::TAG_ALL_ITEMS, status.GetItemCount());
    }

    //-- If this is the finished-message, set the last-updated timestamp
    if (status.GetFlags() & CStatus::sfFinished)
    {
        feedInfo->LastUpdateTimestamp=QDateTime::currentDateTime().toTimeSpec(Qt::UTC);	//All times are handled in UTC!
        dd->insert(RssView::TAG_UPDATE_TIME, QDateTime::currentDateTime());
    }

    if (status.GetFlags() & CStatus::sfFailed)
    {
        feedInfo->LastUpdateTimestamp=QDateTime::currentDateTime().toTimeSpec(Qt::UTC);	//All times are handled in UTC!
        dd->insert(RssView::TAG_UPDATE_TIME, QDateTime::currentDateTime());
    }

    // TODO
    list_view_.update();
}

void RssReaderView::onDone()
{
    if (CancelWindow)
    {
        CancelWindow->hide();
        delete CancelWindow;
        CancelWindow=0;
    }

    if (ExitOnDone)
    {
        close(true);
    }
    else
    {
        saveFeedList();
    }
}

void RssReaderView::onStartUpdate()
{
    //-- Only start downloading if it is not already running
    if (!Loader->IsBusy())
    {
        //-- Reset the exit on done flag if it is already set
        ExitOnDone=false;

        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
        startWifi();

        for (int i = 0; i < rss_feeds_.size(); i++)
        {
            if (rss_feeds_.at(i)->Selected)
            {
                // TODO
                // rss_feeds_.at(i)->ResetStatus();
                enqueueListItem(*rss_feeds_.at(i));

                OData * dd = rss_feeds_.at(i)->GetAssociatedListItem();
                dd->insert(RssView::TAG_UPDATE_TIME, QDateTime());
            }
        }
    }

    list_view_.update();
}

void RssReaderView::onSettings()
{
    SettingsView view(this, rss_feeds_);
    update();
    onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GC);

    connect(&view, SIGNAL(reloadFeedList()), this, SLOT(onReloadFeedList()));
    connect(&view, SIGNAL(addFeed(const CRSSFeedInfo &)), this, SLOT(onAddFeed(const CRSSFeedInfo &)));
    connect(&view, SIGNAL(removeFeed(int)), this, SLOT(onRemoveFeed(int)));
    view.popup();

    // TODO
    createListView();
}

void RssReaderView::enqueueListItem(const CRSSFeedInfo & feedInfo)
{
    emit sendUpdateFeed(feedInfo);
}

void RssReaderView::startWifi()
{
    static bool tried = false;

    if(!tried)
    {
        QProcess cmd;
        cmd.start("ping -c 2 8.8.8.8");
        cmd.waitForFinished();
        if(cmd.exitCode())
        {
            configNetwork();
        }
        tried = true;
    }

    //sys::SysStatus::instance().connectionManager().start();
}

void RssReaderView::closeEvent(QCloseEvent* event)
{
    saveFeedList();
    event->accept();
}

void RssReaderView::loadSettings()
{
}

void RssReaderView::saveSettings()
{
}

void RssReaderView::createLayout()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    vbox_.setSpacing(0);
    vbox_.setContentsMargins(0, 0, 0, 0);

    title_layout_.setSpacing(2);
    title_layout_.setContentsMargins(5, 5, 5, 5);

    label_desktop_.setPixmap(QPixmap(":/rss_images/desktop.png"));
    title_layout_.addWidget(&label_desktop_, 0, Qt::AlignTop);
    title_layout_.addStretch(0);

    QFont font(label_text_.font());
    font.setPointSize(30);
    label_text_.setFont(font);
    label_text_.setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    label_text_.setStyleSheet("QLabel {color: white}");
    label_text_.setText(tr("News Feeds"));
    title_layout_.addWidget(&label_text_, 0, Qt::AlignCenter);

    title_layout_.addStretch(0);

    label_thumbnail_.setPixmap(QPixmap(":/rss_images/thumbnail.png"));
    // title_layout_.addWidget(&label_thumbnail_, 0, Qt::AlignTop);
    label_list_.setPixmap(QPixmap(":/rss_images/list_view.png"));
    // title_layout_.addWidget(&label_list_, 0, Qt::AlignTop);

    vbox_.addLayout(&title_layout_, 0);

    vbox_.addSpacing(30);
    vbox_.addWidget(&list_view_, 100, 0);
    vbox_.addStretch(0);
    vbox_.addSpacing(100);

    vbox_.addLayout(&button_layout_, 0);

    button_layout_.addStretch(0);
    button_layout_.addWidget(&button_view_);

    vbox_.addSpacing(20);
    vbox_.addWidget(&status_bar_);

    // Connections.
    connect(&status_bar_, SIGNAL(menuClicked()), this, SLOT(popupMenu()));

}

void RssReaderView::createListView()
{
    const int height = defaultItemHeight() + 4 * SPACING;
    list_view_.setSubItemType(RssView::type());
    list_view_.setPreferItemSize(QSize(-1, height));

    list_data_.clear();

    ODatas   selected_data;
    for (int i = 0; i < rss_feeds_.size(); i++)
    {
        CRSSFeedInfo * info = rss_feeds_.at(i);

        OData *dd = new OData;
        dd->insert(TAG_TITLE, info->Title);
        dd->insert(TAG_URL, info->Url.toString());
        dd->insert(RssView::TAG_NEW_ITEMS, info->NewItems);
        dd->insert(RssView::TAG_ALL_ITEMS, info->ItemCount);
        dd->insert(RssView::TAG_UPDATE_TIME, info->LastUpdateTimestamp);
        dd->insert(TAG_ROW, i);
        list_data_.push_back(dd);

        dd->insert(TAG_POINTER, (int)info);
        info->AssociateListItem(dd);

        if (info->Selected && selected_data.size() < ROWS)
        {
            selected_data.push_back(dd);
        }
    }

    list_view_.setSpacing(2);
    list_view_.setData(selected_data);
    list_view_.setNeighbor(&button_view_, CatalogView::DOWN);
}

void RssReaderView::createButtonView()
{
    const static QSize MENU_ITEM_SIZE = QSize(60, 60);

    button_view_.setSubItemType(ButtonView::type());

    button_view_.setPreferItemSize(MENU_ITEM_SIZE);

    OData * dd = new OData;
    dd->insert(TAG_TITLE, tr("Update rss-feeds"));
    dd->insert(TAG_ID, UPDATE_RSS);
    button_data_.push_back(dd);

    dd = new OData;
    dd->insert(TAG_TITLE, tr("Settings"));
    dd->insert(TAG_ID, SETTINGS);
    button_data_.push_back(dd);

    button_view_.setSpacing(2);
    button_view_.setFixedGrid(1, 2);
    button_view_.setData(button_data_);
    button_view_.setSearchPolicy(CatalogView::NeighborFirst
            | CatalogView::AutoHorRecycle);
    button_view_.setNeighbor(&list_view_, CatalogView::RECYCLE_UP);
}

void RssReaderView::reArrageDatas()
{
    ODatas   selected_data;
    for (int i = 0; i < rss_feeds_.size(); i++)
    {
        CRSSFeedInfo * info = rss_feeds_.at(i);
        if (info->Selected && selected_data.size() < ROWS)
        {
            OData * dd = info->GetAssociatedListItem();
            selected_data.push_back(dd);
        }
    }

    list_view_.setData(selected_data);
}

void RssReaderView::connectWithChildren()
{
    connect(&list_view_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&button_view_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));

    connect(this, SIGNAL(sendUpdateFeed(CRSSFeedInfo)), Loader, SLOT(OnUpdateFeed(CRSSFeedInfo)), Qt::QueuedConnection);
    connect(Loader, SIGNAL(sendStatus(CRSSFeedInfo, CStatus)), this, SLOT(onStatus(CRSSFeedInfo, CStatus)), Qt::QueuedConnection);
    connect(Loader, SIGNAL(sendDone()), this, SLOT(onDone()), Qt::QueuedConnection);
}

void RssReaderView::popupMenu()
{
    ui::PopupMenu menu(this);

    NetworkActions network_actions_;
    std::vector<ui::NetworkType> networks;
    networks.push_back(ui::NETWORK_WIFI);
    network_actions_.generateActions(networks);

    menu.addGroup(&network_actions_);
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == network_actions_.category())
    {
        if (network_actions_.selected() == ui::NETWORK_WIFI ||
            network_actions_.selected() == ui::NETWORK_WCDMA)
        {
            configNetwork();
        }
    }
}

void RssReaderView::onProgressClicked(const int percent, const int value)
{
}


void RssReaderView::close(bool)
{
    hide();
    saveSettings();
    saveFeedList();

    qApp->exit();
}

void RssReaderView::mouseReleaseEvent(QMouseEvent * event)
{
    if (label_desktop_.geometry().contains(event->pos()))
    {
        onDesktopClicked();
        event->accept();
    }
}

void RssReaderView::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
    switch(ke->key())
    {
        case Qt::Key_PageDown:
            break;
        case Qt::Key_PageUp:
            break;
        default:
            break;
    }
}

void RssReaderView::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    switch(ke->key())
    {
        case Qt::Key_Escape:
            ExitApp(false);
            break;
        case ui::Device_Menu_Key:
            popupMenu();
            break;
        default:
            break;
    }
}

void RssReaderView::ExitApp(bool exitOnDone)
{
    if (Loader->IsBusy())
    {
        cancelLoader(exitOnDone);
        ExitOnDone=exitOnDone;
    }
    else
    {
        close(true);
    }
}

void RssReaderView::cancelLoader(bool showExit)
{
    //-- If the loader is already running or we're already canceling
    if (Loader->IsBusy()&&(!Loader->Cancel)) //&&(CancelWindow==NULL))
    {
        Loader->Cancel=true;
        CancelWindow=new CCancelWindow(this, (showExit?tr("Exiting, please wait..."):tr("Canceling, please wait...")));
        CancelWindow->show();
    }
}

bool RssReaderView::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::UpdateRequest &&
        onyx::screen::instance().isUpdateEnabled() && isActiveWindow())
    {
        static int count = 0;
        qDebug("Update request %d", ++count);
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW);
        event->accept();
        return true;
    }
    return ret;
}

void RssReaderView::paintEvent(QPaintEvent * event)
{
    label_text_.setText(tr("News Feeds"));

    QPainter p(this);

    QRect r(0, 0, rect().width(), 70);
    p.fillRect(r, Qt::black);
}

void RssReaderView::resizeEvent(QResizeEvent * event)
{
    QSize s = qApp->desktop()->screenGeometry().size();
#ifndef Q_WS_QWS
    setFixedSize(600, 800);
#else
    setFixedSize(s);
#endif

    // list_view_.setFixedHeight(500);
    list_view_.setFixedGrid(7, 1);

    button_view_.setSpacing(20);
    button_view_.setContentsMargins(0, 0, 20, 0);
    button_view_.setFixedHeight(40);
    button_view_.setFixedWidth(450);
    // button_view_.visibleSubItems().at(0)->setFixedWidth(330);
    // button_view_.visibleSubItems().at(1)->setFixedWidth(70);
    ((QGridLayout *) button_view_.layout())->setColumnStretch(0, 140);
    ((QGridLayout *) button_view_.layout())->setColumnStretch(1, 70);
}

void RssReaderView::onItemActivated(CatalogView *catalog,
                                   ContentView *item,
                                   int user_data)
{
    OData * item_data = item->data();
    if (item_data->contains(TAG_ROW))
    {
        if (user_data == RssView::UPDATE)
        {
            ExitOnDone = false;

            startWifi();

            int i =  item_data->value(TAG_ROW).toInt(); 
            enqueueListItem(*rss_feeds_.at(i));

            OData * dd = rss_feeds_.at(i)->GetAssociatedListItem();
            dd->insert(RssView::TAG_UPDATE_TIME, QDateTime());

            item->updateView();
        }
        else
        {
            QString str = item_data->value(TAG_URL).toString();
            if (openInBrowser(str))  
            { 
                markRead();
            }
            // update();
            // onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
        }
    }
    else
    {
        int id = item_data->value(TAG_ID).toInt();
        if (id == UPDATE_RSS)
        {
            NotifyDialog dialog(this);

            if (dialog.exec() == QMessageBox::Yes)
            {
                onStartUpdate();
            }
        }
        else if(id == SETTINGS)
        {
            onSettings();
        }
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, false, onyx::screen::ScreenCommand::WAIT_ALL);
    }
}

bool RssReaderView::openInBrowser(const QString & str)
{
    CFeedStore feedStore;

    QUrl feedURL(str);

    if (feedStore.Initialize(feedURL)==CFeedStore::ssSucceeded)
    {
        return QDesktopServices::openUrl(feedStore.GetLocalFileUrl("index.html"));
    }
    return false;
}

void RssReaderView::markRead()
{
}

void RssReaderView::onDesktopClicked()
{
    close(true);
}

void RssReaderView::onReportWifiNetwork(const int signal, const int total, const int network)
{
    if (signal <= 0)
    {
        //sys::SysStatus::instance().popupWifiDialog();
    }
}

void RssReaderView::onWpaStateChanged(bool running)
{
    if (!running)
    {
        configNetwork();
    }
    else
    {
        // TODO.
    }
}

void RssReaderView::onPasswordRequired(WifiProfile profile)
{
    configNetwork();
}

void RssReaderView::onNoMatchedAP()
{
    configNetwork();
}

void RssReaderView::onConnectionChanged(WifiProfile profile, WpaConnection::ConnectionState state)
{
    //Todo
}

void RssReaderView::configNetwork()
{
    QString type = sys::SysStatus::instance().connectionType();
    if (type.contains("wifi", Qt::CaseInsensitive))
    {
        if (!wifi_dialog->isVisible())
        {
            wifi_dialog->popup(true);
        }
        else
        {
            return;
        }
    }
    else if (type.contains("3g", Qt::CaseInsensitive))
    {
        ConnectingDialog tg_dialog(0, SysStatus::instance());
        if(qgetenv("CONNECT_TO_DEFAULT_APN").toInt() > 0 &&
            sys::SysStatus::instance().isPowerSwitchOn())
        {
            sys::SysStatus::instance().connect3g("","","");
            tg_dialog.popup();
        }
        else
        {
            if (!wifi_dialog->isVisible())
            {
                wifi_dialog->popup(true);
            }
            else
            {
                return;
            }
        }
    }

    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
}

void RssReaderView::connectToNetwork()
{
    sys::SysStatus::instance().connectionManager().start();
}

NotifyDialog::NotifyDialog(QWidget *parent)
: MessageDialog(QMessageBox::Question,
                tr("confirm"),
                tr("Do you want to update rss feed?"),
                QMessageBox::Yes|QMessageBox::No,
                parent)
{
    button(QMessageBox::No)->setStyleSheet(BUTTON_STYLE);
    button(QMessageBox::Yes)->setStyleSheet(BUTTON_STYLE);

    button(QMessageBox::Yes)->setFocus();
}

NotifyDialog::~NotifyDialog (void)
{
}

}
