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
#include "../lcl_ui/advanced_actions.h"
#include "onyx/data/configuration.h"

class QKeyEvent;

class OnyxMainWindow : public QMainWindow, public PropsChangeCallback
{
    Q_OBJECT

  public:
    virtual void onPropsChange( PropsRef props );
    OnyxMainWindow(QWidget *parent = 0);
    ~OnyxMainWindow();

    void updateScreenManually();

  public slots:
    void on_actionFindText_triggered();

  protected:
    virtual void showEvent ( QShowEvent * event );
    virtual void focusInEvent ( QFocusEvent * event );
    virtual void closeEvent ( QCloseEvent * event );
    virtual void keyPressEvent(QKeyEvent *ke);
    virtual void keyReleaseEvent(QKeyEvent *ke);

  private slots:
    void popupMenu();
    void onProgressClicked(const int, const int);
    bool addBookmark();
    bool addCite();
    void updateScreen();
    void onScreenSizeChanged(int);
    void ableGoToPage();

  private:
    void toggleProperty( const char * name );
    bool isFullScreenByWidgetSize();
    void processToolActions();
    void processAdvancedActions();
    void showClock();
    void gotoPage();
    void showTableOfContents();
    void setLineHeight(const unsigned int lineHeightPercentage);

    bool loadDocumentOptions(const QString &);
    bool saveDocumentOptions(const QString &path);
    void storeThumbnail();
    void showAllBookmarks();
    void showAllCites();
    void bookmarkModel(QStandardItemModel & model,
                       QModelIndex & selected);
    void citeModel(QStandardItemModel & model,
                       QModelIndex & selected);
    QStandardItem * searchParent(const int index,
                                 std::vector<int> & entries,
                                 std::vector<QStandardItem *> & ptrs,
                                 QStandardItemModel &model);

    void updateReadingStyleActions();
    void updateToolActions();
    bool updateActions();
    const QFont & currentFont();

    CR3View *view_;
    ui::StatusBar *status_bar_;

    QString file_name_to_open_;

    QFont select_font_;
    vbf::Configuration conf_;

    ui::SystemActions system_actions_;
    ui::ReadingToolsActions reading_tool_actions_;
    ui::ZoomSettingActions zoom_setting_actions_;
    ui::FontFamilyActions font_family_actions_;
    ui::FontActions font_actions_;
    ui::ReadingStyleActions reading_style_actions_;
    ui::AdvancedActions advanced_actions_;

    PropsRef props_ref_;
    bool able_go_to_page_;
};

#endif // OnyxMainWindow_H
