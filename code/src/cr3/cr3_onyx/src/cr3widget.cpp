#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QMenu>
#include <QtGui/QStyleFactory>
#include <QtGui/QStyle>
#include <QtGui/QApplication>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDebug>

#include "cr3widget.h"

#include "crqtutil.h"
#include "qpainter.h"
#include "settings.h"

#include "../crengine/include/lvdocview.h"
#include "../crengine/include/crtrace.h"
#include "../crengine/include/props.h"

#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/ui_utils.h"

using namespace std;

static const int BEFORE_SEARCH = 0;
static const int IN_SEARCHING  = 1;

/// to hide non-qt implementation, place all crengine-related fields here
class CR3View::DocViewData
{
    friend class CR3View;
    lString16 _settingsFileName;
    lString16 _historyFileName;
    CRPropRef _props;
};

DECL_DEF_CR_FONT_SIZES;

// comment out to eliminate compiler warning
//static void replaceColor( char * str, lUInt32 color )
//{
//    // in line like "0 c #80000000",
//    // replace value of color
//    for ( int i=0; i<8; i++ ) {
//        str[i+5] = toHexDigit((color>>28) & 0xF);
//        color <<= 4;
//    }
//}

CR3View::CR3View( QWidget *parent)
        : QWidget( parent, Qt::WindowFlags() ), _scroll(NULL), _propsCallback(NULL)
        , _selecting(false), _selected(false), _editMode(false)
        , select_word_point_(0, 0)
        , bookmark_image_(":/images/bookmark_flag.png")
{
#if WORD_SELECTOR_ENABLED==1
    _wordSelector = NULL;
#endif
    _data = new DocViewData();
    _data->_props = LVCreatePropsContainer();
    _docview = new LVDocView();
    _docview->setCallback( this );
    _selStart = ldomXPointer();
    _selEnd = ldomXPointer();
    _selText.clear();
    ldomXPointerEx p1;
    ldomXPointerEx p2;
    _selRange.setStart(p1);
    _selRange.setEnd(p2);
    LVArray<int> sizes( cr_font_sizes, sizeof(cr_font_sizes)/sizeof(int) );
    _docview->setFontSizes( sizes, false );

//    LVStreamRef stream;
//    stream = LVOpenFileStream("/home/lve/.cr3/textures/old_paper.png", LVOM_READ);
    //stream = LVOpenFileStream("/home/lve/.cr3/textures/tx_wood.jpg", LVOM_READ);
    //stream = LVOpenFileStream("/home/lve/.cr3/backgrounds/Background1.jpg", LVOM_READ);
//    if ( !stream.isNull() ) {
//        LVImageSourceRef img = LVCreateStreamCopyImageSource(stream);
//        if ( !img.isNull() ) {
//            //img = LVCreateUnpackedImageSource(img, 1256*1256*4, false);
//            _docview->setBackgroundImage(img, true);
//        }
//    }
    updateDefProps();
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    search_tool_ = new SearchTool(this, this);
}

void CR3View::updateDefProps()
{
    _data->_props->setStringDef( PROP_WINDOW_FULLSCREEN, "0" );
    _data->_props->setStringDef( PROP_WINDOW_SHOW_MENU, "1" );
    _data->_props->setStringDef( PROP_WINDOW_SHOW_SCROLLBAR, "1" );
    _data->_props->setStringDef( PROP_WINDOW_TOOLBAR_SIZE, "1" );
    _data->_props->setStringDef( PROP_WINDOW_SHOW_STATUSBAR, "0" );
    _data->_props->setStringDef( PROP_APP_START_ACTION, "0" );

    QStringList styles = QStyleFactory::keys();
    QStyle * s = QApplication::style();
    QString currStyle = s->objectName();
    CRLog::debug("Current system style is %s", currStyle.toUtf8().data() );
    QString style = cr2qt(_data->_props->getStringDef( PROP_WINDOW_STYLE, currStyle.toUtf8().data() ));
    if ( !styles.contains(style, Qt::CaseInsensitive) )
        _data->_props->setString( PROP_WINDOW_STYLE, qt2cr(currStyle) );
}

CR3View::~CR3View()
{
#if WORD_SELECTOR_ENABLED==1
    if ( _wordSelector )
        delete _wordSelector;
#endif
    _docview->savePosition();
    saveHistory( QString() );
    saveSettings( QString() );
    delete _docview;
    delete _data;

    delete search_tool_;
    tts_widget_.release();
    dicts_.release();
    tts_engine_.release();
    dict_widget_.release();
    search_widget_.release();
}

#if WORD_SELECTOR_ENABLED==1
void CR3View::startWordSelection() {
    if ( isWordSelection() )
        endWordSelection();
    _wordSelector = new LVPageWordSelector(_docview);
    update();
}

QString CR3View::endWordSelection() {
    QString text;
    if ( isWordSelection() ) {
        ldomWordEx * word = _wordSelector->getSelectedWord();
        if ( word )
            text = cr2qt(word->getText());
        delete _wordSelector;
        _wordSelector = NULL;
        _docview->clearSelection();
        update();
    }
    return text;
}
#endif

void CR3View::setHyphDir( QString dirname )
{
    HyphMan::initDictionaries( qt2cr( dirname) );
    _hyphDicts.clear();
    for ( int i=0; i<HyphMan::getDictList()->length(); i++ ) {
        HyphDictionary * item = HyphMan::getDictList()->get( i );
        QString fn = cr2qt( item->getFilename() );
        _hyphDicts.append( fn );
    }
}

const QStringList & CR3View::getHyphDicts()
{
    return _hyphDicts;
}

LVTocItem * CR3View::getToc()
{
    return _docview->getToc();
}

/// go to position specified by xPointer string
void CR3View::goToXPointer(QString xPointer)
{
    ldomXPointer p = _docview->getDocument()->createXPointer(qt2cr(xPointer));
    _docview->savePosToNavigationHistory();
    //if ( _docview->getViewMode() == DVM_SCROLL ) {
    doCommand( DCMD_GO_POS, p.toPoint().y );
    //} else {
    //    doCommand( DCMD_GO_PAGE, item->getPage() );
    //}
}

/// returns current page
int CR3View::getCurPage()
{
    return _docview->getCurPage();
}

void CR3View::setDocumentText( QString text )
{
    _docview->savePosition();
    clearSelection();
    _docview->createDefaultDocument( lString16(), qt2cr(text) );
}

bool CR3View::loadLastDocument()
{
    CRFileHist * hist = _docview->getHistory();
    if ( !hist || hist->getRecords().length()<=0 )
        return false;
    return loadDocument( cr2qt(hist->getRecords()[0]->getFilePathName()) );
}

bool CR3View::loadDocument( QString fileName )
{
    _docview->savePosition();
    clearSelection();
    bool res = _docview->LoadDocument( qt2cr(fileName).c_str() );
    if ( res ) {
        _docview->swapToCache();
        QByteArray utf8 = fileName.toUtf8();
        CRLog::debug( "Trying to restore position for %s", utf8.constData() );
        _docview->restorePosition();
    } else {
        _docview->createDefaultDocument( lString16(), qt2cr(tr("Error while opening document ") + fileName) );
    }
    update();
    return res;
}

