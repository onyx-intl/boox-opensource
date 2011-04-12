#include "ZLFileListDialog.h"
#include "onyx/ui/factory.h"
#include "onyx/ui/content_view.h"
#include "onyx/data/data.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/screen/screen_update_watcher.h"


static const QString TAG_FILE_INDEX = "file_index";

ZLFileListDialog::ZLFileListDialog(const QStringList &file_list,
        int current_file, QWidget *parent)
    : OnyxDialog(parent)
    , layout_(&content_widget_)
    , file_list_view_(0, this)
    , file_list_(file_list)
    , current_file_(current_file)
    , selected_file_(0)
{
    createLayout();
    resize(bestDialogSize());
    onyx::screen::watcher().addWatcher(this);
}

ZLFileListDialog::~ZLFileListDialog()
{
}

void ZLFileListDialog::createLayout()
{
    content_widget_.setBackgroundRole(QPalette::Button);
    updateTitle(QApplication::tr("Open File"));
    updateTitleIcon(QPixmap(":/images/small/txt.png"));

    description_label_.setText(QApplication::tr("Continue to read:"));
    description_label_.setWordWrap(true);
    layout_.setContentsMargins(SPACING, 0, SPACING, 0);
    layout_.addWidget(&description_label_, 0, Qt::AlignTop);

    createFileList();

    layout_.addWidget(&file_list_view_, 1);

    connect(&file_list_view_,
            SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this,
            SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

void ZLFileListDialog::createFileList()
{
    file_list_view_.setPreferItemSize(QSize(-1, defaultItemHeight()));

    ODatas ds;
    int size = file_list_.size();
    for (int i=0; i<size; i++)
    {
        OData *dd = new OData;
        dd->insert(TAG_TITLE, file_list_.at(i));
        dd->insert(TAG_FILE_INDEX, i);
        int alignment = Qt::AlignLeft | Qt::AlignVCenter;
        dd->insert(TAG_ALIGN, alignment);
        ds.push_back(dd);
    }
    file_list_view_.setData(ds);

    file_list_view_.setFixedGrid(bestDialogSize().height() / defaultItemHeight() - 2 , 1);
    file_list_view_.setSearchPolicy(CatalogView::AutoVerRecycle);
}

void ZLFileListDialog::onItemActivated(CatalogView *catalog,
        ContentView *item, int user_data)
{
    OData * item_data = item->data();
    selected_file_ = item_data->value(TAG_FILE_INDEX).toInt();
    accept();
}

