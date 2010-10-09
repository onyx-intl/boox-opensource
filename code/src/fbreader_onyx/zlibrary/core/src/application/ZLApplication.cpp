// -*- mode: c++; c-basic-offset: 4; -*-

/*
 * Copyright (C) 2004-2009 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <ZLibrary.h>

#include "ZLApplication.h"
#include "ZLApplicationWindow.h"
#include "ZLKeyBindings.h"
#include "ZLToolbar.h"
#include "ZLMenu.h"
#include "ZLPopupData.h"
#include "../view/ZLView.h"
#include "../view/ZLViewWidget.h"
#include "../view/ZLPaintContext.h"

#include "onyx/cms/content_thumbnail.h"

const std::string ZLApplication::MouseScrollDownKey = "<MouseScrollDown>";
const std::string ZLApplication::MouseScrollUpKey = "<MouseScrollUp>";
const std::string ZLApplication::NoAction = "none";

static const std::string ROTATION = "Rotation";
static const std::string ANGLE = "Angle";
static const std::string STATE = "State";
static const std::string KEYBOARD = "Keyboard";
static const std::string FULL_CONTROL = "FullControl";
static const std::string CONFIG = "Config";
static const std::string AUTO_SAVE = "AutoSave";
static const std::string TIMEOUT = "Timeout";

using namespace cms;
using namespace vbf;

ZLApplication::ZLApplication(const std::string &name)
        : ZLApplicationBase(name),
          RotationAngleOption(ZLCategoryKey::CONFIG, ROTATION, ANGLE, ZLView::DEGREES90),
          AngleStateOption(ZLCategoryKey::CONFIG, STATE, ANGLE, ZLView::DEGREES0),
          KeyboardControlOption(ZLCategoryKey::CONFIG, KEYBOARD, FULL_CONTROL, false),
          ConfigAutoSavingOption(ZLCategoryKey::CONFIG, CONFIG, AUTO_SAVE, false),
          ConfigAutoSaveTimeoutOption(ZLCategoryKey::CONFIG, CONFIG, TIMEOUT, 1, 6000, 30),
          KeyDelayOption(ZLCategoryKey::CONFIG, "Options", "KeyDelay", 0, 5000, 0),
          SearchPatternOption(ZLCategoryKey::CONFIG, CONFIG, "SearchPattern", ""),
          SearchForwardOption(ZLCategoryKey::CONFIG, CONFIG, "SearchForward", true),
          EncodingOption(ZLCategoryKey::CONFIG, CONFIG, "Encoding", ""),
          StateOption(ZLCategoryKey::CONFIG, CONFIG, "State", ""),
          content_ready(false),
          has_thumbnail(false),
          myViewWidget(0),
          myWindow(0),
          bookmarks_loaded_(false),
          bookmarks_dirty_(false)
{
    myContext.reset(ZLibrary::createContext());
    if (ConfigAutoSavingOption.value()) {
        ZLOption::startAutoSave(ConfigAutoSaveTimeoutOption.value());
    }

    myPresentWindowHandler.reset(new PresentWindowHandler(*this));
    ZLCommunicationManager::instance().registerHandler("present", myPresentWindowHandler);

    // createToolbar(ZLApplicationWindow::WINDOW_TOOLBAR);
    // createToolbar(ZLApplicationWindow::FULLSCREEN_TOOLBAR);
    // createMenubar();
}

ZLApplication::~ZLApplication() {
    saveDocumentOptions(document_path);
    saveBookmarks(document_path);

    if (myWindow != 0) {
        if (KeyboardControlOption.value()) {
            grabAllKeys(false);
        }
        delete myWindow;
    }
    if (myViewWidget != 0) {
        delete myViewWidget;
    }
}

bool ZLApplication::initWindow() {
    loadDocumentOptions(document_path);

    if (KeyboardControlOption.value()) {
        grabAllKeys(true);
    }
    myWindow->init();
    setView(myInitialView);
    return true;
}

bool ZLApplication::closeView() {
    quit();
    return true;
}

void ZLApplication::openFile(const std::string&) {
}

ZLApplicationWindow::ZLApplicationWindow(ZLApplication *application) : myApplication(application), myToggleButtonLock(false) {
    myApplication->myWindow = this;
}

void ZLApplicationWindow::init() {
    myApplication->myViewWidget = createViewWidget();

    // John: We don't need the toolbar and menu bar.
    // initToolbar(WINDOW_TOOLBAR);
    // initToolbar(FULLSCREEN_TOOLBAR);
    // initMenu();
}

void ZLApplicationWindow::initToolbar(ToolbarType type) {
    const ZLToolbar::ItemVector &toolbarItems = myApplication->toolbar(type).items();
    for (ZLToolbar::ItemVector::const_iterator it = toolbarItems.begin(); it != toolbarItems.end(); ++it) {
        addToolbarItem(*it);
    }
}

void ZLApplication::addAction(const std::string &actionId, shared_ptr<Action> action) {
    myActionMap[actionId] = action;
}

void ZLApplication::setView(shared_ptr<ZLView> view) {
    if (!view) {
        return;
    }

    if (myViewWidget != 0) {
        myViewWidget->setView(view);
        resetWindowCaption();
        refreshWindow();
    } else {
        myInitialView = view;
    }
}

shared_ptr<ZLView> ZLApplication::currentView() const {
    return (myViewWidget != 0) ? myViewWidget->view() : shared_ptr<ZLView>();
}

void ZLApplication::refreshWindow(bool process)
{
    if (myViewWidget != 0) {
        myViewWidget->repaint();
    }
    if (myWindow != 0) {
        myWindow->refresh(process);
    }
}

void ZLApplication::presentWindow() {
    if (myWindow != 0) {
        myWindow->present();
    }
}

void ZLApplication::Action::checkAndRun() {
    if (isEnabled()) {
        run();
    }
}

shared_ptr<ZLApplication::Action> ZLApplication::action(const std::string &actionId) const {
    std::map<std::string,shared_ptr<Action> >::const_iterator it = myActionMap.find(actionId);
    return (it != myActionMap.end()) ? it->second : shared_ptr<ZLApplication::Action>();
}

bool ZLApplication::isActionVisible(const std::string &actionId) const {
    shared_ptr<Action> a = action(actionId);
    return a && a->isVisible();
}

bool ZLApplication::isActionEnabled(const std::string &actionId) const {
    shared_ptr<Action> _action = action(actionId);
    return _action && _action->isEnabled();
}

void ZLApplication::doAction(const std::string &actionId) {
    shared_ptr<Action> _action = action(actionId);
    if (_action) {
        _action->checkAndRun();
    }
}

void ZLApplication::resetWindowCaption() {
    if (myWindow != 0) {
        if ((currentView() == 0) || (currentView()->caption().empty())) {
            myWindow->setCaption(ZLibrary::ApplicationName());
        } else {
            myWindow->setCaption(ZLibrary::ApplicationName() + " - " + currentView()->caption());
        }
    }
}

bool ZLApplication::loadDocumentOptions(const QString &path)
{
    ContentManager database;
    if (!openDatabase(path, database))
    {
        return false;
    }

    // Check thumbnail is ok or not.
    QFileInfo info(path);
    cms::ContentThumbnail thumbdb(info.absolutePath());
    has_thumbnail = thumbdb.hasThumbnail(info.fileName(), cms::THUMBNAIL_LARGE);

    return vbf::loadDocumentOptions(database, path, conf_);
}

bool ZLApplication::saveDocumentOptions(const QString &path)
{
    ContentManager database;
    if (!openDatabase(path, database))
    {
        return false;
    }

    conf_.options[CONFIG_ENCODING] = QString::fromAscii(EncodingOption.value().c_str());
    return vbf::saveDocumentOptions(database, path, conf_);
}

struct LessByPosition
{
    bool operator()( const Bookmark& a, const Bookmark& b ) const
    {
        return a.data().toInt() < b.data().toInt();
    }
};

Bookmarks & ZLApplication::bookmarks()
{
    if (!bookmarks_loaded_)
    {
        ContentManager database;
        if (!openDatabase(document_path, database))
        {
            return bookmarks_;
        }

        vbf::loadBookmarks(database, document_path, bookmarks_);
        bookmarks_loaded_ = true;
        bookmarks_dirty_ = false;
    }
    return bookmarks_;
}

bool ZLApplication::addBookmark(const vbf::Bookmark & bookmark)
{
    if (vbf::insertBookmark(bookmarks(), bookmark, LessByPosition()))
    {
        bookmarks_dirty_ = true;
        return true;
    }
    return false;
}

bool ZLApplication::removeBookmarks(int from, int to)
{
    vbf::Bookmarks & all = bookmarks();
    BookmarksIter begin = all.begin();
    BookmarksIter end   = all.end();
    for(BookmarksIter iter  = begin; iter != end && all.size() > 0;)
    {
        int abs_pos = iter->data().toInt();
        if (from <= abs_pos && abs_pos < to)
        {
            iter = all.erase(iter);
            bookmarks_dirty_ = true;
        }
        else
        {
            ++iter;
        }
    }
    return bookmarks_dirty_;
}

bool ZLApplication::clearBookmarks()
{
    vbf::Bookmarks & all = bookmarks();
    if (all.size() > 0)
    {
        all.clear();
        bookmarks_dirty_ = true;
        return true;
    }
    return false;
}

bool ZLApplication::hasBookmark(size_t from, size_t to)
{
    vbf::Bookmarks & all = bookmarks();
    BookmarksIter begin = all.begin();
    BookmarksIter end   = all.end();
    for(BookmarksIter iter  = begin; iter != end; ++iter)
    {
        size_t abs_pos = iter->data().toInt();
        if (from <= abs_pos && abs_pos < to)
        {
            return true;
        }
    }
    return false;
}

void ZLApplication::saveBookmarks(const QString &path)
{
    if (!bookmarks_loaded_ || !bookmarks_dirty_)
    {
        return;
    }
    ContentManager database;
    if (!openDatabase(path, database))
    {
        return;
    }

    vbf::saveBookmarks(database, path, bookmarks_);
    bookmarks_dirty_ = false;
}

bool ZLApplication::isTextSelectionEnabled()
{
    return (StateOption.value() == "text_selection");
}

void ZLApplication::enableTextSelection(bool b)
{
    if (b)
    {
        StateOption.setValue("text_selection");
    }
    else
    {
        StateOption.setValue("");
    }
}

Configuration & ZLApplication::conf()
{
    return conf_;
}

ZLApplication::Action::~Action() {
}

bool ZLApplication::Action::isVisible() const {
    return true;
}

bool ZLApplication::Action::isEnabled() const {
    return isVisible();
}

bool ZLApplication::Action::useKeyDelay() const {
    return true;
}

void ZLApplication::trackStylus(bool track) {
    if (myViewWidget != 0) {
        myViewWidget->trackStylus(track);
    }
}

void ZLApplicationWindow::refresh(bool process) {
    // John: No toolbar now.
    // refreshToolbar(WINDOW_TOOLBAR);
    // refreshToolbar(FULLSCREEN_TOOLBAR);
    if (process)
    {
        processAllEvents();
    }
    updateScreen();
}

void ZLApplicationWindow::refreshToolbar(ToolbarType type) {
    const ZLToolbar::ItemVector &items = application().toolbar(type).items();
    bool enableToolbarSpace = false;
    ZLToolbar::ItemPtr lastSeparator;
    for (ZLToolbar::ItemVector::const_iterator it = items.begin(); it != items.end(); ++it) {
        switch ((*it)->type()) {
            case ZLToolbar::Item::TEXT_FIELD:
            case ZLToolbar::Item::COMBO_BOX:
            case ZLToolbar::Item::PLAIN_BUTTON:
            case ZLToolbar::Item::MENU_BUTTON:
                {
                    ZLToolbar::ActionItem &button = (ZLToolbar::ActionItem&)**it;
                    const std::string &id = button.actionId();

                    const bool visible = application().isActionVisible(id);
                    const bool enabled = application().isActionEnabled(id);

                    if (visible) {
                        if (lastSeparator) {
                            setToolbarItemState(lastSeparator, true, true);
                            lastSeparator.reset();
                        }
                        enableToolbarSpace = true;
                    }
                    setToolbarItemState(*it, visible, enabled);
                }
                break;
            case ZLToolbar::Item::TOGGLE_BUTTON:
                {
                    ZLToolbar::ToggleButtonItem &button = (ZLToolbar::ToggleButtonItem&)**it;
                    const std::string &id = button.actionId();

                    const bool visible = application().isActionVisible(id);
                    const bool enabled = application().isActionEnabled(id);

                    if (visible) {
                        if (lastSeparator) {
                            setToolbarItemState(lastSeparator, true, true);
                            lastSeparator.reset();
                        }
                        enableToolbarSpace = true;
                    }
                    /*
                      if (!enabled && button.isPressed()) {
                      button.buttonGroup().press(0);
                      //application().doAction(group->UnselectAllButtonsActionId);
                      myToggleButtonLock = true;
                      setToggleButtonState(button);
                      myToggleButtonLock = false;
                      }
                    */
                    setToolbarItemState(*it, visible, enabled);
                    setToggleButtonState(button);
                }
                break;
            case ZLToolbar::Item::SEPARATOR:
                if (enableToolbarSpace) {
                    lastSeparator = *it;
                    enableToolbarSpace = false;
                } else {
                    setToolbarItemState(*it, false, true);
                }
                break;
        }
    }
    if (lastSeparator) {
        setToolbarItemState(lastSeparator, false, true);
    }
}