vector<QString> CR3View::getRecentBooks()
{
    vector<QString> books;
    LVPtrVector<CRFileHistRecord> & files = _docview->getHistory()->getRecords();

    for (int i = 0; i < files.length(); i++) {
        CRFileHistRecord * file = files.get( i );
        lString16 fn = file->getFilePathName();
        if ( LVFileExists(fn) )
            books.push_back(cr2qt(fn));
    }

    return books;
}

void CR3View::openRecentBook( int index )
{
    _docview->swapToCache();
    _docview->updateCache();
    _docview->getDocument()->updateMap();

    _docview->savePosition();
    LVPtrVector<CRFileHistRecord> & files = _docview->getHistory()->getRecords();
    if ( index >= 0 && index < files.length() ) {
        CRFileHistRecord * file = files.get( index );
        lString16 fn = file->getFilePathName();
        // TODO: check error
        if ( LVFileExists(fn) ) {
            //showWaitIcon();
            loadDocument( cr2qt(fn) );
        }
        //_docview->swapToCache();
    }
}

void CR3View::wheelEvent( QWheelEvent * event )
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    if ( numSteps==0 && numDegrees!=0 )
        numSteps = numDegrees>0 ? 1 : -1;

    if ( numSteps ) {
        if ( _docview->getViewMode() == DVM_SCROLL ) {
            if ( numSteps > 0 )
                doCommand( DCMD_LINEDOWN, -numSteps );
            else
                doCommand( DCMD_LINEUP, numSteps );
        } else {
            if ( numSteps > 0 )
                doCommand( DCMD_PAGEUP, -numSteps );
            else
                doCommand( DCMD_PAGEDOWN, numSteps );
        }
    }
    event->accept();
 }

void CR3View::resizeEvent ( QResizeEvent * event )
{

    QSize sz = event->size();
    _docview->Resize( sz.width(), sz.height() );
}

void CR3View::paintEvent ( QPaintEvent * event )
{
    QPainter painter(this);
    QRect rc = rect();
    LVDocImageRef ref = _docview->getPageImage(0);
    if ( ref.isNull() ) {
        //painter.fillRect();
        return;
    }
    LVDrawBuf * buf = ref->getDrawBuf();
    int dx = buf->GetWidth();
    int dy = buf->GetHeight();
    if ( buf->GetBitsPerPixel()==16 ) {
        img_ = QImage(dx, dy, QImage::Format_RGB16 );
        for ( int i=0; i<dy; i++ ) {
            unsigned char * dst = img_.scanLine( i );
            unsigned char * src = buf->GetScanLine(i);
            for ( int x=0; x<dx; x++ ) {
                *dst++ = *src++;
                *dst++ = *src++;
//                *dst++ = *src++;
//                *dst++ = 0xFF;
//                src++;
            }
        }
        painter.drawImage( rc, img_ );
    } else if ( buf->GetBitsPerPixel()==32 ) {
        img_ = QImage(dx, dy, QImage::Format_RGB32 );
        for ( int i=0; i<dy; i++ ) {
            unsigned char * dst = img_.scanLine( i );
            unsigned char * src = buf->GetScanLine(i);
            for ( int x=0; x<dx; x++ ) {
                *dst++ = *src++;
                *dst++ = *src++;
                *dst++ = *src++;
                *dst++ = 0xFF;
                src++;
            }
        }
        painter.drawImage( rc, img_ );
    }
    if ( _editMode ) {
    }
    if(hasBookmark())
    {
        paintBookmark(painter);
    }
    updateScroll();
}

void CR3View::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(!getSelectionText().isEmpty() &&
       !qgetenv("DISABLE_DICT").toInt())
    {
        select_word_point_ = event->pos();
        lookup();
    }
}

void CR3View::updateScroll()
{
    if ( _scroll!=NULL ) {
        // TODO: set scroll range
        const LVScrollInfo * si = _docview->getScrollInfo();
        bool changed = false;
        if ( si->maxpos != _scroll->maximum() ) {
            _scroll->setMaximum( si->maxpos );
            _scroll->setMinimum(0);
            changed = true;
        }
        if ( si->pagesize != _scroll->pageStep() ) {
            _scroll->setPageStep( si->pagesize );
            changed = true;
        }
        if ( si->pos != _scroll->value() ) {
            _scroll->setValue( si-> pos );
            changed = true;
        }
    }
}

void CR3View::scrollTo( int value )
{
    int currPos = _docview->getScrollInfo()->pos;
    if ( currPos != value ) {
        doCommand( DCMD_GO_SCROLL_POS, value );
    }
}

void CR3View::doCommand( int cmd, int param )
{
    _docview->doCommand( (LVDocCmd)cmd, param );
    update();
}

void CR3View::togglePageScrollView()
{
    if ( _editMode )
        return;
    doCommand( DCMD_TOGGLE_PAGE_SCROLL_VIEW, 1 );
    refreshPropFromView( PROP_PAGE_VIEW_MODE );
}

void CR3View::setEditMode( bool flgEdit )
{
    if ( _editMode == flgEdit )
        return;

    if ( flgEdit && _data->_props->getIntDef( PROP_PAGE_VIEW_MODE, 0 ) )
        togglePageScrollView();
    _editMode = flgEdit;
    update();
}

void CR3View::toggleFontEmbolden() { doCommand(DCMD_TOGGLE_BOLD, 1); };

void CR3View::nextLine() { doCommand( DCMD_LINEDOWN, 1 ); }
void CR3View::prevLine() { doCommand( DCMD_LINEUP, 1 ); }
void CR3View::nextChapter() { doCommand( DCMD_MOVE_BY_CHAPTER, 1 ); }
void CR3View::prevChapter() { doCommand( DCMD_MOVE_BY_CHAPTER, -1 ); }
void CR3View::firstPage() { doCommand( DCMD_BEGIN, 1 ); }
void CR3View::lastPage() { doCommand( DCMD_END, 1 ); }
void CR3View::historyBack() { doCommand( DCMD_LINK_BACK, 1 ); }
void CR3View::historyForward() { doCommand( DCMD_LINK_FORWARD, 1 ); }
void CR3View::nextPage() { doCommand(DCMD_PAGEDOWN, 1); }
void CR3View::prevPage() { doCommand(DCMD_PAGEUP, 1); }

void CR3View::gotoPage(const int dstPage)
{
	doCommand(DCMD_GO_PAGE, dstPage - 1);
}

void CR3View::nextPageWithTTSChecking()
{
    if ((tts_widget_) && (tts_widget_->isVisible()))
    {
        tts_engine_->stop();
        this->nextPage();
        emit requestUpdateAll();
        startTTS();
    }
    else {
        this->nextPage();
        emit requestUpdateAll();
    }
}

