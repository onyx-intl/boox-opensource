#ifdef BUILD_FOR_ARM
#include <QtGui/qwsdisplay_qws.h>
#include <QtGui/qscreen_qws.h>
#endif

#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"

#include "dict_frame.h"

static const int LOOKUP = 0;
static const int DETAILS = 1;
static const int WORD_LIST = 2;
static const int DICTIONARY_LIST = 3;

static const QString TTS_BUTTON_STYLE =   "\
QPushButton                             \
{                                       \
    background: transparent;            \
    font-size: 14px;                    \
    border-width: 1px;                  \
    border-color: transparent;          \
    border-style: solid;                \
    color: black;                       \
    padding: 0px;                       \
}                                       \
QPushButton:pressed                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:checked                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:focus {                     \
    border-width: 2px;                  \
    border-color: black;                \
    border-style: solid;                \
    border-radius: 3;                   \
}                                       \
QPushButton:disabled                    \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: dark;                 \
    color: dark;                        \
    background-color: white;            \
}";

DictFrame::DictFrame(QWidget *parent, DictionaryManager & dict, tts::TTS *tts)
    : OnyxDialog(parent)
    , vlayout_(&content_widget_)
    , hlayout1_(0)
    , hlayout2_(0)
    , dict_edit_(QString(), 0)
    , details_(0)
    , list_widget_(0, 0)
    , keyboard_(0)
    , status_bar_(this, MENU | MESSAGE | BATTERY | CLOCK | SCREEN_REFRESH | INPUT_TEXT)
    , clear_button_(QApplication::tr("Clear"), 0)
    , dict_list_button_(tr("Dictionaries"), 0)
    , lookup_button_(tr("Lookup"), 0)
    , word_list_button_(tr("Similar Words"), 0)
    , details_button_(tr("Explanation"), 0)
    , speak_button_(tr(""), 0)
    , dict_mgr_(dict)
    , tts_engine_(tts)
    , internal_state_(-1)
{
#ifndef Q_WS_QWS
    resize(600, 800);
#else
    resize(qApp->desktop()->screenGeometry().size());
#endif

    createLayout();
    initBrowser();
    initDictionaries();

    connect(&status_bar_, SIGNAL(menuClicked()), this, SLOT(popupMenu()));
    connect(&status_bar_, SIGNAL(requestInputText()), this, SLOT(onInputText()));
#ifdef Q_WS_QWS
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif

    // connect the signals with sys_state_
    SysStatus & sys_status = SysStatus::instance();
    connect(&sys_status, SIGNAL(volumeChanged(int, bool)), this, SLOT(onSystemVolumeChanged(int, bool)));
}

DictFrame::~DictFrame()
{
}

void DictFrame::createLayout()
{
    title_icon_label_.setPixmap(QPixmap(":/images/dictionary_title.png"));
    OnyxDialog::updateTitle(tr("Dictionary"));

    // hbox to layout line edit and buttons.
    vlayout_.setContentsMargins(0, 2, 0, 2);
    vlayout_.setSpacing(2);

    // text edit
    dict_edit_.setFixedHeight(80);
    clear_button_.setFixedHeight(80);
    hlayout1_.addWidget(&dict_edit_);
    hlayout1_.addWidget(&clear_button_);
    vlayout_.addLayout(&hlayout1_);

    // details
    vlayout_.addWidget(&details_);
    vlayout_.addWidget(&list_widget_);
    list_widget_.setVisible(false);

    // hlayout
    hlayout2_.setContentsMargins(2, 2, 2, 2);
    hlayout2_.setSpacing(2);
    vlayout_.addLayout(&hlayout2_);
    hlayout2_.addWidget(&dict_list_button_);
    hlayout2_.addWidget(&word_list_button_);
    hlayout2_.addWidget(&details_button_);
    hlayout2_.addWidget(&lookup_button_);

    speak_button_.setStyleSheet(TTS_BUTTON_STYLE);
    QPixmap speak_map(":/images/tts_menu.png");
    speak_button_.setIcon(QIcon(speak_map));
    speak_button_.setIconSize(speak_map.size());
    speak_button_.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    speak_button_.setCheckable(false);
    hlayout2_.addWidget(&speak_button_);

    dict_list_button_.useDefaultHeight();
    lookup_button_.useDefaultHeight();
    word_list_button_.useDefaultHeight();
    details_button_.useDefaultHeight();

    // keyboard.
    keyboard_.attachReceiver(this);
    vlayout_.addWidget(&keyboard_);

    // status bar
    vlayout_.addWidget(&status_bar_);

    // Setup connection.
    connect(&details_button_, SIGNAL(clicked(bool)), this,
            SLOT(onDetailsClicked(bool)), Qt::QueuedConnection);

    connect(&lookup_button_, SIGNAL(clicked(bool)), this,
            SLOT(onLookupClicked(bool)), Qt::QueuedConnection);

    connect(&word_list_button_, SIGNAL(clicked(bool)), this,
            SLOT(onWordListClicked(bool)), Qt::QueuedConnection);

    connect(&dict_list_button_, SIGNAL(clicked(bool)), this,
            SLOT(onDictListClicked(bool)), Qt::QueuedConnection);

    connect(&speak_button_, SIGNAL(clicked(bool)), this,
            SLOT(onSpeakClicked(bool)), Qt::QueuedConnection);

    connect(&clear_button_, SIGNAL(clicked()), &dict_edit_,
            SLOT(clear()), Qt::QueuedConnection);

    connect(&list_widget_, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(onItemClicked(const QModelIndex &)));

    connect(&list_widget_, SIGNAL(exceed(bool)),
            this, SLOT(moreSimilarWords(bool)));

    details_button_.setCheckable(true);
    lookup_button_.setCheckable(true);
    word_list_button_.setCheckable(true);
    dict_list_button_.setCheckable(true);
    details_button_.setChecked(true);
    list_widget_.showHeader(false);

    // Install event filter.
    dict_edit_.installEventFilter(this);
    details_.installEventFilter(this);
    list_widget_.installEventFilter(this);
}

