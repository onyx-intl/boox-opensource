#ifndef OnyxMainWindow_H
#define OnyxMainWindow_H

#include <QtGui/QMainWindow>
#include "cr3widget.h"
#include "onyx/ui/status_bar.h"
#include "onyx/ui/system_actions.h"
#include "onyx/ui/reading_tools_actions.h"
#include "onyx/ui/font_actions.h"
#include "onyx/ui/font_family_actions.h"
#include "onyx/ui/reading_style_actions.h"
#include "onyx/ui/zoom_setting_actions.h"

class QKeyEvent;
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
/*
    void on_actionNextPage3_triggered();
    void on_actionToggleEditMode_triggered();
    void on_actionRotate_triggered();
    void on_actionFileProperties_triggered();
    void on_actionShowBookmarksList_triggered();
    void on_actionAddBookmark_triggered();
    void on_actionAboutCoolReader_triggered();
    void on_actionAboutQT_triggered();
    void on_actionCopy2_triggered();
    void on_actionCopy_triggered();
    void on_actionSettings_triggered();
    void on_actionRecentBooks_triggered();
    void on_actionTOC_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionZoom_In_triggered();
    void on_actionToggle_Full_Screen_triggered();
    void on_actionToggle_Pages_Scroll_triggered();
    void on_actionPrevChapter_triggered();
    void on_actionNextChapter_triggered();
    void on_actionForward_triggered();
    void on_actionBack_triggered();
    void on_actionLastPage_triggered();
    void on_actionFirstPage_triggered();
    void on_actionPrevLine_triggered();
    void on_actionNextLine_triggered();
    void on_actionPrevPage_triggered();
    void on_actionNextPage_triggered();
    void on_actionPrevPage2_triggered();
    void on_actionNextPage2_triggered();
    void on_actionClose_triggered();
    void on_actionMinimize_triggered();
    void on_actionOpen_triggered();
    void on_actionExport_triggered();
    void on_view_destroyed();
*/
    void showContextMenu();

private:
    CR3View *view_;
    StatusBar *statusbar_;
    QString _filenameToOpen;

    QFont select_font;

    SystemActions system_actions_;
    ReadingToolsActions reading_tool_actions_;
    ZoomSettingActions zoom_setting_actions_;
    FontFamilyActions font_family_actions_;
    FontActions font_actions_;
    ReadingStyleActions reading_style_actions_;

    PropsRef props_ref;


    void toggleProperty( const char * name );
    bool isFullScreenByWidgetSize();
    void processToolActions();
    void showClock();
    void gotoPage();
    void updateScreen();

    void updateZoomingActions();
    void updateToolActions();
    bool updateActions();
    const QFont & currentFont();
};

#endif // OnyxMainWindow_H