void ZLApplication::doActionByKey(const std::string &key) {
    shared_ptr<ZLKeyBindings> bindings = keyBindings();
    if (!bindings) {
        return;
    }
    shared_ptr<Action> a = action(bindings->getBinding(key));
    if (a &&
        (!a->useKeyDelay() ||
         (myLastKeyActionTime.millisecondsTo(ZLTime()) >= KeyDelayOption.value()))) {
        a->checkAndRun();
        myLastKeyActionTime = ZLTime();
    }
}

void ZLApplication::grabAllKeys(bool grab) {
    if (myWindow != 0) {
        myWindow->grabAllKeys(grab);
    }
}

void ZLApplication::setHyperlinkCursor(bool hyperlink) {
    if (myWindow != 0) {
        myWindow->setHyperlinkCursor(hyperlink);
    }
}

bool ZLApplication::isFullscreen() const {
    return (myWindow != 0) && myWindow->isFullscreen();
}

void ZLApplication::setFullscreen(bool fullscreen) {
    if (myWindow != 0) {
        myWindow->setFullscreen(fullscreen);
    }
}

void ZLApplication::quit() {
    if (myWindow != 0) {
        myWindow->close();
    }
}

ZLApplication::PresentWindowHandler::PresentWindowHandler(ZLApplication &application) : myApplication(application) {
}

