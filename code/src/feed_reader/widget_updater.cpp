// -*- mode: c++; c-basic-offset: 4; -*-

#include "widget_updater.h"

#include <QEvent>
#include <QKeyEvent>
#include <QWidget>
#include <QtGlobal>

#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "onyx/screen/screen_proxy.h"

namespace onyx {
namespace feed_reader {

using onyx::screen::ScreenProxy;

QEvent::Type DELAYED_UPDATE_EVENT_TYPE;

WidgetUpdater::WidgetUpdater()
        : waveform_map_(),
          default_waveform_(ScreenProxy::GU),
          ignore_next_update_(false),
          force_next_update_(false) {
    masked_events_.insert(QEvent::Enter);
    masked_events_.insert(QEvent::Leave);
    masked_events_.insert(QEvent::HoverEnter);
    masked_events_.insert(QEvent::HoverLeave);
    masked_events_.insert(QEvent::CursorChange);
    masked_events_.insert(QEvent::MouseMove);
}

void WidgetUpdater::addWidget(QWidget* widget, Waveform waveform) {
    if (waveform_map_.count(widget) > 0) {
        qCritical("Widget at %p is already in the WidgetUpdater!",
                  widget);
        return;
    }
    waveform_map_[widget] = waveform;
    widget->installEventFilter(this);
}

bool WidgetUpdater::eventFilter(QObject* obj, QEvent* event) {
    if (masked_events_.count(event->type()) > 0 ||
        (event->type() == QEvent::KeyPress &&
         down_cast<QKeyEvent*>(event)->key() == Qt::Key_F23)) {
        // qDebug() << "Event type: " << event->type() << "(masked)";
        ignore_next_update_ = true;
        return QObject::eventFilter(obj, event);
    } else {
        if (event->type() != QEvent::UpdateRequest) {
            ignore_next_update_ = false;
        }
        // qDebug() << "Event type: " << event->type();
    }

    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease) {
        force_next_update_ = true;
    } else if (event->type() == QEvent::UpdateRequest) {
        if (ignore_next_update_ && !force_next_update_) {
            ignore_next_update_ = false;
            qDebug() << "UpdateRequest ignored";
            return QObject::eventFilter(obj, event);
        }
        force_next_update_= false;
        ignore_next_update_= false;
        QCoreApplication::postEvent(obj, new QEvent(DELAYED_UPDATE_EVENT_TYPE));
    } else if (event->type() == DELAYED_UPDATE_EVENT_TYPE) {
        Waveform waveform = default_waveform_;
        QWidget* widget = down_cast<QWidget*>(obj);
        map<QWidget*, Waveform>::iterator wf_iter = waveform_map_.find(widget);
        if (wf_iter == waveform_map_.end()) {
            qCritical("Widget at %p is not in the WidgetUpdater!", widget);
        } else {
            waveform = wf_iter->second;
        }
        onyx::screen::instance().flush(down_cast<QWidget*>(obj),
                                              waveform);
    }
    return QObject::eventFilter(obj, event);
}

void WidgetUpdater::postDelayedUpdate(QWidget* w) {
    QCoreApplication::postEvent(w, new QEvent(DELAYED_UPDATE_EVENT_TYPE));
}

}  // namespace feed_reader
}  // namespace onyx
