#ifndef DICT_FRAME_H_
#define DICT_FRAME_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "onyx/sys/sys.h"
#include "tts/tts.h"
#include "dictionary_manager.h"

using namespace ui;
using namespace base;

class DictFrame : public OnyxDialog
{
    Q_OBJECT
public:
    DictFrame(QWidget *parent, DictionaryManager & dict, tts::TTS *tts = 0);
    ~DictFrame();

    bool lookup(const QString &word);

Q_SIGNALS:
    void rotateScreen();
    void keyReleaseSignal(int);
    void closeClicked();

private Q_SLOTS:
    void popupMenu();
    void onScreenSizeChanged(int);
    void onInputText();

protected:
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual bool event(QEvent *);
    void moveEvent(QMoveEvent *e);
    void resizeEvent(QResizeEvent *e);
    void hideEvent(QHideEvent * event);

private:
    void createLayout();
    void initBrowser();
    void initDictionaries();
    void updateActions();

    void updateSimilarWordsModel(int count);
    void resetSimilarWordsOffset() { similar_words_offset_ = 0; }
    void updateSimilarWords();

    void updateDictionaryListModel();
    void updateDictionaryList();

    void updateVisibleWidgets();
    bool eventFilter(QObject *obj, QEvent *event);

    void changeInternalState(int);
    int  internalState() { return internal_state_; }

    // rotate
    void rotate();

    void returnToLibrary();

private Q_SLOTS:
    void onItemClicked(const QModelIndex & index);
    void onDetailsClicked(bool);
    void onLookupClicked(bool);
    void onWordListClicked(bool);
    void onDictListClicked(bool);
    void onCloseClicked();
    void onSpeakClicked(bool);
    void moreSimilarWords(bool);

    void onSystemVolumeChanged(int value, bool muted);

private:
    QVBoxLayout             vlayout_;           ///< Widgets Layout
    QHBoxLayout             hlayout1_;          ///< hlayout 1
    QHBoxLayout             hlayout2_;          ///< hlayout 2

    OnyxTextEdit            dict_edit_;         ///< Dict Edit
    OnyxTextBrowser         details_;           ///< The lookup result.
    OnyxTreeView            list_widget_;
    KeyBoard                keyboard_;          ///< Keyboard
    StatusBar               status_bar_;        ///< Status Bar

    OnyxPushButton          clear_button_;
    OnyxPushButton          dict_list_button_;
    OnyxPushButton          lookup_button_;
    OnyxPushButton          word_list_button_;
    OnyxPushButton          details_button_;
    OnyxPushButton          speak_button_;

    DictionaryManager       &dict_mgr_;
    tts::TTS                *tts_engine_;

    QButtonGroup            button_group_;
    QTextDocument           doc_;
    QString                 word_;              ///< Word currently queried.
    QStringList             similar_words_;
    int                     similar_words_offset_;
    QTimer                  timer_;        ///< Timer to update the screen.

    QStandardItemModel      similar_words_model_;
    QStandardItemModel      dict_list_model_;

    int                     internal_state_;
    SystemActions           system_actions_;    ///< System Actions

private:
    NO_COPY_AND_ASSIGN(DictFrame);
};

#endif