void CR3View::prevPageWithTTSChecking() {
    if ((tts_widget_) && (tts_widget_->isVisible())) {
        tts_engine_->stop();
        this->prevPage();
        emit requestUpdateAll();
        startTTS();
    }
    else {
        this->prevPage();
        emit requestUpdateAll();
    }
}

void CR3View::gotoPageWithTTSChecking(const int dstPage)
{
    if ((tts_widget_) && (tts_widget_->isVisible())) {
        tts_engine_->stop();
        this->gotoPage(dstPage);
        emit requestUpdateAll();
        startTTS();
    }
    else {
        this->gotoPage(dstPage);
        emit requestUpdateAll();
    }
}

void CR3View::refreshPropFromView( const char * propName )
{
    _data->_props->setString( propName, _docview->propsGetCurrent()->getStringDef( propName, "" ) );
}

void CR3View::zoomIn()
{
    int size;
    PropsRef props_ref = getOptions();
    props_ref->getInt(PROP_FONT_SIZE, size);
    if (size >= 40)
    {
        return;
    }
    else
    {
        props_ref->setString( PROP_FONT_SIZE, QString::number(size+1));
        setOptions(props_ref);
    }
}

void CR3View::zoomOut()
{
    int size;
    PropsRef props_ref = getOptions();
    props_ref->getInt(PROP_FONT_SIZE, size);
    if (size <= 12)
    {
        return;
    }
    else
    {
        props_ref->setString( PROP_FONT_SIZE, QString::number(size-1));
        setOptions(props_ref);
    }
}

QScrollBar * CR3View::scrollBar() const
{
    return _scroll;
}

void CR3View::setScrollBar( QScrollBar * scroll )
{
    _scroll = scroll;
    if ( _scroll!=NULL ) {
        QObject::connect(_scroll, SIGNAL(valueChanged(int)),
                          this,  SLOT(scrollTo(int)));
    }
}

/// load fb2.css file
bool CR3View::loadCSS( QString fn )
{
    lString16 filename( qt2cr(fn) );
    lString8 css;
    if ( LVLoadStylesheetFile( filename, css ) ) {
        if ( !css.empty() ) {
            QFileInfo f( fn );
            CRLog::info( "Using style sheet from %s", fn.toUtf8().constData() );
            _cssDir = f.absolutePath() + "/";
            _docview->setStyleSheet( css );
            return true;
        }
    }
    return false;
}

/// load settings from file
bool CR3View::loadSettings( QString fn )
{
    lString16 filename( qt2cr(fn) );
    _data->_settingsFileName = filename;
    LVStreamRef stream = LVOpenFileStream( filename.c_str(), LVOM_READ );
    bool res = false;
    if ( !stream.isNull() && _data->_props->loadFromStream( stream.get() ) ) {
        CRLog::error("Loading settings from file %s", fn.toUtf8().data() );
        res = true;
    } else {
        // when first load, give some default values. joy@onyx
        const char* default_font_size = "26";
        if( ui::screenGeometry().height() >= 1024 )
        {
            default_font_size = "28";
        }
        _data->_props->setString( PROP_FONT_SIZE, default_font_size);

        const unsigned int Default_Line_Height_Percentage = 130;
        _data->_props->setInt(PROP_INTERLINE_SPACE, Default_Line_Height_Percentage);

        CRLog::error("Cannot load settings from file %s", fn.toUtf8().data() );
    }
    _docview->propsUpdateDefaults( _data->_props );
    updateDefProps();
    CRPropRef r = _docview->propsApply( _data->_props );
    PropsRef unknownOptions = cr2qt(r);
    if ( _propsCallback != NULL )
        _propsCallback->onPropsChange( unknownOptions );
    return res;
}

/// toggle boolean property
void CR3View::toggleProperty( const char * name )
{
    int state = _data->_props->getIntDef( name, 0 )!=0 ? 0 : 1;
    PropsRef props = Props::create();
    props->setString( name, state?"1":"0" );
    setOptions( props );
}

/// set new option values
PropsRef CR3View::setOptions( PropsRef props )
{
//    for ( int i=0; i<_data->_props->getCount(); i++ ) {
//        CRLog::debug("Old [%d] '%s'=%s ", i, _data->_props->getName(i), UnicodeToUtf8(_data->_props->getValue(i)).c_str() );
//    }
//    for ( int i=0; i<props->count(); i++ ) {
//        CRLog::debug("New [%d] '%s'=%s ", i, props->name(i), props->value(i).toUtf8().data() );
//    }
    CRPropRef changed = _data->_props ^ qt2cr(props);
//    for ( int i=0; i<changed->getCount(); i++ ) {
//        CRLog::debug("Changed [%d] '%s'=%s ", i, changed->getName(i), UnicodeToUtf8(changed->getValue(i)).c_str() );
//    }
    _data->_props = changed | _data->_props;
//    for ( int i=0; i<_data->_props->getCount(); i++ ) {
//        CRLog::debug("Result [%d] '%s'=%s ", i, _data->_props->getName(i), UnicodeToUtf8(_data->_props->getValue(i)).c_str() );
//    }
    CRPropRef r = _docview->propsApply( changed );
    PropsRef unknownOptions = cr2qt(r);
    if ( _propsCallback != NULL )
        _propsCallback->onPropsChange( unknownOptions );
    saveSettings( QString() );
    update();
    return unknownOptions;
}

void CR3View::saveWindowPos( QWidget * window, const char * prefix )
{
    ::saveWindowPosition( window, _data->_props, prefix );
}

void CR3View::restoreWindowPos( QWidget * window, const char * prefix, bool allowExtraStates  )
{
    ::restoreWindowPosition( window, _data->_props, prefix, allowExtraStates );
}

/// get current option values
PropsRef CR3View::getOptions()
{
    return Props::clone(cr2qt( _data->_props ));
}

/// save settings from file
bool CR3View::saveSettings( QString fn )
{
    lString16 filename( qt2cr(fn) );
    crtrace log;
    if ( filename.empty() )
        filename = _data->_settingsFileName;
    if ( filename.empty() )
        return false;
    _data->_settingsFileName = filename;
    log << "V3DocViewWin::saveSettings(" << filename << ")";
    LVStreamRef stream = LVOpenFileStream( filename.c_str(), LVOM_WRITE );
    if ( !stream ) {
        lString16 path16 = LVExtractPath( filename );
        lString8 path = UnicodeToUtf8(path16);
        if ( !LVCreateDirectory( path16 ) ) {
            CRLog::error("Cannot create directory %s", path.c_str() );
        } else {
            stream = LVOpenFileStream( filename.c_str(), LVOM_WRITE );
        }
    }
    if ( stream.isNull() ) {
        lString8 fn = UnicodeToUtf8( filename );
        CRLog::error("Cannot save settings to file %s", fn.c_str() );
        return false;
    }
    return _data->_props->saveToStream( stream.get() );
}