void DictFrame::initBrowser()
{
    QFont font(QApplication::font());
    font.setPointSize(20);
    doc_.setDefaultFont(font);
    details_.setDocument(&doc_);
}

void DictFrame::initDictionaries()
{
    dict_mgr_.loadDictionaries();
}

void DictFrame::onItemClicked(const QModelIndex & index)
{
    onyx::screen::instance().enableUpdate(false);
    if (word_list_button_.isChecked())
    {
        QString text = similar_words_model_.itemFromIndex(index)->data().toString();
        dict_edit_.setText(text);
        lookup(text);
        details_button_.click();
    }
    else if (dict_list_button_.isChecked())
    {
        dict_mgr_.select(dict_list_model_.itemFromIndex(index)->text());
        lookup(word_);
        details_button_.click();
    }
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
}

void DictFrame::moreSimilarWords(bool begin)
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

void DictFrame::onDetailsClicked(bool)
{
    changeInternalState(DETAILS);
    details_button_.setChecked(true);
    lookup_button_.setChecked(false);
    dict_list_button_.setChecked(false);
    word_list_button_.setChecked(false);
    details_.show();
    details_.setFocus();
    list_widget_.hide();
}

bool DictFrame::lookup(const QString &word)
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
    bool ret = dict_mgr_.translate(word_, result);

    // Result
    doc_.setHtml(result);

    updateVisibleWidgets();
    onDetailsClicked(true);
    return ret;
}

void DictFrame::keyReleaseEvent(QKeyEvent *ke)
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

    ke->accept();
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
        break;
    default:
        break;
    }
}

/// The keyPressEvent could be sent from virtual keyboard.
void DictFrame::keyPressEvent(QKeyEvent * ke)
{
    ke->accept();
    if (ke->key() == Qt::Key_Shift || ke->key() == Qt::Key_CapsLock)
    {
        return;
    }

    // Disable the parent widget to update screen.
    onyx::screen::instance().enableUpdate(false);

    if (dict_edit_.hasFocus() ||
        (ke->key() != Qt::Key_Down &&
         ke->key() != Qt::Key_Up &&
         ke->key() != Qt::Key_Left &&
         ke->key() != Qt::Key_Right))
    {
        QKeyEvent * key_event = new QKeyEvent(ke->type(), ke->key(), ke->modifiers(), ke->text());
        QApplication::postEvent(&dict_edit_, key_event);
    }

    while (QApplication::hasPendingEvents())
    {
        QApplication::processEvents();
    }
    onyx::screen::instance().enableUpdate(true);

    // Update the line edit.
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW, true);
}

void DictFrame::updateSimilarWordsModel(int count)
{
    // Pick up similar words from current dictionary.
    similar_words_.clear();
    dict_mgr_.similarWords(word_, similar_words_, similar_words_offset_, count);
    similar_words_model_.clear();
    QStandardItem *parentItem = similar_words_model_.invisibleRootItem();
    QString explanation;
    foreach(QString item, similar_words_)
    {
        // Get explanation.
        dict_mgr_.translate(item, explanation);
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

void DictFrame::updateSimilarWords()
{
    list_widget_.clear();
    list_widget_.setFocus();
    list_widget_.show();
    updateSimilarWordsModel(list_widget_.itemsPerPage());

    // Update the list.
    list_widget_.setModel(&similar_words_model_);
}

void DictFrame::updateDictionaryListModel()
{
    int count = dict_list_model_.rowCount(dict_list_model_.invisibleRootItem()->index());
    if (count <= 0)
    {
        dict_list_model_.clear();
        QStringList list;
        dict_mgr_.dictionaries(list);
        foreach(QString item, list)
        {
            dict_list_model_.appendRow(new QStandardItem(item));
        }
    }
}

void DictFrame::updateDictionaryList()
{
    resetSimilarWordsOffset();
    updateDictionaryListModel();
    list_widget_.clear();
    list_widget_.setModel(&dict_list_model_);
    list_widget_.show();
    list_widget_.select(dict_mgr_.selected());
    list_widget_.setFocus();
}

void DictFrame::updateVisibleWidgets()
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

bool DictFrame::eventFilter(QObject *obj, QEvent *event)
{
    bool ret = OnyxDialog::eventFilter(obj, event);
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            ke->accept();
            changeInternalState(DETAILS);
            lookup_button_.setFocus();
            return true;
        }
    }
    return ret;
}

