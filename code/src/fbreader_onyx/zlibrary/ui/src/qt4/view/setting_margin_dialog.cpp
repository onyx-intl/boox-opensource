#include "setting_margin_dialog.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/keyboard_navigator.h"
#include "onyx/data/data_tags.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/sys/sys_status.h"

struct ItemStruct
{
    const char * title;
    int size;
};

static const char* SCOPE = "pm";
static const int ITEM_HEIGHT = 60;
static const QString TITLE_INDEX = "title_index";
static const ItemStruct MARGINS[4]={ {"hide margin", 0},
                                     {"small", 8},
                                     {"medium", 30},
                                     {"big", 60}
                                      };
static const int DISPLAY_COUNT = sizeof(MARGINS) / sizeof(ItemStruct);

MarginSettingDialog::MarginSettingDialog(int value, QWidget *parent)
    : OnyxDialog(parent)
    , ver_layout_(&content_widget_)
    , margin_(0)
    , hor_layout_(0)
{
    setModal(true);
    resize(200, 300);
    setSelectedValue(value);
    createLayout();
    onyx::screen::watcher().addWatcher(this);
}

MarginSettingDialog::~MarginSettingDialog(void)
{
}

int MarginSettingDialog::exec()
{
    shadows_.show(true);
    show();

    cancel_.setFocus();
    cancel_.setFocusTo(0, 0);

    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(
        0,
        outbounding(parentWidget()),
        onyx::screen::ScreenProxy::GC,
        false,
        onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void MarginSettingDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void MarginSettingDialog::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
        break;
    case Qt::Key_Return:
        break;
    case Qt::Key_Escape:
        reject();
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        break;
    }
}

void MarginSettingDialog::createLayout()
{
    content_widget_.setBackgroundRole(QPalette::Button);

    updateTitle(QApplication::tr("Page Margin Setting"));

    // The big layout.
    ver_layout_.setContentsMargins(SPACING, 0, SPACING, 0);
    ver_layout_.addSpacing(10);

    // Create display items
    buttons_.setSubItemType(CheckBoxView::type());
    buttons_.setMargin(2, 2, 2, 2);
    buttons_.setPreferItemSize(QSize(0, ITEM_HEIGHT));
    ODatas d;

    for(int i=0; i < 4; ++i)
    {
        OData * item = new OData;
        item->insert(TAG_TITLE, qApp->translate(SCOPE, MARGINS[i].title));
        item->insert(TITLE_INDEX, MARGINS[i].size);
        if (MARGINS[i].size == selected_value_)
        {
            item->insert(TAG_CHECKED, true);
        }
        d.push_back(item);
    }

    buttons_.setData(d, true);
    buttons_.setMinimumHeight( (ITEM_HEIGHT+2)*d.size());
    buttons_.setFixedGrid(d.size(), 1);
    buttons_.setSpacing(3);
    QObject::connect(&buttons_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
                     this, SLOT(onButtonChanged(CatalogView *, ContentView *, int)), Qt::QueuedConnection);

    ver_layout_.addWidget(&buttons_);


    // OK cancel buttons.
    cancel_.setSubItemType(ui::CoverView::type());
    cancel_.setPreferItemSize(QSize(100, 60));
    ODatas d2;

    OData * item = new OData;
    item->insert(TAG_TITLE, tr("Cancel"));
    item->insert(TITLE_INDEX, 1);
    d2.push_back(item);


    cancel_.setData(d2, true);
    cancel_.setMinimumHeight( 60 );
    cancel_.setMinimumWidth(100);
    cancel_.setFocusPolicy(Qt::TabFocus);
    cancel_.setNeighbor(&buttons_, CatalogView::UP);
    connect(&cancel_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)), this, SLOT(onCancelClicked()));

    hor_layout_.addStretch(0);
    hor_layout_.addWidget(&cancel_);


    ver_layout_.addStretch(0);
    ver_layout_.addLayout(&hor_layout_);
    ver_layout_.addSpacing(8);
}

void MarginSettingDialog::onButtonChanged(CatalogView *catalog, ContentView *item, int user_data)
{
    if (!item || !item->data())
    {
        return;
    }

    OData *selected = item->data();
    margin_ = selected->value(TITLE_INDEX).toInt();
    accept();
}

bool MarginSettingDialog::event(QEvent* qe)
{
    bool ret = QDialog::event(qe);
    if (qe->type() == QEvent::UpdateRequest
            && onyx::screen::instance().isUpdateEnabled())
    {
         onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW);
    }
    return ret;
}

void MarginSettingDialog::onCancelClicked()
{
    reject();
}

void MarginSettingDialog::setSelectedValue(int value)
{
    int i;
    for(i=0; i < DISPLAY_COUNT; ++i)
    {
        if (MARGINS[i].size >= value)
        {
            break;
        }
    }
    i = (i < DISPLAY_COUNT)? i :  DISPLAY_COUNT-1;
    selected_value_ = MARGINS[i].size;
}
