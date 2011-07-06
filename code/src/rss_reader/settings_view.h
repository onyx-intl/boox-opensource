#ifndef SETTING_VIEW_H_
#define SETTING_VIEW_H_

#include "onyx/ui/message_dialog.h"
#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/catalog_view.h"
#include "rssFeedInfo.h"


using namespace ui;

namespace rss_reader
{

class SettingsView :public OnyxDialog
{
    Q_OBJECT
public:
    SettingsView(QWidget *parent, QVector<CRSSFeedInfo * > & rss);
    virtual ~SettingsView();
    void updateView();

    int popup();

private Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item, int user_data);

Q_SIGNALS:
    void reloadFeedList();
    void addFeed(const CRSSFeedInfo & feedInfo);
    void removeFeed(int i);

protected:
    bool event(QEvent * event);
    void paintEvent(QPaintEvent * event);
    void resizeEvent ( QResizeEvent * event ) ;
    void keyReleaseEvent(QKeyEvent *ke);
    void mouseReleaseEvent(QMouseEvent * event);

private:
    void createLayout();
    void createListView();
    void createButtonView();

private:
    QVBoxLayout v_layout_;
    QHBoxLayout h_layout_;

    QLabel label_title_;

    QLabel label_arrow_left_;
    QLabel label_arrow_right_;

    CatalogView list_view_;
    ODatas list_data_;

    CatalogView button_view_;
    ODatas button_data_;

    QVector<CRSSFeedInfo * >  & rss_feeds_;

    int num_selected_;
};

class DeleteRssDialog : public MessageDialog
{
    Q_OBJECT

public:
    DeleteRssDialog(const QString & rssName, QWidget *parent = 0);
    ~DeleteRssDialog (void);
};

};
#endif // SETTING_VIEW_H_
