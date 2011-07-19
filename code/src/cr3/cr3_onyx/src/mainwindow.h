#ifndef OnyxMainWindow_H
#define OnyxMainWindow_H

#include <QtGui/QMainWindow>
#include <QImage>
#include "cr3widget.h"
#include "onyx/ui/status_bar.h"
#include "onyx/ui/system_actions.h"
#include "onyx/ui/reading_tools_actions.h"
#include "onyx/ui/font_actions.h"
#include "onyx/ui/font_family_actions.h"
#include "onyx/ui/reading_style_actions.h"
#include "onyx/ui/zoom_setting_actions.h"
#include "onyx/ui/onyx_search_dialog.h"
#include "onyx/dictionary/dictionary_manager.h"
#include "onyx/dictionary/dict_widget.h"

class QKeyEvent;
class SearchTool;
using namespace ui;

class OnyxMainWindow : public QMainWindow, public PropsChangeCallback
{
    Q_OBJECT

public:
    virtual void onPropsChange( PropsRef props );
    OnyxMainWindow(QWidget *parent = 0);
    ~OnyxMainWindow();

public slots:
    void contextMenu( QPoint pos );
    void on_actionFindText_triggered();

protected:
    virtual void showEvent ( QShowEvent * event );
    virtual void focusInEvent ( QFocusEvent * event );
    virtual void closeEvent ( QCloseEvent * event );
    virtual void keyPressEvent(QKeyEvent *ke);
    virtual void keyReleaseEvent(QKeyEvent *ke);

private slots:
    void showContextMenu();
    void onProgressClicked(const int, const int);
    void onSearchClosed();
    void onSearch(OnyxSearchContext& context);
    void lookup();
    void onDictClosed();
    bool addBookmark();
    void processKeyReleaseEvent(int key);
    void updateScreen();

private:
    CR3View *view_;
    StatusBar *statusbar_;
    SearchTool *search_tool_;
    QString _filenameToOpen;

    QFont select_font;

    SystemActions system_actions_;
    ReadingToolsActions reading_tool_actions_;
    ZoomSettingActions zoom_setting_actions_;
    FontFamilyActions font_family_actions_;
    FontActions font_actions_;
    ReadingStyleActions reading_style_actions_;

    PropsRef props_ref;
    QRect selected_rect_;
    scoped_ptr<DictionaryManager> dicts_;
    scoped_ptr<DictWidget> dict_widget_;
    scoped_ptr<OnyxSearchDialog> search_widget_;
    OnyxSearchContext search_context_;

    void toggleProperty( const char * name );
    bool isFullScreenByWidgetSize();
    void processToolActions();
    void showClock();
    void gotoPage();
    void showSearchWidget();
    bool updateSearchWidget();
    void showTableOfContents();
    QStandardItem * searchParent(const int index,
                                 std::vector<int> & entries,
                                 std::vector<QStandardItem *> & ptrs,
                                 QStandardItemModel &model);
    void showAllBookmarks();
    void bookmarkModel(QStandardItemModel & model,
                       QModelIndex & selected);

    void startDictLookup();
    void hideHelperWidget(QWidget * wnd);
    bool adjustDictWidget();

    void updateZoomingActions();
    void updateToolActions();
    bool updateActions();
    const QFont & currentFont();
};

#endif // OnyxMainWindow_H