/// load history from file
bool CR3View::loadHistory( QString fn )
{
    lString16 filename( qt2cr(fn) );
    CRLog::trace("V3DocViewWin::loadHistory( %s )", UnicodeToUtf8(filename).c_str());
    _data->_historyFileName = filename;
    LVStreamRef stream = LVOpenFileStream( filename.c_str(), LVOM_READ );
    if ( stream.isNull() ) {
        return false;
    }
    if ( !_docview->getHistory()->loadFromStream( stream ) )
        return false;
    return true;
}

/// save history to file
bool CR3View::saveHistory( QString fn )
{
    lString16 filename( qt2cr(fn) );
    crtrace log;
    if ( filename.empty() )
        filename = _data->_historyFileName;
    if ( filename.empty() ) {
        CRLog::info("Cannot write history file - no file name specified");
        return false;
    }
    //CRLog::debug("Exporting bookmarks to %s", UnicodeToUtf8(_bookmarkDir).c_str());
    //_docview->exportBookmarks(_bookmarkDir); //use default filename
    lString16 bmdir = qt2cr(_bookmarkDir);
    LVAppendPathDelimiter( bmdir );
    _docview->exportBookmarks( bmdir ); //use default filename
    _data->_historyFileName = filename;
    log << "V3DocViewWin::saveHistory(" << filename << ")";
    LVStreamRef stream = LVOpenFileStream( filename.c_str(), LVOM_WRITE );
    if ( !stream ) {
        lString16 path16 = LVExtractPath( filename );
        lString8 path = UnicodeToUtf8(path16);
        if ( !LVCreateDirectory( path16 ) ) {
            CRLog::error("Cannot create directory %s", path.c_str() );
        } else {
            stream = LVOpenFileStream( filename.c_str(), LVOM_WRITE );
        }
    }
    if ( stream.isNull() ) {
    	CRLog::error("Error while creating history file %s - position will be lost", UnicodeToUtf8(filename).c_str() );
    	return false;
    }
    return _docview->getHistory()->saveToStream( stream.get() );
}

void CR3View::contextMenu( QPoint pos )
{
}

/// returns true if point is inside selected text
bool CR3View::isPointInsideSelection( QPoint pos )
{
    if ( !_selected )
        return false;
    lvPoint pt( pos.x(), pos.y() );
    ldomXPointerEx p( _docview->getNodeByPoint( pt ) );
    if ( p.isNull() )
        return false;
    return _selRange.isInside( p );
}

void CR3View::mouseMoveEvent ( QMouseEvent * event )
{
    //bool left = (event->buttons() & Qt::LeftButton);
    //bool right = (event->buttons() & Qt::RightButton);
    //bool mid = (event->buttons() & Qt::MidButton);
    lvPoint pt ( event->x(), event->y() );
    ldomXPointer p = _docview->getNodeByPoint( pt );
    lString16 path;
    lString16 href;
    if ( !p.isNull() ) {
        path = p.toString();
        href = p.getHRef();
        updateSelection(p);
    } else {
        //CRLog::trace("Node not found for %d, %d", event->x(), event->y());
    }
    //CRLog::trace("mouseMoveEvent - doc pos (%d,%d), buttons: %d %d %d %s", pt.x, pt.y, (int)left, (int)right
    //             , (int)mid, href.empty()?"":UnicodeToUtf8(href).c_str()
    //             //, path.empty()?"":UnicodeToUtf8(path).c_str()
    //             );
}

void CR3View::clearSelection()
{
    if ( _selected ) {
        _docview->clearSelection();
        update();
    }
    _selecting = false;
    _selected = false;
    _selStart = ldomXPointer();
    _selEnd = ldomXPointer();
    _selText.clear();
    ldomXPointerEx p1;
    ldomXPointerEx p2;
    _selRange.setStart(p1);
    _selRange.setEnd(p2);
}

void CR3View::startSelection( ldomXPointer p )
{
    clearSelection();
    _selecting = true;
    _selStart = p;
    updateSelection( p );
}

bool CR3View::endSelection( ldomXPointer p )
{
    if ( !_selecting )
        return false;
    updateSelection( p );
    if ( _selected ) {

    }
    _selecting = false;
    return _selected;
}

bool CR3View::updateSelection( ldomXPointer p )
{
    if ( !_selecting )
        return false;
    _selEnd = p;
    ldomXRange r( _selStart, _selEnd );

    return this->updateSelection(&r);
}

bool CR3View::updateSelection(ldomXRange *range)
{
    if (range->getStart().isNull() || range->getEnd().isNull())
        return false;

    range->sort();
    if (!_editMode) {
        if (!range->getStart().isVisibleWordStart()) {
            range->getStart().prevVisibleWordStart();
        }
        //lString16 start = r.getStart().toString();
        if (!range->getEnd().isVisibleWordEnd()) {
            range->getEnd().nextVisibleWordEnd();
        }
    }
    if (range->isNull())
        return false;
    //lString16 end = r.getEnd().toString();
    //CRLog::debug("Range: %s - %s", UnicodeToUtf8(start).c_str(), UnicodeToUtf8(end).c_str());
    range->setFlags(1);
    _docview->selectRange(*range);
    _selText = cr2qt(range->getRangeText('\n', 10000));
    _selected = true;
    _selRange = *range;

    _selStart = range->getStart();
    _selEnd = range->getEnd();

    // TODO comment out by joy@onyx
//    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
//
//    update();

    return true;
}

void CR3View::mousePressEvent ( QMouseEvent * event )
{
    bool left = event->button() == Qt::LeftButton;
    //bool right = event->button() == Qt::RightButton;
    //bool mid = event->button() == Qt::MidButton;
    begin_point_ = event->pos();
    lvPoint pt (event->x(), event->y());
    ldomXPointer p = _docview->getNodeByPoint( pt );
    lString16 path;
    lString16 href;
    if ( !p.isNull() ) {
        path = p.toString();
        CRLog::debug("mousePressEvent(%s)", LCSTR(path));
        bool ctrlPressed = (event->modifiers() & Qt::ControlModifier)!=0;
        if ( ctrlPressed || !_editMode )
            href = p.getHRef();
    }
    if ( href.empty() ) {
        //CRLog::trace("No href pressed" );
        if ( !p.isNull() && left ) {
            startSelection(p);
        }
    } else {
        CRLog::info("Link is selected: %s", UnicodeToUtf8(href).c_str() );
        if ( left ) {
            // link is pressed
            if ( _docview->goLink( href ) )
            {
                update();
                emit requestUpdateAll();
            }
        }
    }
    //CRLog::debug("mousePressEvent - doc pos (%d,%d), buttons: %d %d %d", pt.x, pt.y, (int)left, (int)right, (int)mid);
}

