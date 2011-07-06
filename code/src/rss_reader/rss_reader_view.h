#ifndef RSS_READER_VIEW_H_
#define RSS_READER_VIEW_H_

#include "onyx/ui/status_bar_item_slide.h"
#include "onyx/ui/content_view.h"
#include "onyx/ui/catalog_view.h"
#include "onyx/ui/menu.h"
#include "onyx/ui/menu_item.h"
#include "onyx/ui/status_bar.h"
#include "rssLoaderThread.h"
#include "status.h"
#include "cancelWindow.h"
#include "onyx/wireless/wifi_dialog.h"

using namespace ui;

namespace rss_reader
{



class RssReaderView : public QWidget
{
    Q_OBJECT
public:
    RssReaderView(QWidget *parent = 0);
    ~RssReaderView();

    void setStatus(OData * dd, const CStatus & status);

public Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item, int user_data);
    void onStatus(CRSSFeedInfo feed, CStatus status);
	void onDone();
    void onStartUpdate();
    void onSettings();

    void OnOpenUrl(QUrl url);
    void onReloadFeedList();
    void onAddFeed(const CRSSFeedInfo & feedInfo);
    void onRemoveFeed(int i);

    void updateOnStart();

Q_SIGNALS:
    void sendUpdateFeed(CRSSFeedInfo feed);

private Q_SLOTS:
    void popupMenu();
    void onProgressClicked(const int percent, const int value);

protected:
    void paintEvent(QPaintEvent * event);
    void resizeEvent(QResizeEvent * event);

private:
    void mouseReleaseEvent(QMouseEvent * event);
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);
    bool event(QEvent * event);
    void closeEvent (QCloseEvent* event);

    void loadSettings();
    void saveSettings();

    void createLayout();
    void createListView();
    void createButtonView();
    void connectWithChildren();
    void onDesktopClicked();
    void close(bool);

    void startWifi();
    void enqueueListItem(const CRSSFeedInfo & feedInfo);

    void configNetwork();

    void markRead();
    bool openInBrowser(const QString & str);
    void saveFeedList();
    bool loadFeedList();

    void cancelLoader(bool showExit);
    void ExitApp(bool exitOnDone);

    void reArrageDatas();

private:
    QVBoxLayout             vbox_;
    QHBoxLayout             title_layout_;
    QHBoxLayout             button_layout_;

    OnyxLabel               label_desktop_;
    OnyxLabel               label_text_;
    OnyxLabel               label_thumbnail_;
    OnyxLabel               label_list_;

    CatalogView             list_view_;
    CatalogView             button_view_;

    StatusBar               status_bar_;

    ODatas                  list_data_;
    ODatas                  button_data_;

    WifiDialog *wifi_dialog;


    QDir AppDir;
    CRSSLoaderThread * Loader;
    CCancelWindow* CancelWindow;

	bool	ExitOnDone;

    QVector<CRSSFeedInfo * >  rss_feeds_;
};

class NotifyDialog : public MessageDialog
{
    Q_OBJECT

public:
    NotifyDialog(QWidget *parent = 0);
    ~NotifyDialog (void);
};

};
#endif