void DictFrame::changeInternalState(int state)
{
    internal_state_ = state;
}

bool DictFrame::event(QEvent *e)
{
    int ret = QDialog::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        if (sys::SysStatus::instance().isSystemBusy())
        {
            sys::SysStatus::instance().setSystemBusy(false);
        }

        static int count = 0;
        qDebug("Update request %d", ++count);
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
        e->accept();
    }

    return ret;
}

void DictFrame::moveEvent(QMoveEvent *e)
{
    e->accept();
}

void DictFrame::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void DictFrame::hideEvent(QHideEvent * event)
{
    QDialog::hideEvent(event);
}

void DictFrame::onLookupClicked(bool)
{
    changeInternalState(LOOKUP);
    lookup_button_.setChecked(true);
    dict_list_button_.setChecked(false);
    word_list_button_.setChecked(false);
    details_button_.setChecked(false);
    lookup(dict_edit_.toPlainText());
}

void DictFrame::onWordListClicked(bool)
{
    changeInternalState(WORD_LIST);
    onyx::screen::instance().enableUpdate(false);
    lookup_button_.setChecked(false);
    dict_list_button_.setChecked(false);
    details_button_.setChecked(false);
    word_list_button_.setChecked(true);
    details_.hide();
    updateSimilarWords();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
}

void DictFrame::onDictListClicked(bool)
{
    changeInternalState(DICTIONARY_LIST);
    onyx::screen::instance().enableUpdate(false);
    lookup_button_.setChecked(false);
    word_list_button_.setChecked(false);
    details_button_.setChecked(false);
    dict_list_button_.setChecked(true);
    details_.hide();
    updateDictionaryList();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU, false);
}

void DictFrame::onCloseClicked()
{
    releaseKeyboard();
    hide();
    emit closeClicked();
}

void DictFrame::onSpeakClicked(bool)
{
    QString text = dict_edit_.toPlainText();
    if (!text.isEmpty() && tts_engine_ != 0)
    {
        tts_engine_->speak(text);
    }
}

void DictFrame::onSystemVolumeChanged(int value, bool muted)
{
    qDebug("Volume Change:%d", value);
    tts_engine_->sound().setVolume(value);
}

void DictFrame::onInputText()
{
    if (keyboard_.isHidden())
    {
        keyboard_.setVisible(true);
    }
    else
    {
        keyboard_.setVisible(false);
    }
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void DictFrame::onScreenSizeChanged(int)
{
    onyx::screen::instance().enableUpdate(false);
    qDebug("Screen Resize");
    resize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
}

void DictFrame::updateActions()
{
    std::vector<int> actions;
    actions.push_back(ROTATE_SCREEN);
    actions.push_back(SCREEN_UPDATE_TYPE);
    actions.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(actions);
}

static RotateDegree getSystemRotateDegree()
{
    int degree = 0;
#ifdef BUILD_FOR_ARM
    degree = QScreen::instance()->transformOrientation();
#endif
    return static_cast<RotateDegree>(degree);
}

void DictFrame::rotate()
{
#ifndef Q_WS_QWS
    RotateDegree prev_degree = getSystemRotateDegree();
    if (prev_degree == ROTATE_0_DEGREE)
    {
        resize(800, 600);
    }
    else
    {
        resize(600, 800);
    }
#else
    SysStatus::instance().rotateScreen();
#endif
}

void DictFrame::popupMenu()
{
    ui::PopupMenu menu(this);
    updateActions();
    menu.setSystemAction(&system_actions_);

    qDebug("Popup Menu");
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == system_actions_.category())
    {
        SystemAction system = system_actions_.selected();
        if (system == RETURN_TO_LIBRARY)
        {
            returnToLibrary();
        }
        else if (system == SCREEN_UPDATE_TYPE)
        {
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
            onyx::screen::instance().toggleWaveform();
        }
        else if (system == MUSIC)
        {
            // Start or show music player.
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
            sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
        }
        else if (system == ROTATE_SCREEN)
        {
            rotate();
        }
        return;
    }
}

void DictFrame::returnToLibrary()
{
    qApp->exit();
}
