#ifndef CR3WIDGET_H
#define CR3WIDGET_H

#include <qwidget.h>
#include <QScrollBar>
#include <QImage>

#include "crqtutil.h"
#include "search_tool.h"

#include "onyx/tts/tts_widget.h"
#include "onyx/ui/onyx_search_dialog.h"
#include "onyx/dictionary/dictionary_manager.h"
#include "onyx/dictionary/dict_widget.h"

class LVDocView;
class LVTocItem;
class CRBookmark;

class PropsChangeCallback {
public:
    virtual void onPropsChange( PropsRef props ) = 0;
    virtual ~PropsChangeCallback() { }
};

#define WORD_SELECTOR_ENABLED 1

class CR3View : public QWidget, public LVDocViewCallback
{

        Q_OBJECT

        Q_PROPERTY( QScrollBar* scrollBar READ scrollBar WRITE setScrollBar )

        class DocViewData;

#if WORD_SELECTOR_ENABLED==1
        LVPageWordSelector * _wordSelector;

    protected:
        void startWordSelection();
        QString endWordSelection();
        bool isWordSelection() { return _wordSelector!=NULL; }
#endif

    public:
        CR3View( QWidget *parent = 0 );
        virtual ~CR3View();

        bool loadDocument( QString fileName );
        vector<QString> getRecentBooks();
        void openRecentBook(int index);

        bool loadLastDocument();
        void setDocumentText( QString text );

        QScrollBar * scrollBar() const;

        /// get document's table of contents
        LVTocItem * getToc();
        /// return LVDocView associated with widget
        LVDocView * getDocView() { return _docview; }
        /// go to position specified by xPointer string
        void goToXPointer(QString xPointer);

        /// returns current page
        int getCurPage();
        inline QPoint getSelectWordPoint(){ return select_word_point_;}
        inline QImage & getPageImage() { return img_; }

        /// load settings from file
        bool loadSettings( QString filename );
        /// save settings from file
        bool saveSettings( QString filename );
        /// load history from file
        bool loadHistory( QString filename );
        /// save history to file
        bool saveHistory( QString filename );

        void setHyphDir( QString dirname );
        const QStringList & getHyphDicts();

        /// load fb2.css file
        bool loadCSS( QString filename );
        /// set bookmarks dir
        void setBookmarksDir( QString dirname );
        /// set new option values
        PropsRef setOptions( PropsRef props );
        /// get current option values
        PropsRef getOptions();
        /// turns on/off Edit mode (forces Scroll view)
        void setEditMode( bool flgEdit );
        /// returns true if edit mode is active
        bool getEditMode() { return _editMode; }

        void saveWindowPos( QWidget * window, const char * prefix );
        void restoreWindowPos( QWidget * window, const char * prefix, bool allowExtraStates = false );

        void setPropsChangeCallback ( PropsChangeCallback * propsCallback )
        {
            _propsCallback = propsCallback;
        }
        /// toggle boolean property
        void toggleProperty( const char * name );
        /// returns true if point is inside selected text
        bool isPointInsideSelection( QPoint pt );
        /// returns selection text
        QString getSelectionText() { return _selText; }
        /// create bookmark
        CRBookmark * createBookmark();
        CRBookmark * createCitation();
        void deleteCitation(QMouseEvent * evnet);
        void paintCitation();
        /// go to bookmark and highlight it
        void goToBookmark( CRBookmark * bm );
        bool hasBookmark();
        void deleteBookmark();

        void enableAddCitation(bool enable) { enable_add_citation_ = enable; }
        void enableDeleteCitation(bool enable) { enable_delete_citation_ = enable; }

        void setFullScreen(bool full) { is_full_screen_ = full;}

        /// rotate view, +1 = 90` clockwise, -1 = 90` counterclockwise
        void rotate( int angle );
        /// Override to handle external links
        virtual void OnExternalLink( lString16 url, ldomNode * node );
        /// format detection finished
        virtual void OnLoadFileFormatDetected( doc_format_t fileFormat );
        /// on starting file loading
        virtual void OnLoadFileStart( lString16 filename );
        /// first page is loaded from file an can be formatted for preview
        virtual void OnLoadFileFirstPagesReady();
        /// file load finiished with error
        virtual void OnLoadFileError( lString16 message );
        /// file loading is finished successfully - drawCoveTo() may be called there
        virtual void OnLoadFileEnd();
        /// document formatting started
        virtual void OnFormatStart();
        /// document formatting finished
        virtual void OnFormatEnd();
        /// file progress indicator, called with values 0..100
        virtual void OnLoadFileProgress( int percent );
        /// format progress, called with values 0..100
        virtual void OnFormatProgress( int percent );