void CR3View::mouseReleaseEvent ( QMouseEvent * event )
{
    bool left = event->button() == Qt::LeftButton;
    //bool right = event->button() == Qt::RightButton;
    //bool mid = event->button() == Qt::MidButton;
    lvPoint pt (event->x(), event->y());
    ldomXPointer p = _docview->getNodeByPoint( pt );
    lString16 path;
    lString16 href;
    if ( !p.isNull() ) {
        path = p.toString();
        href = p.getHRef();
    }
    if ( _selecting )
        endSelection(p);
    if ( href.empty() ) {
        //CRLog::trace("No href pressed" );
        if ( !p.isNull() ) {
            //startSelection(p);
        }
    } else {
        CRLog::info("Link is selected: %s", UnicodeToUtf8(href).c_str() );
        if ( left ) {
            // link is pressed
            //if ( _docview->goLink( href ) )
            //    update();
        }
    }
    //CRLog::debug("mouseReleaseEvent - doc pos (%d,%d), buttons: %d %d %d", pt.x, pt.y, (int)left, (int)right, (int)mid);
    //FIXME: cite mode
    //stylusPan(event->pos(), begin_point_);

    if(dict_widget_.get() &&
       dict_widget_->isVisible() &&
       !getSelectionText().isEmpty() &&
       !qgetenv("DISABLE_DICT").toInt())
    {
        select_word_point_ = event->pos();
        update();
        lookup();
    }
    else if(!getSelectionText().isEmpty())
    {
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
        update();
    }
}

/// Override to handle external links
void CR3View::OnExternalLink( lString16 url, ldomNode * node )
{
    // TODO: add support of file links
    // only URL supported for now
    QUrl qturl( cr2qt(url) );
    QDesktopServices::openUrl( qturl );
}

/// create bookmark
CRBookmark * CR3View::createBookmark()
{
    CRBookmark * bm = NULL;
    /*
    if ( getSelectionText().length()>0 && !_selRange.isNull() ) {
        bm = _docview->saveRangeBookmark( _selRange, bmkt_comment, lString16() );
    } else {
    */
        bm = _docview->saveCurrentPageBookmark(lString16());
    //}

    return bm;
}

/// create cite
CRBookmark * CR3View::createCite()
{
    CRBookmark * bm = NULL;
    if ( getSelectionText().length()>0 && !_selRange.isNull() )
        bm = _docview->saveRangeBookmark( _selRange, bmkt_comment, lString16() );

    return bm;
}

void CR3View::goToBookmark( CRBookmark * bm )
{
    ldomXPointer start = _docview->getDocument()->createXPointer( bm->getStartPos() );
    ldomXPointer end = _docview->getDocument()->createXPointer( bm->getEndPos() );
    if ( start.isNull() )
        return;
    if ( end.isNull() )
        end = start;
    startSelection( start );
    endSelection( end );
    goToXPointer( cr2qt(bm->getStartPos()) );
    update();
}

/// rotate view, +1 = 90` clockwise, -1 = 90` counterclockwise
void CR3View::rotate( int angle )
{
    _docview->doCommand( DCMD_ROTATE_BY, angle );
    update();

//    int currAngle = _data->_props->getIntDef( PROP_ROTATE_ANGLE, 0 );
//    int newAngle = currAngle + angle;
//    newAngle = newAngle % 4;
//    if ( newAngle < 0 )
//        newAngle += 4;
//    if ( newAngle == currAngle )
//        return;
//    _docview->SetRotateAngle( (cr_rotate_angle_t) newAngle );
//    _data->_props->setInt( PROP_ROTATE_ANGLE, newAngle );
//    update();
}

/// format detection finished
void CR3View::OnLoadFileFormatDetected( doc_format_t fileFormat )
{
    QString filename = "fb2.css";
    if ( _cssDir.length() > 0 ) {
        switch ( fileFormat ) {
        case doc_format_txt:
            filename = "txt.css";
            break;
        case doc_format_rtf:
            filename = "rtf.css";
            break;
        case doc_format_epub:
            filename = "epub.css";
            break;
        case doc_format_html:
            filename = "htm.css";
            break;
        case doc_format_doc:
            filename = "doc.css";
            break;
        case doc_format_chm:
            filename = "chm.css";
            break;
        default:
            // do nothing
            break;
        }
        CRLog::debug( "CSS file to load: %s", filename.toUtf8().constData() );
        if ( QFileInfo( _cssDir + filename ).exists() ) {
            loadCSS( _cssDir + filename );
        } else if ( QFileInfo( _cssDir + "fb2.css" ).exists() ) {
            loadCSS( _cssDir + "fb2.css" );
        }
    }
}

/// on starting file loading
void CR3View::OnLoadFileStart( lString16 filename )
{
}

/// file load finiished with error
void CR3View::OnLoadFileError( lString16 message )
{
}

/// file loading is finished successfully - drawCoveTo() may be called there
void CR3View::OnLoadFileEnd()
{
    emit updateProgress(_docview->getCurPage()+1, _docview->getPageCount());
}

/// document formatting started
void CR3View::OnFormatStart()
{
}

/// document formatting finished
void CR3View::OnFormatEnd()
{
}

/// set bookmarks dir
void CR3View::setBookmarksDir( QString dirname )
{
    _bookmarkDir = dirname;
}

void CR3View::keyPressEvent ( QKeyEvent * event )
{
#if WORD_SELECTOR_ENABLED==1
    if ( isWordSelection() ) {
        MoveDirection dir = DIR_ANY;
        switch ( event->key() ) {
        case Qt::Key_Left:
        case Qt::Key_A:
            dir = DIR_LEFT;
            break;
        case Qt::Key_Right:
        case Qt::Key_D:
            dir = DIR_RIGHT;
            break;
        case Qt::Key_W:
        case Qt::Key_Up:
            dir = DIR_UP;
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            dir = DIR_DOWN;
            break;
        case Qt::Key_Q:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
            {
                QString text = endWordSelection();
                event->setAccepted(true);
                CRLog::debug("Word selected: %s", LCSTR(qt2cr(text)));
            }
            return;
        case Qt::Key_Backspace:
            _wordSelector->reducePattern();
            update();
            break;
        default:
            {
                int key = event->key();
                if ( key>=Qt::Key_A && key<=Qt::Key_Z ) {
                    QString text = event->text();
                    if ( text.length()==1 ) {
                        _wordSelector->appendPattern(qt2cr(text));
                        update();
                    }
                }
            }
            event->setAccepted(true);
            return;
        }
        int dist = event->modifiers() & Qt::ShiftModifier ? 5 : 1;
        _wordSelector->moveBy(dir, dist);
        update();
        event->setAccepted(true);
    } else {
        if ( event->key()==Qt::Key_F3 && (event->modifiers() & Qt::ShiftModifier) ) {
            startWordSelection();
            event->setAccepted(true);
            return;
        }

    }
#endif
    if ( !_editMode )
        return;
    switch ( event->key() ) {
    case Qt::Key_Left:
        break;
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        break;
    }
}

