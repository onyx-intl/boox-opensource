// -*- mode: c++; c-basic-offset: 4; -*-

#include "add_feed_dialog.h"

#include <QDebug>
#include <QtGui>

#include "onyx/ui/keyboard.h"
#include "onyx/ui/buttons.h"
#include "onyx/ui/line_edit.h"
#include "onyx/ui/label.h"

#include "singleton.h"
#include "widget_updater.h"

namespace onyx {
namespace feed_reader {

using onyx::screen::ScreenProxy;

AddFeedDialog::AddFeedDialog(QWidget *parent)
        : QDialog(parent),
          key_board_(new ui::KeyBoard),
          add_button_(new ui::OnyxPushButton(tr("Add"), this)),
          cancel_button_(new ui::OnyxPushButton(tr("Cancel"), this)),
          url_edit_(new ui::OnyxLineEdit("http://", this)),
          url_("") {
    ui::OnyxLabel *findLabel = new ui::OnyxLabel(tr("Feed address:"));
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(findLabel);
    hlayout->addWidget(url_edit_);
    hlayout->addWidget(add_button_);
    hlayout->addWidget(cancel_button_);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setSpacing(1);
    vlayout->setContentsMargins(5, 1, 5, 1);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(key_board_);
    setLayout(vlayout);
    setWindowTitle(tr("Add a new feed"));
    connect(add_button_, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(cancel_button_, SIGNAL(clicked()), this, SLOT(reject()));
    connect(cancel_button_, SIGNAL(clicked()), url_edit_, SLOT(setFocus()));
    url_edit_->setFocus();
    key_board_->attachReceiver(url_edit_);
    WidgetUpdater& updater(Singleton<WidgetUpdater>::instance());
    updater.addWidget(this, ScreenProxy::GU);
    updater.addWidget(url_edit_, ScreenProxy::GC);
    updater.addWidget(add_button_, ScreenProxy::GU);
    updater.addWidget(cancel_button_, ScreenProxy::GU);
    setModal(true);
}

AddFeedDialog::~AddFeedDialog() {}

void AddFeedDialog::addClicked() {
    url_ = url_edit_->text();

    if (url_.isEmpty()) {
        QMessageBox::information(
                this, tr("Empty URL"),
                tr("Please enter an address to add a feed from."));
        url_edit_->setFocus();
    } else if (!url_.isValid() || url_.host().isEmpty()) {
        QMessageBox::information(this, tr("Invalid URL"),
                                 tr("Please enter a valid URL."));
        url_edit_->setFocus();
    } else if (url_.scheme() != "http" && url_.scheme() != "https") {
        QMessageBox::information(this, tr("Unsupported protocol"),
                                 tr("Only http and https are supported."));
        url_edit_->setFocus();
    } else {
        accept();
        url_edit_->setFocus();
        url_edit_->setText("http://");
    }
}

}  // namespace feed_reader
}  // namespace onyx
