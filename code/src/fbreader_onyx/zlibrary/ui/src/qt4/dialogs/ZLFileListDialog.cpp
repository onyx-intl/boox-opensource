#include "ZLFileListDialog.h"
#include "onyx/ui/factory.h"
#include "onyx/ui/content_view.h"
#include "onyx/data/data.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/screen/screen_update_watcher.h"


static const QString TAG_FILE_INDEX = "file_index";

ZLFileListDialog::ZLFileListDialog(const QStringList &file_list,
        int current_file, bool forward, QWidget *parent)
    : OnyxDialog(parent)
    , layout_(&content_widget_)
    , file_list_view_(0, this)
    , file_list_(file_list)
    , current_file_(current_file)
    , selected_file_(0)
    , forward_(forward)
{
    createLayout();
    setFixedSize(bestDialogSize());
//    onyx::screen::watcher().addWatcher(this);
}

ZLFileListDialog::~ZLFileListDialog()
{
    clearDatas(file_list_datas_);
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

    int size = file_list_.size();
    for (int i=0; i<size; i++)
    {
        OData *dd = new OData;
        dd->insert(TAG_TITLE, file_list_.at(i));
        dd->insert(TAG_FILE_INDEX, i);
        int alignment = Qt::AlignLeft | Qt::AlignVCenter;
        dd->insert(TAG_ALIGN, alignment);
        file_list_datas_.push_back(dd);
    }
    file_list_view_.setData(file_list_datas_);

    file_list_view_.setFixedGrid(bestDialogSize().height() / defaultItemHeight() - 2 , 1);
    file_list_view_.setSearchPolicy(CatalogView::AutoVerRecycle);
}

int ZLFileListDialog::getFocusFile()
{
    int focus_file = current_file_;
    if (forward_)
    {
        focus_file++;
        if (focus_file >= file_list_.size())
        {
            focus_file = file_list_.size() - 1;
        }
    }
    else
    {
        focus_file--;
        if (focus_file < 0)
        {
            focus_file = 0;
        }
    }
    return focus_file;
}

bool ZLFileListDialog::popup()
{
    if (isHidden())
    {
        show();
    }

    int focus_file = getFocusFile();
    OData *focus = file_list_datas_.at(focus_file);
    onyx::screen::watcher().addWatcher(this);
    file_list_view_.select(focus);

    // TODO calculate right rows here for multi-page.
    file_list_view_.setCheckedTo(current_file_, 0);

    bool ret = OnyxDialog::exec();
    onyx::screen::watcher().removeWatcher(this);
    return ret;
}

void ZLFileListDialog::onItemActivated(CatalogView *catalog,
        ContentView *item, int user_data)
{
    OData * item_data = item->data();
    selected_file_ = item_data->value(TAG_FILE_INDEX).toInt();
    accept();
}