void CR3View::keyReleaseEvent(QKeyEvent * event)
{
    if ( (tts_widget_) &&
         (tts_widget_->isVisible()))
    {
        switch (event->key())
        {
        case Qt::Key_Escape:
            {
                stopTTS();
                break;
            }
        case Qt::Key_PageUp:
            {
                this->prevPageWithTTSChecking();
                break;
            }
        case Qt::Key_PageDown:
            {
                this->nextPageWithTTSChecking();
                break;
            }
        }
    }
    else
    {
        QWidget::keyReleaseEvent(event);
    }
}

/// file progress indicator, called with values 0..100
void CR3View::OnLoadFileProgress( int percent )
{
    CRLog::info( "OnLoadFileProgress(%d%%)", percent );
}

/// format progress, called with values 0..100
void CR3View::OnFormatProgress( int percent )
{
    CRLog::info( "OnFormatProgress(%d%%)", percent );
}

/// first page is loaded from file an can be formatted for preview
void CR3View::OnLoadFileFirstPagesReady()
{
#if 0 // disabled
	if ( !_data->_props->getBoolDef( PROP_PROGRESS_SHOW_FIRST_PAGE, 1 ) ) {
        CRLog::info( "OnLoadFileFirstPagesReady() - don't paint first page because " PROP_PROGRESS_SHOW_FIRST_PAGE " setting is 0" );
        return;
    }
    CRLog::info( "OnLoadFileFirstPagesReady() - painting first page" );
    _docview->setPageHeaderOverride(qt2cr(tr("Loading: please wait...")));
    //update();
    repaint();
    CRLog::info( "OnLoadFileFirstPagesReady() - painting done" );
    _docview->setPageHeaderOverride(lString16());
    _docview->requestRender();
    // TODO: remove debug sleep
    //sleep(5);
#endif
}

bool CR3View::hasBookmark()
{
    int now_page = getCurPage();
    CRFileHistRecord * rec = _docview->getCurrentFileHistRecord();
    if ( !rec )
        return false;
    LVPtrVector<CRBookmark> & list( rec->getBookmarks() );

    for(int i  = 0; i < list.length(); i++)
    {
        CRBookmark * bmk = list[i];
        if (!bmk || (bmk->getType() == bmkt_comment || bmk->getType() == bmkt_correction))
            continue;

        ldomXPointer pt1 = _docview->getDocument()->createXPointer( list[i]->getStartPos() );
        ldomXPointer pt2 = _docview->getDocument()->createXPointer( list[i]->getStartPos() );
        if( _docview->getBookmarkPage(pt1) == now_page ||
            _docview->getBookmarkPage(pt2) == now_page )
        {
            return true;
        }
    }
    return false;
}

void CR3View::paintBookmark( QPainter & painter )
{
    QPoint pt(rect().width()- bookmark_image_.width(), 0);
    painter.drawImage(pt, bookmark_image_);
}

void CR3View::deleteBookmark()
{
    int now_page = getCurPage();
    CRFileHistRecord * rec = _docview->getCurrentFileHistRecord();
    if ( !rec )
        return;
    LVPtrVector<CRBookmark> & list( rec->getBookmarks() );

    for(int i  = 0; i < list.length(); i++)
    {
        ldomXPointer pt1 = _docview->getDocument()->createXPointer( list[i]->getStartPos() );
        ldomXPointer pt2 = _docview->getDocument()->createXPointer( list[i]->getStartPos() );
        if( _docview->getBookmarkPage(pt1) == now_page ||
            _docview->getBookmarkPage(pt2) == now_page )
        {
            list.remove(i);
            return;
        }
    }
}

void CR3View::collectTTSContent()
{
    text_to_speak_.clear();
    tts_paragraph_index_ = 0;
    QString text_tmp = cr2qt( _docview->getPageText(false, -1) );
    text_to_speak_.push_back(text_tmp);
}

bool CR3View::hasPendingTTSContent()
{
    return (tts_paragraph_index_ < text_to_speak_.size());
}

void CR3View::startTTS()
{
    hideHelperWidget(search_widget_.get());
    hideHelperWidget(dict_widget_.get());
    collectTTSContent();
    if (hasPendingTTSContent())
    {
        ttsWidget().ensureVisible();
        ttsWidget().speak(text_to_speak_.at(tts_paragraph_index_++));
    }
    else
    {
        onSpeakDone();
    }
}

void CR3View::onSpeakDone()
{
    if (!hasPendingTTSContent())
    {
        if ( (_docview->getCurPage()+1) != _docview->getPageCount())
        {
            nextPage();
            emit requestUpdateAll();
            startTTS();
        }
        else
        {
            ttsWidget().stop();
        }
    }
    else
    {
        tts().speak(text_to_speak_.at(tts_paragraph_index_++));
    }
}

void CR3View::stopTTS()
{
    ttsWidget().stop();
    hideHelperWidget(tts_widget_.get());
}

tts::TTSWidget & CR3View::ttsWidget()
{
    if (!tts_widget_)
    {
        tts_widget_.reset(new tts::TTSWidget(this, tts()));
        tts_widget_->installEventFilter(this);
        connect(tts_widget_.get(), SIGNAL(closed()), this, SLOT(stopTTS()));
    }
    return *tts_widget_;
}

tts::TTS & CR3View::tts()
{
    if (!tts_engine_)
    {
        tts_engine_.reset(new tts::TTS(QLocale::system()));
        connect(tts_engine_.get(), SIGNAL(speakDone()), this , SLOT(onSpeakDone()));
    }
    return *tts_engine_;
}

void CR3View::hideHelperWidget(QWidget * wnd)
{
    if (wnd)
    {
        wnd->hide();
    }
}

