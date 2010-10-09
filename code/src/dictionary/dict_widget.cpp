#include "dict_widget.h"
#include "onyx/screen/screen_proxy.h"

namespace ui
{
static const int LOOKUP = 0;
static const int DETAILS = 1;
static const int WORD_LIST = 2;
static const int DICTIONARY_LIST = 3;

DictWidget::DictWidget(QWidget *parent, DictionaryManager & dict, tts::TTS *tts)
    : OnyxDialog(parent)
    , dict_(dict)
    , tts_(tts)
    , hbox_(&content_widget_)
    , vbox1_(0)
    , vbox2_(0)
    , dict_list_button_(tr("Dictionaries"), 0)
    , lookup_button_(tr("Lookup"), 0)
    , word_list_button_(tr("Similar Words"), 0)
    , details_button_(tr("Explanation"), 0)
    , details_(0)
    , list_widget_(0, 0)
    , similar_words_offset_(0)
    , timer_(this)
    , internal_state_(-1)
    , update_parent_(false)
{
    createLayout();
    initBrowser();
    initDictionaries();

    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));

    setModal(false);
}

DictWidget::~DictWidget()
{
}

/// Ensure the region of selected word is visible. The region
/// is specified by the rectangle.
bool DictWidget::ensureVisible(const QRectF & rect,
                               bool update_parent)
{
    bool changed = false;
    shadows_.show(true);
    if (!isVisible())
    {
        changed = true;
        show();
    }

    QRect parent_rect = parentWidget()->rect();
    int border = (frameGeometry().width() - geometry().width());
    if (border == 0)
    {
        border = Shadows::PIXELS;
    }
    int width = parent_rect.width() - border * 2;
    if (size().width() != width)
    {
        changed = true;
        resize(width, height());
    }

    // Check position.
    QPoint new_pos(border, border);
    if (rect.bottom() < parent_rect.height() / 2)
    {
        new_pos.ry() = parent_rect.height() - height() - border * 2;
    }

    if (pos() != new_pos)
    {
        changed = true;
        move(new_pos);
    }

    update_parent_ = false;
    if (changed || update_parent)
    {
        update_parent_ = true;
    }
    return changed;
}

bool DictWidget::lookup(const QString &word)
{
    if (word.isEmpty())
    {
        return false;
    }

    resetSimilarWordsOffset();

    // Clean the word.
    word_ = word.trimmed();

    // Title
    QString result;
    bool ret = dict_.translate(word_, result);
    OnyxDialog::updateTitle(dict_.selected());

    // Result
    doc_.setHtml(result);

    // Always stop timer as the screen can be updated later.
    launchTimer(false);

    updateVisibleWidgets();
    onDetailsClicked(true);
    return ret;
}

void DictWidget::keyReleaseEvent(QKeyEvent *ke)
{
    QWidget * wnd = 0;
    QPushButton * btn = 0;
    int key = ke->key();
    if (key == Qt::Key_Escape || key == ui::Device_Menu_Key)
    {
        ke->accept();
        onCloseClicked();
        return;
    }

    if (internalState() == LOOKUP)
    {
        emit keyReleaseSignal(ke->key());
        return;
    }

    switch (ke->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Down:
        wnd = ui::moveFocus(this, key);
        if (wnd)
        {
            wnd->setFocus();
        }
        ke->accept();
        return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        wnd = content_widget_.focusWidget();
        btn = qobject_cast<QPushButton*>(wnd);
        if (btn != 0)
        {
             btn->click();
        }
        ke->accept();
        return ;
    default:
        break;
    }
    ke->ignore();
    emit keyReleaseSignal(ke->key());
}

/// The keyPressEvent could be sent from virtual keyboard.
void DictWidget::keyPressEvent(QKeyEvent * ke)
{

    if (ke->key() == Qt::Key_Enter)
    {
        ke->accept();
        // lookup(text_edit_.text());
        return;
    }
    else if (ke->key() == Qt::Key_Shift || ke->key() == Qt::Key_CapsLock)
    {
        ke->accept();
        return;
    }
    else if (ke->key() == Qt::Key_Return)
    {
        ke->accept();
        QWidget * wnd = content_widget_.focusWidget();
        QPushButton * btn = qobject_cast<QPushButton*>(wnd);
        if (btn != 0)
        {
            // btn->click();
        }
    }
    ke->ignore();
    /*
    // Must use flush on qws platform.
    QKeyEvent * key_event = new QKeyEvent(ke->type(), ke->key(), ke->modifiers(), ke->text());
    screen::ScreenManager::instance().enableUpdate(false);
    QApplication::postEvent(&text_edit_, key_event);
    QApplication::processEvents();
    screen::ScreenManager::instance().enableUpdate(true);

    screen::ScreenManager::instance().enableFastestUpdate(true);
    screen::ScreenManager::instance().fastUpdateWidget(&text_edit_, false);
    screen::ScreenManager::instance().enableFastestUpdate(false);
    */

    // Launch timer to make sure screen will be updated
    // launchTimer(true);
}

