#ifndef ZL_FILE_LIST_DIALOG_H_
#define ZL_FILE_LIST_DIALOG_H_

#include "onyx/ui/onyx_keyboard_utils.h"
#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/catalog_view.h"

using namespace ui;

class ZLFileListDialog: public OnyxDialog
{
    Q_OBJECT

public:
    ZLFileListDialog(const QStringList &file_list, int current_file,
            bool forward, QWidget *parent = 0);
    ~ZLFileListDialog();

    inline int selectedFile() { return selected_file_; }

    bool popup();

private Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item, int user_data);

private:
    void createLayout();
    void createFileList();

    int getFocusFile();

private:
    QVBoxLayout layout_;
    OnyxLabel description_label_;
    CatalogView file_list_view_;

    ODatas file_list_datas_;

    QStringList file_list_;
    int current_file_;
    int selected_file_;
    bool forward_;
};

#endif