bool CR3View::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (key_event->key() == Qt::Key_Escape)
        {
            return true;
        }
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (key_event->key() == Qt::Key_PageDown ||
            key_event->key() == Qt::Key_PageUp ||
            key_event->key() == Qt::Key_Escape)
        {
            keyReleaseEvent(key_event);
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

void CR3View::startDictLookup()
{
    if (!dicts_)
    {
        dicts_.reset(new DictionaryManager);
    }

    if (!dict_widget_)
    {
        dict_widget_.reset(new ui::DictWidget(this, *dicts_, &(tts())) );
        connect(dict_widget_.get(), SIGNAL(keyReleaseSignal(int)), this, SLOT(processKeyReleaseEvent(int)));
        connect(dict_widget_.get(), SIGNAL(closeClicked()), this, SLOT(onDictClosed()));
    }

    hideHelperWidget(search_widget_.get());
    hideHelperWidget(&(ttsWidget()));

    // When dictionary widget is not visible, it's necessary to update the text view.
    dict_widget_->lookup(getSelectionText());
    dict_widget_->ensureVisible(selected_rect_, true);
}

void CR3View::showSearchWidget()
{
    if (!search_widget_)
    {
        search_widget_.reset(new ui::OnyxSearchDialog(this, search_context_));
        connect(search_widget_.get(), SIGNAL(search(OnyxSearchContext &)),
            this, SLOT(onSearch(OnyxSearchContext &)));
        connect(search_widget_.get(), SIGNAL(closeClicked()), this, SLOT(onSearchClosed()));

        sys::SystemConfig conf;
        onyx::screen::watcher().addWatcher(this, conf.screenUpdateGCInterval());
    }

    search_context_.userData() = BEFORE_SEARCH;
    hideHelperWidget(dict_widget_.get());
    hideHelperWidget(&(ttsWidget()));
    search_widget_->showNormal();
}

void CR3View::onDictClosed()
{
    dict_widget_.reset(0);
    search_tool_->onCloseSearch();
}

void CR3View::lookup()
{
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
    if (!dict_widget_)
    {
        startDictLookup();
    }

    adjustDictWidget();
    dict_widget_->lookup(getSelectionText());
}

bool CR3View::adjustDictWidget()
{
    if(dict_widget_.get()->isVisible())
    {
        QPoint point = getSelectWordPoint();
        selected_rect_.setCoords(point.x(), point.y(), point.x()+1, point.y()+1);
    }

    return dict_widget_->ensureVisible(selected_rect_);
}

void CR3View::onSearch(OnyxSearchContext& context)
{
    if (search_context_.userData() <= BEFORE_SEARCH)
    {
        search_tool_->setSearchPattern(context.pattern());
        search_tool_->setReverse(!context.forward());
        search_context_.userData() = IN_SEARCHING;

        // TODO
        updateSearchWidget();
    }
    else
    {
        updateSearchWidget();
    }
}

void CR3View::onSearchClosed()
{
    //OnyxMainWindow->doAction("clearSearchResult");
    search_tool_->onCloseSearch();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
}

bool CR3View::updateSearchWidget()
{
    search_tool_->setReverse(!search_context_.forward());
    if (search_context_.forward())
    {
        if (!search_tool_->FindNext())
        {
            search_widget_->noMoreMatches();
            return false;
        }
    }
    else
    {
        if (!search_tool_->FindNext())
        {
            search_widget_->noMoreMatches();
            return false;
        }
    }
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
    return true;
}

void CR3View::processKeyReleaseEvent(int key)
{
    switch (key)
    {
    case Qt::Key_Escape:
        hideHelperWidget(dict_widget_.get());
        onDictClosed();
        break;
    case Qt::Key_PageDown:
        nextPageWithTTSChecking();
        break;
    case Qt::Key_PageUp:
        prevPageWithTTSChecking();
        break;
    default:
        if (dict_widget_->isInRetrieveWordsMode()) {
            switch (key) {
                case Qt::Key_Up:
                    this->selectUpWord();
                    break;
                case Qt::Key_Down:
                    this->selectDownWord();
                    break;
                case Qt::Key_Left:
                    this->selectPreviousWord();
                    break;
                case Qt::Key_Right:
                    this->selectNextWord();
                    break;
                case Qt::Key_Enter: // fall through
                case Qt::Key_Return:
                    this->lookup();
                    break;
                default:
                    break;
            }
        }
        break;
    }
}



void CR3View::stylusPan(const QPoint &now, const QPoint &old)
{
    int direction = sys::SystemConfig::direction(old, now);

    if (direction > 0)
    {
        clearSelection();
        nextPageWithTTSChecking();
    }
    else if (direction < 0)
    {
        clearSelection();
        prevPageWithTTSChecking();
    }
    else
    {
        if( dict_widget_ &&
            dict_widget_->isVisible() &&
            !getSelectionText().isEmpty())
        {
            select_word_point_ = now;
            update();
            lookup();
        }
        else
        {
            direction = sys::SystemConfig::whichArea(old, now);
            if (direction > 0)
            {
                nextPageWithTTSChecking();
            }
            else if (direction < 0)
            {
                prevPageWithTTSChecking();
            }
        }
    }
}

bool CR3View::docToWindowRect(lvRect &rect)
{
    lvPoint top_left = rect.topLeft();
    if (!_docview->docToWindowPoint(top_left)) {
        CRLog::error("(%s, %d) docToWindowPoint failed (%d, %d)", __FILE__,
                        __LINE__, top_left.x, top_left.y);
        return false;
    }

    lvPoint bottom_right = rect.bottomRight();
    if (!_docview->docToWindowPoint(bottom_right)) {
        CRLog::error("(%s, %d) docToWindowPoint failed (%d, %d)", __FILE__,
                __LINE__, bottom_right.x, bottom_right.y);
        return false;
    }

    rect.setTopLeft(top_left);
    rect.setBottomRight(bottom_right);

    return true;
}

bool CR3View::getPreviousWord(ldomXRange *range, bool *meetPageBoundary)
{
    *meetPageBoundary = false;

    ldomXRange test_range = ldomXRange(range->getStart(), range->getEnd());

    if (!test_range.getStart().prevVisibleWordStart()) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }
    if (!test_range.getEnd().prevVisibleWordEnd()) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }

    // to deal with cross-line range, we have to use range's end point instead of the range for locating. joy@onyx
    lvRect test_start_rect;
    if (!test_range.getStart().getRect(test_start_rect)) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }

    LVRendPageList *page_list = _docview->getPageList();
    LVRendPageInfo *cur_page = (*page_list)[_docview->getCurPage()];


    if (test_start_rect.top >= cur_page->start) {
        *range = test_range;
        return true;
    }
    else {
        *meetPageBoundary = true;
        return false;
    }
}

bool CR3View::getNextWord(ldomXRange *range, bool *meetPageBoundary)
{
    *meetPageBoundary = false;

    ldomXRange test_range = ldomXRange(range->getStart(), range->getEnd());

    if (!test_range.getStart().nextVisibleWordStart()) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }
    if (!test_range.getEnd().nextVisibleWordEnd()) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }

    // to deal with cross-line range, we have to use range's end point instead of the range for locating. joy@onyx
    lvRect test_end_rect;
    if (!test_range.getEnd().getRect(test_end_rect)) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }

    LVRendPageList *page_list = _docview->getPageList();
    LVRendPageInfo *cur_page = (*page_list)[_docview->getCurPage()];


    if (test_end_rect.bottom <= (cur_page->start + cur_page->height)) {
        *range = test_range;
        return true;
    }
    else {
        *meetPageBoundary = true;
        return false;
    }
}