void DictWidget::launchTimer(bool launch)
{
    if (timer_.isActive())
    {
        timer_.stop();
    }

    if (launch)
    {
        timer_.start(1000);
    }
}

void DictWidget::updateSimilarWordsModel(int count)
{
    // Pick up similar words from current dictionary.
    similar_words_.clear();
    dict_.similarWords(word_, similar_words_, similar_words_offset_, count);
    similar_words_model_.clear();
    QStandardItem *parentItem = similar_words_model_.invisibleRootItem();
    QString explanation;
    foreach(QString item, similar_words_)
    {
        // Get explanation.
        dict_.translate(item, explanation);
        QStandardItem *ptr = new QStandardItem();
        ptr->setData(item);

        item = item.trimmed();
        explanation = explanation.trimmed();
        item += "\t";
        item += explanation;
        ptr->setText(item);
        parentItem->appendRow(ptr);
    }
}

void DictWidget::updateSimilarWords()
{
    list_widget_.clear();
    list_widget_.setFocus();
    list_widget_.show();
    updateSimilarWordsModel(list_widget_.itemsPerPage());

    // Update the list.
    list_widget_.setModel(&similar_words_model_);
}

void DictWidget::updateDictionaryListModel()
{
    int count = dict_list_model_.rowCount(dict_list_model_.invisibleRootItem()->index());
    if (count <= 0)
    {
        dict_list_model_.clear();
        QStringList list;
        dict_.dictionaries(list);
        foreach(QString item, list)
        {
            dict_list_model_.appendRow(new QStandardItem(item));
        }
    }
}

void DictWidget::updateDictionaryList()
{
    resetSimilarWordsOffset();
    updateDictionaryListModel();
    list_widget_.clear();
    list_widget_.setModel(&dict_list_model_);
    list_widget_.show();
    list_widget_.select(dict_.selected());
    list_widget_.setFocus();
}

void DictWidget::updateVisibleWidgets()
{
    if (word_list_button_.isChecked())
    {
        updateSimilarWords();
    }
    else if (dict_list_button_.isChecked())
    {
        updateDictionaryList();
    }
}

bool DictWidget::eventFilter(QObject *obj, QEvent *event)
{
    bool ret = OnyxDialog::eventFilter(obj, event);
    if (event->type() == QEvent::KeyPress)
    {

        event->ignore();
        return true;
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            keyEvent->accept();
            changeInternalState(DETAILS);
            lookup_button_.setFocus();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_PageDown ||
                 keyEvent->key() == Qt::Key_PageUp)
        {
            if (internalState() == LOOKUP)
            {
                keyEvent->ignore();
                return true;
            }
        }
    }
    return ret;
}

void DictWidget::changeInternalState(int state)
{
    internal_state_ = state;
}

bool DictWidget::event(QEvent *e)
{
    int ret = QDialog::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        onyx::screen::instance().sync(&shadows_.hor_shadow());
        onyx::screen::instance().sync(&shadows_.ver_shadow());
        if (update_parent_)
        {
            onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GC, true);
            update_parent_ = false;
        }
        else
        {
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
        }
        launchTimer(false);
        e->accept();
    }

    return ret;
}

void DictWidget::moveEvent(QMoveEvent *e)
{
    shadows_.onWidgetMoved(this);
    update_parent_ = true;
}

void DictWidget::resizeEvent(QResizeEvent *e)
{
    OnyxDialog::resizeEvent(e);
    update_parent_ = true;
}

void DictWidget::hideEvent(QHideEvent * event)
{
    QDialog::hideEvent(event);
    shadows_.show(false);
}