void ZLApplication::PresentWindowHandler::onMessageReceived(const std::vector<std::string> &arguments) {
    myApplication.presentWindow();
    if (arguments.size() == 1) {
        myLastCaller = arguments[0];
    }
}

const std::string &ZLApplication::PresentWindowHandler::lastCaller() const {
    return myLastCaller;
}

void ZLApplication::PresentWindowHandler::resetLastCaller() {
    myLastCaller.erase();
}

const std::string &ZLApplication::lastCaller() const {
    return ((PresentWindowHandler&)*myPresentWindowHandler).lastCaller();
}

void ZLApplication::resetLastCaller() {
    ((PresentWindowHandler&)*myPresentWindowHandler).resetLastCaller();
}

shared_ptr<ZLPaintContext> ZLApplication::context() {
    return myContext;
}

ZLApplicationWindow::ToolbarType ZLApplicationWindow::type(const ZLToolbar::Item &item) const {
    return
            (&item.toolbar() == &application().toolbar(WINDOW_TOOLBAR)) ?
            WINDOW_TOOLBAR : FULLSCREEN_TOOLBAR;
}

bool ZLApplicationWindow::hasFullscreenToolbar() const {
    return !application().toolbar(FULLSCREEN_TOOLBAR).items().empty();
}

void ZLApplicationWindow::onButtonPress(const ZLToolbar::AbstractButtonItem &button) {
    if (myToggleButtonLock) {
        return;
    }
    if (button.type() == ZLToolbar::Item::TOGGLE_BUTTON) {
        ZLToolbar::ToggleButtonItem &toggleButton = (ZLToolbar::ToggleButtonItem&)button;
        myToggleButtonLock = true;
        if (toggleButton.isPressed()) {
            setToggleButtonState(toggleButton);
            myToggleButtonLock = false;
            return;
        } else {
            toggleButton.press();
            const ZLToolbar::ButtonGroup::ItemSet &items = toggleButton.buttonGroup().myItems;
            for (ZLToolbar::ButtonGroup::ItemSet::const_iterator it = items.begin(); it != items.end(); ++it) {
                setToggleButtonState(**it);
            }
        }
        myToggleButtonLock = false;
    }
    application().doAction(button.actionId());
}