bool CR3View::getUpWord(ldomXRange *range)
{
    // to deal with cross-line range, we have to use range's end points instead of the range for locating. joy@onyx
    lvRect src_start_rect;
    if (!range->getStart().getRect(src_start_rect)) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }

    lvRect src_end_rect;
    if (!range->getStart().getRect(src_end_rect)) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }

    ldomXRange test_range = ldomXRange(range->getStart(),
            range->getEnd());
    ldomXRange prev_range = ldomXRange();
    lvRect prev_start_rect = src_start_rect;
    lvRect prev_end_rect = src_end_rect;

    do {
        // word finding is a bit intricate, please be careful to maintain it. joy@onyx
        bool page_boundary = false;
        if (!this->getPreviousWord(&test_range, &page_boundary)) {
            if (page_boundary) {
                if (prev_start_rect.bottom <= src_start_rect.top) {
                    // already in up line, prev_range is the best choice
                    *range = prev_range;
                }
                break;
            }
            else {
                return false;
            }
        }

        lvRect test_start_rect;
        if (!test_range.getStart().getRect(test_start_rect)) {
            CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
            return false;
        }

        lvRect test_end_rect;
        if (!test_range.getEnd().getRect(test_end_rect)) {
            CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
            return false;
        }

        // when going up, choosing which word is a problem,
        // here we choose the approach that using range->getStart()'s cursor as standard.
        // joy@onyx
        if ((test_start_rect.bottom <= src_start_rect.top)) {
            if ((prev_start_rect.bottom <= src_start_rect.top)
                    && (test_start_rect.bottom <= prev_start_rect.top)) {
                // not crossing to 2nd up line
                *range = prev_range;
                break;
            } else if (test_start_rect.left < src_start_rect.left) {
                if (test_end_rect.left <= src_start_rect.left) {
                    if (test_end_rect.bottom > src_start_rect.top) {
                        // test_range crossing current line and up line
                        *range = test_range;
                        break;
                    } else if (prev_start_rect.bottom > src_start_rect.top) {
                        // prev_range in the same line with current range,
                        // in this case, test_range is the only choice
                        *range = test_range;
                        break;
                    }
                    else {
                        *range = prev_range;
                        break;
                    }
                } else {
                    *range = test_range;
                    break;
                }
            }
        }

        prev_range = test_range;
        prev_start_rect = test_start_rect;
        prev_end_rect = test_end_rect;
    } while (true);

    return true;
}

bool CR3View::getDownWord(ldomXRange *range)
{
    // to deal with cross-line range, we have to use range's end points instead of the range for locating. joy@onyx
    lvRect src_start_rect;
    if (!range->getStart().getRect(src_start_rect)) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }

    lvRect src_end_rect;
    if (!range->getStart().getRect(src_end_rect)) {
        CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
        return false;
    }

    ldomXRange test_range = ldomXRange(range->getStart(), range->getEnd());
    ldomXRange prev_range = ldomXRange();
    lvRect prev_start_rect = src_start_rect;
    lvRect prev_end_rect = src_end_rect;

    do {
        // word finding is a bit intricate, please be careful to maintain it. joy@onyx
        bool page_boundary = false;
        if (!this->getNextWord(&test_range, &page_boundary)) {
            if (page_boundary) {
                if (prev_start_rect.top <= src_start_rect.bottom) {
                    // already in down line, prev_range is the best choice
                    *range = prev_range;
                }
                break;
            } else {
                return false;
            }
        }

        lvRect test_start_rect;
        if (!test_range.getStart().getRect(test_start_rect)) {
            CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
            return false;
        }

        lvRect test_end_rect;
        if (!test_range.getEnd().getRect(test_end_rect)) {
            CRLog::error("(%s, %d) function failed", __FILE__, __LINE__);
            return false;
        }

        // when going down, selecting which word is a problem,
        // here we choose the approach that using range->getStart()'s cursor as standard.
        // joy@onyx
        if ((test_start_rect.top >= src_start_rect.bottom)) {
            if ((prev_start_rect.top >= src_start_rect.bottom)
                    && (test_start_rect.top >= prev_start_rect.bottom)) {
                // try not to cross to 2nd down line
                *range = prev_range;
                break;
            } else if (test_end_rect.right > src_start_rect.left) {
                *range = test_range;
                break;
            }
        }

        prev_range = test_range;
        prev_start_rect = test_start_rect;
        prev_end_rect = test_end_rect;
    } while (true);

    return true;
}

bool CR3View::selectPreviousWord()
{
    ldomXRangeList range_list =  _docview->getDocument()->getSelections();
    if (range_list.empty()) {
        assert(false);
        CRLog::error("(%s, %d) current selection range should not be empty", __FILE__, __LINE__);
        return false;
    }
    // TODO should delete after used? joy@onyx
    ldomXRange *current_range = range_list.first();
    LVArray<ldomWord> range_words;
    current_range->getRangeWords(range_words);
    if (range_words.length() != 1) {
        // TODO such as "e-reader" will be in selection, how to handle this situation? joy@onyx
//        return false;
    }

    bool page_boundary = false;
    if (!this->getPreviousWord(current_range, &page_boundary)) {
        if (!page_boundary) {
            return false;
        }
    }

    this->clearSelection();

    if (!this->updateSelection(current_range)) {
        return false;
    }

    return true;
}

bool CR3View::selectNextWord()
{
    ldomXRangeList range_list = _docview->getDocument()->getSelections();
    if (range_list.empty()) {
        assert(false);
        CRLog::error("(%s, %d) current selection range should not be empty", __FILE__, __LINE__);
        return false;
    }
    // TODO should delete after used? joy@onyx
    ldomXRange *current_range = range_list.first();
    LVArray<ldomWord> range_words;
    current_range->getRangeWords(range_words);
    if (range_words.length() != 1) {
        // TODO such as "e-reader" will be in selection, how to handle this situation? joy@onyx
//            return false;
    }

    bool page_boundary = false;
    if (!this->getNextWord(current_range, &page_boundary)) {
        if (!page_boundary) {
            return false;
        }
    }

    this->clearSelection();

    if (!this->updateSelection(current_range)) {
        return false;
    }

    return true;
}
bool CR3View::selectUpWord()
{
    ldomXRangeList range_list = _docview->getDocument()->getSelections();
    if (range_list.empty()) {
        assert(false);
        CRLog::error("(%s, %d) current selection range should not be empty", __FILE__, __LINE__);
        return false;
    }
    // TODO should delete after used? joy@onyx
    ldomXRange *current_range = range_list.first();
    LVArray<ldomWord> range_words;
    current_range->getRangeWords(range_words);
    if (range_words.length() != 1) {
        // TODO such as "e-reader" will be in selection, how to handle this situation? joy@onyx
        //            return false;
    }

    if (!this->getUpWord(current_range)) {
        return false;
    }

    this->clearSelection();

    if (!this->updateSelection(current_range)) {
        return false;
    }

    return true;
}
bool CR3View::selectDownWord()
{
    ldomXRangeList range_list = _docview->getDocument()->getSelections();
    if (range_list.empty()) {
        assert(false);
        CRLog::error("(%s, %d) current selection range should not be empty",
                __FILE__, __LINE__);
        return false;
    }
    // TODO should delete after used? joy@onyx
    ldomXRange *current_range = range_list.first();
    LVArray<ldomWord> range_words;
    current_range->getRangeWords(range_words);
    if (range_words.length() != 1) {
        // TODO such as "e-reader" will be in selection, how to handle this situation? joy@onyx
        //            return false;
    }

    if (!this->getDownWord(current_range)) {
        return false;
    }

    this->clearSelection();

    if (!this->updateSelection(current_range)) {
        return false;
    }

    return true;
}
