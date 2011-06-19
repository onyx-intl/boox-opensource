// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_WIDGET_UPDATER__
#define ONYX_FEED_READER_WIDGET_UPDATER__

#include <map>

#include <QObject>
#include <QEvent>

#include "onyx/screen/screen_proxy.h"

namespace onyx {
namespace feed_reader {

extern QEvent::Type DELAYED_UPDATE_EVENT_TYPE;

/// A WidgetUpdater takes care of calling ScreenProxy::updateWidget
/// for a set of QWidgets. Typically, only one WidgetUpdater instance
/// is needed in an application.
class WidgetUpdater : QObject {
    Q_OBJECT;
    typedef onyx::screen::ScreenProxy::Waveform Waveform;
  public:
    WidgetUpdater();

    /// Install the WidgetUpdater as an EventFilter for the
    /// widget. The WidgetUpdater only processes the UpdateRequest
    /// event. It updates the widget using the given waveform.
    void addWidget(QWidget* widget, Waveform waveform);

    void postDelayedUpdate(QWidget* widget);
  protected:
    bool eventFilter(QObject* obj, QEvent* event);

  private:
    map<QWidget*, Waveform> waveform_map_;
    set<QEvent::Type> masked_events_;
    Waveform default_waveform_;
    bool ignore_next_update_;
    bool force_next_update_;
};

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_WIDGET_UPDATER__
