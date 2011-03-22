#include "djvu_application.h"
#include "djvu_view.h"
#include "djvu_thumbnail_view.h"

namespace djvu_reader
{

DjvuApplication::DjvuApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , main_window_(this)
    , model_(argv[0])
{
    if (argc > 1)
    {
        ui::loadTranslator(QLocale::system().name());
        current_path_ = QString::fromLocal8Bit(argv[1]);
    }
}

DjvuApplication::~DjvuApplication(void)
{
    close( current_path_ );
}

bool DjvuApplication::open( const QString &path_name )
{
    main_window_.attachModel(&model_);
    main_window_.show();

    sys::SystemConfig conf;
    onyx::screen::watcher().addWatcher(&main_window_,
            conf.screenUpdateGCInterval());

    DjvuView *view = down_cast<DjvuView*>(main_window_.activateView(DJVU_VIEW));

    // connect the signals with view
    connect( view, SIGNAL(rotateScreen()), this, SLOT(onRotateScreen()) );
    connect( view, SIGNAL(testSuspend()), this, SLOT(onSuspend()) );
    connect( view, SIGNAL(testWakeUp()), this, SLOT(onWakeUp()) );

    // connect the signals with sys_state_
    SysStatus & sys_status = SysStatus::instance();
    connect( &sys_status, SIGNAL( mountTreeSignal( bool, const QString & ) ),
             this, SLOT( onMountTreeSignal( bool, const QString &) ) );
    connect( &sys_status, SIGNAL( sdCardChangedSignal( bool ) ), this, SLOT( onSDChangedSignal( bool ) ) );
    connect( &sys_status, SIGNAL( aboutToShutdown() ), this, SLOT( onAboutToShutDown() ) );
    connect( &sys_status, SIGNAL( wakeup() ), this, SLOT( onWakeUp() ) );

    // connect the long press and multi-point signals
    connect(&sys_status, SIGNAL( mouseLongPress(QPoint, QSize) ), view,
            SLOT( onMouseLongPress(QPoint, QSize) ));
    connect(&sys_status, SIGNAL( multiTouchPressDetected(QRect, QRect) ), view,
            SLOT( onMultiTouchPressDetected(QRect, QRect) ));
    connect(&sys_status, SIGNAL( multiTouchReleaseDetected(QRect, QRect) ),
            view, SLOT( onMultiTouchReleaseDetected(QRect, QRect) ));

#ifdef Q_WS_QWS
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif

    view->attachModel(&model_);
    bool ret = model_.open(path_name);
    if ( !ret )
    {
        if ( sys::SysStatus::instance().isSystemBusy() )
        {
            // if loading fails, set busy to be false
            sys::SysStatus::instance().setSystemBusy( false );
        }
        view->deattachModel();
    }
    return ret;
}

bool DjvuApplication::isOpened()
{
    return model_.isReady();
}

bool DjvuApplication::errorFound()
{
    // TODO. Implement Me
    return false;
}

bool DjvuApplication::close(const QString &path_name)
{
    model_.save();
    model_.close();
    main_window_.clearViews();
    return true;
}

bool DjvuApplication::onSuspend()
{
    // save all of the options for waking up
    return model_.save();
}

void DjvuApplication::onWakeUp()
{
    // set the system to be busy at this moment
    sys::SysStatus::instance().setSystemBusy(true, false);

    // save the configurations before closing
    model_.save();

    // close the document because it might be already invalid
    model_.close();

    // open the document again
    model_.open( current_path_ );
    sys::SysStatus::instance().setSystemBusy(false, false);
}

void DjvuApplication::onAboutToShutDown()
{
    qDebug("System is about to shut down");
    qApp->exit();
}

void DjvuApplication::onUSBSignal(bool inserted)
{
    qDebug("USB %s", inserted ? "inserted" : "disconnect");
    if ( model_.path().startsWith( USB_ROOT ) && !inserted )
    {
        qApp->exit();
    }
}

void DjvuApplication::onMountTreeSignal(bool inserted, const QString &mount_point)
{
    qDebug( "Mount point:%s %s",
            qPrintable( mount_point ),
            inserted ? "inserted" : "disconnect" );
    if ( !inserted && model_.path().startsWith( mount_point ) )
    {
        qApp->exit();
    }
}

void DjvuApplication::onSDChangedSignal(bool inserted)
{
    qDebug("SD %s", inserted ? "inserted" : "disconnect");
    if ( model_.path().startsWith( SDMMC_ROOT ) && !inserted )
    {
        qApp->exit();
    }
}

void DjvuApplication::onConnectToPCSignal(bool connected)
{
    qDebug("Connection to PC%s", connected ? "connected" : "disconnected");
    if ( connected )
    {
        qApp->exit();
    }
}

void DjvuApplication::onBatterySignal(const int, const int, bool)
{
    qDebug("Battery");
    // TODO. Implement Me
}

void DjvuApplication::onSystemIdleSignal()
{
    qDebug("System Idle");
    // TODO. Implement Me
}

void DjvuApplication::onRotateScreen()
{
    SysStatus::instance().rotateScreen();
}

void DjvuApplication::onScreenSizeChanged(int)
{
    onyx::screen::instance().enableUpdate(false);
    main_window_.resize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(&main_window_, onyx::screen::ScreenProxy::GC);
}

void DjvuApplication::onCreateView(int type, MainWindow* main_window, QWidget*& result)
{
    QWidget* view = 0;
    switch (type)
    {
    case DJVU_VIEW:
        view = new DjvuView(main_window);
        break;
    case TOC_VIEW:
        view = new TreeViewDialog(main_window);
        break;
    case THUMBNAIL_VIEW:
        view = new ThumbnailView(main_window);
        break;
    default:
        break;
    }
    result = view;
}

void DjvuApplication::onAttachView(int type, QWidget* view, MainWindow* main_window)
{
    switch (type)
    {
    case DJVU_VIEW:
        {
            dynamic_cast<DjvuView*>(view)->attachMainWindow(main_window);
        }
        break;
    case TOC_VIEW:
        {
#ifdef MAIN_WINDOW_TOC_ON
            QWidget* reading_view = main_window->getView(Djvu_VIEW);
            if (reading_view == 0)
            {
                return;
            }
            down_cast<TreeViewDialog*>(view)->attachMainWindow(main_window);
            down_cast<DjvuView*>(reading_view)->attachTreeView(down_cast<TreeViewDialog*>(view));
#endif
        }
        break;
    case THUMBNAIL_VIEW:
        down_cast<ThumbnailView*>(view)->attachMainWindow(main_window);
        break;
    default:
        break;
    }
}

void DjvuApplication::onDeattachView(int type, QWidget* view, MainWindow* main_window)
{
    switch (type)
    {
    case DJVU_VIEW:
        down_cast<DjvuView*>(view)->deattachMainWindow(main_window);
        break;
    case TOC_VIEW:
        {
#ifdef MAIN_WINDOW_TOC_ON
            QWidget* reading_view = main_window->getView(Djvu_VIEW);
            if (reading_view == 0)
            {
                return;
            }
            down_cast<TreeViewDialog*>(view)->deattachMainWindow(main_window);
            down_cast<DjvuView*>(reading_view)->deattachTreeView(down_cast<TreeViewDialog*>(view));
#endif
        }
        break;
    case THUMBNAIL_VIEW:
        down_cast<ThumbnailView*>(view)->deattachMainWindow(main_window);
        break;
    default:
        break;
    }
}

bool DjvuApplication::flip(int direction)
{
    QWidget* reading_view = main_window_.getView(DJVU_VIEW);
    if (reading_view == 0)
    {
        return false;
    }
    return down_cast<DjvuView*>(reading_view)->flip(direction);
}

bool DjvuApplicationAdaptor::flip(int direction)
{
    return app_->flip(direction);
}

}