bool ZLApplication::isViewFinal() const {
    return true;
}

void ZLApplication::setVisualParameter(const std::string &id, const std::string &value) {
    if (myWindow != 0) {
        myWindow->setVisualParameter(id, value);
    }
}

void ZLApplication::setParameterValueList(const std::string &id, const std::vector<std::string> &values) {
    if (myWindow != 0) {
        myWindow->setParameterValueList(id, values);
    }
}

const std::string &ZLApplication::visualParameter(const std::string &id) {
    if (myWindow != 0) {
        return myWindow->visualParameter(id);
    }
    static const std::string EMPTY;
    return EMPTY;
}

void ZLApplicationWindow::setVisualParameter(const std::string &id, const std::string &value) {
    std::map<std::string,shared_ptr<VisualParameter> >::iterator it = myParameterMap.find(id);
    if (it != myParameterMap.end()) {
        it->second->setValue(value);
    }
}

void ZLApplicationWindow::setParameterValueList(const std::string &id, const std::vector<std::string> &values) {
    std::map<std::string,shared_ptr<VisualParameter> >::iterator it = myParameterMap.find(id);
    if (it != myParameterMap.end()) {
        it->second->setValueList(values);
    }
}

const std::string &ZLApplicationWindow::visualParameter(const std::string &id) {
    std::map<std::string,shared_ptr<VisualParameter> >::const_iterator it = myParameterMap.find(id);
    if (it != myParameterMap.end()) {
        return it->second->value();
    }
    static const std::string EMPTY;
    return EMPTY;
}

void ZLApplicationWindow::addVisualParameter(const std::string &id, shared_ptr<VisualParameter> parameter) {
    if (parameter) {
        myParameterMap[id] = parameter;
    }
}

ZLApplicationWindow::VisualParameter::~VisualParameter() {
}

const std::string &ZLApplicationWindow::VisualParameter::value() const {
    myValue = internalValue();
    return myValue;
}

void ZLApplicationWindow::VisualParameter::setValue(const std::string &value) {
    if (value != myValue) {
        myValue = value;
        internalSetValue(value);
    }
}

void ZLApplicationWindow::VisualParameter::restoreOldValue() {
    internalSetValue(myValue);
}

ZLView::Angle ZLApplication::rotation() const {
    return (myViewWidget != 0) ? myViewWidget->rotation() : ZLView::DEGREES0;
}

shared_ptr<ZLKeyBindings> ZLApplication::keyBindings() {
    return shared_ptr<ZLKeyBindings>();
}