void DictWidget::createLayout()
{
    title_icon_label_.setPixmap(QPixmap(":/images/dictionary_title.png"));
    if (dict_.selected().isEmpty())
    {
        OnyxDialog::updateTitle(tr("Dictionary Lookup"));
    }
    else
    {
        OnyxDialog::updateTitle(dict_.selected());
    }

    // top_vbox_
    hbox_.setContentsMargins(SPACING, SPACING, SPACING, SPACING);
    hbox_.setSpacing(SPACING);
    hbox_.addLayout(&vbox1_);
    hbox_.addLayout(&vbox2_);

    // vbox 1
    const int V_SPACING = 20;
    vbox1_.setContentsMargins(0, SPACING, 0, SPACING);
    vbox1_.setSpacing(V_SPACING);
    vbox1_.addWidget(&lookup_button_);
    vbox1_.addWidget(&details_button_);
    vbox1_.addWidget(&word_list_button_);
    vbox1_.addWidget(&dict_list_button_);

    dict_list_button_.useDefaultHeight();
    lookup_button_.useDefaultHeight();
    word_list_button_.useDefaultHeight();
    details_button_.useDefaultHeight();

    // vbox 2
    vbox2_.setContentsMargins(SPACING, SPACING, SPACING, SPACING);
    vbox2_.setSpacing(SPACING);
    vbox2_.addWidget(&details_);
    vbox2_.addWidget(&list_widget_);
    list_widget_.showHeader(false);
    list_widget_.hide();

    // Setup connection.
    connect(&details_button_, SIGNAL(clicked(bool)), this,
        SLOT(onDetailsClicked(bool)), Qt::QueuedConnection);

    connect(&lookup_button_, SIGNAL(clicked(bool)), this,
            SLOT(onLookupClicked(bool)), Qt::QueuedConnection);

    connect(&word_list_button_, SIGNAL(clicked(bool)), this,
            SLOT(onWordListClicked(bool)), Qt::QueuedConnection);

    connect(&dict_list_button_, SIGNAL(clicked(bool)), this,
            SLOT(onDictListClicked(bool)), Qt::QueuedConnection);

    connect(&list_widget_, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(onItemClicked(const QModelIndex &)));

    connect(&list_widget_, SIGNAL(exceed(bool)),
            this, SLOT(moreSimilarWords(bool)));

    // Change buttons attributes.
    details_button_.setFocusPolicy(Qt::StrongFocus);
    lookup_button_.setFocusPolicy(Qt::StrongFocus);
    word_list_button_.setFocusPolicy(Qt::StrongFocus);
    dict_list_button_.setFocusPolicy(Qt::StrongFocus);

    details_.setFocusPolicy(Qt::StrongFocus);
    list_widget_.setFocusPolicy(Qt::StrongFocus);

    details_button_.setCheckable(true);
    lookup_button_.setCheckable(true);
    word_list_button_.setCheckable(true);
    dict_list_button_.setCheckable(true);

    details_button_.setChecked(true);

    // Install event filter.
    details_.installEventFilter(this);
    list_widget_.installEventFilter(this);
}

void DictWidget::initBrowser()
{
    QFont font(QApplication::font());
    font.setPointSize(20);
    doc_.setDefaultFont(font);
    details_.setDocument(&doc_);
}

void DictWidget::initDictionaries()
{
    dict_.loadDictionaries();
}

/// Use the timeout to update the screen when necessary.
/// Because we don't wait for the screen update finished, there
/// would be some dirty region on the screen.
void DictWidget::onTimeout()
{
    onyx::screen::instance().updateScreen(onyx::screen::ScreenProxy::GU);

    // Make sure the timer is stopped.
    launchTimer(false);
}

void DictWidget::onItemClicked(const QModelIndex & index)
{
    if (word_list_button_.isChecked())
    {
        QString text = similar_words_model_.itemFromIndex(index)->data().toString();
        lookup(text);
        details_button_.click();
    }
    else if (dict_list_button_.isChecked())
    {
        dict_.select(dict_list_model_.itemFromIndex(index)->text());
        lookup(word_);
        details_button_.click();
    }
}


void DictWidget::moreSimilarWords(bool begin)
{
    if (!word_list_button_.isChecked())
    {
        return;
    }

    if (begin)
    {
        similar_words_offset_ -= list_widget_.itemsPerPage();
    }
    else
    {
        similar_words_offset_ += list_widget_.itemsPerPage();
    }
    updateSimilarWords();
}

void DictWidget::onDetailsClicked(bool)
{
    changeInternalState(DETAILS);
    details_button_.setChecked(true);
    lookup_button_.setChecked(false);
    dict_list_button_.setChecked(false);
    word_list_button_.setChecked(false);
    details_.show();
    details_.setFocus();
    list_widget_.hide();
    if (!update_parent_)
    {
        onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC, false);
    }
    else
    {
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC, true);
    }
}

void DictWidget::onLookupClicked(bool)
{
    changeInternalState(LOOKUP);
    lookup_button_.setChecked(true);
    dict_list_button_.setChecked(false);
    word_list_button_.setChecked(false);
    details_button_.setChecked(false);
}

void DictWidget::onWordListClicked(bool)
{
    changeInternalState(WORD_LIST);
    lookup_button_.setChecked(false);
    dict_list_button_.setChecked(false);
    details_button_.setChecked(false);
    word_list_button_.setChecked(true);
    details_.hide();
    updateSimilarWords();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC, false);
}

void DictWidget::onDictListClicked(bool)
{
    changeInternalState(DICTIONARY_LIST);
    lookup_button_.setChecked(false);
    word_list_button_.setChecked(false);
    details_button_.setChecked(false);
    dict_list_button_.setChecked(true);
    details_.hide();
    updateDictionaryList();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC, false);
}

void DictWidget::onCloseClicked()
{
    releaseKeyboard();
    shadows_.show(false);
    hide();
    emit closeClicked();
}

}   // namespace ui