    public slots:
        void contextMenu( QPoint pos );
        void setScrollBar( QScrollBar * scroll );
        /// on scroll
        void togglePageScrollView();
        void scrollTo( int value );

        // handle screen updating internally
        void nextPageWithTTSChecking();
        // handle screen updating internally
        void prevPageWithTTSChecking();
        // handle screen updating internally
        void gotoPageWithTTSChecking(const int dstPage);

        void nextLine();
        void prevLine();
        void nextChapter();
        void prevChapter();
        void firstPage();
        void lastPage();
        void historyBack();
        void historyForward();
        void zoomIn();
        void zoomOut();
        void ableTurnPage();

        void startDictLookup();
        void showSearchWidget();

        void collectTTSContent();
        bool hasPendingTTSContent();
        void startTTS();
        void onSpeakDone();
        void stopTTS();
        void waitToStartTTS();

        void toggleFontEmbolden();

        tts::TTSWidget & ttsWidget();
        tts::TTS & tts();

    signals:
        //void fileNameChanged( const QString & );
        void updateProgress(int, int);
        void requestTranslate();
        void requestUpdateAll();

    protected:
        virtual void keyPressEvent ( QKeyEvent * event );
        virtual void keyReleaseEvent(QKeyEvent * event);
        virtual void paintEvent ( QPaintEvent * event );
        virtual void resizeEvent ( QResizeEvent * event );
        virtual void wheelEvent ( QWheelEvent * event );
        virtual void updateScroll();
        virtual void doCommand( int cmd, int param = 0 );
        virtual void mouseMoveEvent ( QMouseEvent * event );
        virtual void mousePressEvent ( QMouseEvent * event );
        virtual void mouseReleaseEvent ( QMouseEvent * event );
        virtual void refreshPropFromView( const char * propName );
        virtual void mouseDoubleClickEvent(QMouseEvent *event);
        virtual bool eventFilter(QObject *obj, QEvent *event);

    private slots:
        void nextPage();
        void prevPage();
        void gotoPage(const int dstPage);

        void lookup();
        void onDictClosed();
        void onSearchClosed();
        void onSearch(OnyxSearchContext& context);
        void processKeyReleaseEvent(int key);
        void onBatterySignal(int value, int status);

    private:
        void updateDefProps();
        void clearSelection();
        void startSelection( ldomXPointer p );
        bool endSelection( ldomXPointer p );

        bool updateSelection( ldomXPointer p );
        bool updateSelection(ldomXRange * range);

        bool adjustDictWidget();

        void paintBookmark( QPainter & painter );
        void hideHelperWidget(QWidget * wnd);
        bool updateSearchWidget();
        void stylusPan(const QPoint &now, const QPoint &old);

        bool docToWindowRect(lvRect &rect);

        bool getPreviousWord(ldomXRange *range, bool *meetPageBoundary);
        bool getNextWord(ldomXRange *range, bool *meetPageBoundary);
        bool getUpWord(ldomXRange *range);
        bool getDownWord(ldomXRange *range);

        bool selectPreviousWord();
        bool selectNextWord();
        bool selectUpWord();
        bool selectDownWord();
        void handleMouseReleaseInCitationMode(QMouseEvent * event);

        bool isDictionaryMode();
        void moveDictWidget();

        QImage & image();
        QString resourcePath();

    private:
        DocViewData * _data; // to hide non-qt implementation
        LVDocView * _docview;
        QScrollBar * _scroll;
        PropsChangeCallback * _propsCallback;
        QStringList _hyphDicts;

        bool _selecting;
        bool _selected;
        ldomXPointer _selStart;
        ldomXPointer _selEnd;
        QString _selText;
        ldomXRange _selRange;
        QString _cssDir;
        QString _bookmarkDir;
        bool _editMode;

        QPoint select_word_point_;
        QPoint begin_point_;

        QImage bookmark_image_;
        QRect selected_rect_;

        SearchTool *search_tool_;
        scoped_ptr<ui::OnyxSearchDialog> search_widget_;
        OnyxSearchContext search_context_;
        scoped_ptr<DictionaryManager> dicts_;
        scoped_ptr<ui::DictWidget> dict_widget_;
        scoped_ptr<tts::TTS> tts_engine_;
        scoped_ptr<tts::TTSWidget> tts_widget_;
        QStringList text_to_speak_;
        int tts_paragraph_index_;

        QImage img_;
        bool able_turn_page_;

        bool _citation_mode_;
        QString file_name_;

        Images images_;
        int value_;
        int status_;
        bool is_full_screen_;
        QRect battery_rcet_;

        bool enable_add_citation_;
        bool enable_delete_citation_;
};

#endif // CR3WIDGET_H
