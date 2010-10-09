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

#include <iostream>
#include <queue>

#include <ZLibrary.h>
#include <ZLFile.h>
#include <ZLDialogManager.h>
#include <ZLOptionsDialog.h>
#include <ZLDir.h>
#include <ZLStringUtil.h>
#include <ZLResource.h>
#include <ZLMessage.h>

#include <ZLTextHyphenator.h>

#include "FBReader.h"
#include "FBReaderActions.h"
#include "BookTextView.h"
#include "FootnoteView.h"
#include "ContentsView.h"

#include "../options/FBOptions.h"
#include "../bookmodel/BookModel.h"
#include "../formats/FormatPlugin.h"

static const std::string OPTIONS = "Options";
static const std::string STATE = "State";
static const std::string BOOK = "Book";

const std::string LARGE_SCROLLING = "LargeScrolling";
const std::string SMALL_SCROLLING = "SmallScrolling";
const std::string MOUSE_SCROLLING = "MouseScrolling";
const std::string TAP_SCROLLING = "TapScrolling";

const std::string DELAY = "ScrollingDelay";
const std::string MODE = "Mode";
const std::string LINES_TO_KEEP = "LinesToKeep";
const std::string LINES_TO_SCROLL = "LinesToScroll";
const std::string PERCENT_TO_SCROLL = "PercentToScroll";

const std::string FBReader::PageIndexParameter = "pageIndex";

using namespace cms;
using namespace vbf;

FBReader::ScrollingOptions::ScrollingOptions(const std::string &groupName,
                                             long delayValue,
                                             long modeValue,
                                             long linesToKeepValue,
                                             long linesToScrollValue,
                                             long percentToScrollValue)
        : DelayOption(ZLCategoryKey::CONFIG, groupName, DELAY, 0, 5000,
                      delayValue),
          ModeOption(ZLCategoryKey::CONFIG, groupName, MODE, modeValue),
          LinesToKeepOption(ZLCategoryKey::CONFIG, groupName, LINES_TO_KEEP, 1,
                          100, linesToKeepValue),
          LinesToScrollOption(ZLCategoryKey::CONFIG, groupName, LINES_TO_SCROLL,
                              1, 100, linesToScrollValue),
          PercentToScrollOption(ZLCategoryKey::CONFIG, groupName,
                                PERCENT_TO_SCROLL, 1, 100,
                                percentToScrollValue) {
}

FBReader::FBReader(const std::string &bookToOpen)
        : ZLApplication("onyx_reader"),
          QuitOnCancelOption(ZLCategoryKey::CONFIG, OPTIONS, "QuitOnCancel",
                             false),
          LargeScrollingOptions(LARGE_SCROLLING, 250,
                                ZLTextView::NO_OVERLAPPING, 1, 1, 50),
          SmallScrollingOptions(SMALL_SCROLLING, 50,
                                ZLTextView::SCROLL_LINES, 1, 1, 50),
          MouseScrollingOptions(MOUSE_SCROLLING, 0,
                                ZLTextView::SCROLL_LINES, 1, 1, 50),
          TapScrollingOptions(TAP_SCROLLING, 0,
                              ZLTextView::NO_OVERLAPPING, 1, 1, 50),
          EnableTapScrollingOption(ZLCategoryKey::CONFIG, TAP_SCROLLING,
                                   "Enabled", false),
          TapScrollingOnFingerOnlyOption(ZLCategoryKey::CONFIG, TAP_SCROLLING,
                                         "FingerOnly", false),
          UseSeparateBindingsOption(ZLCategoryKey::CONFIG, "KeysOptions",
                                    "UseSeparateBindings", false),
          EnableSingleClickDictionaryOption(ZLCategoryKey::CONFIG, "Dictionary",
                                            "SingleClick", false),
          myBindings0(new ZLKeyBindings("Keys")),
          myBindings90(new ZLKeyBindings("Keys90")),
          myBindings180(new ZLKeyBindings("Keys180")),
          myBindings270(new ZLKeyBindings("Keys270")),
          myBookToOpen(bookToOpen),
          myBookAlreadyOpen(false),
          myActionOnCancel(UNFULLSCREEN) {
    myModel = 0;
    myBookTextView.reset(new BookTextView(*this, context()));
    myFootnoteView.reset(new FootnoteView(*this, context()));
    myContentsView.reset(new ContentsView(*this, context()));
    myMode = UNDEFINED_MODE;
    myPreviousMode = BOOK_TEXT_MODE;
    setMode(BOOK_TEXT_MODE);

    addAction(ActionCode::SHOW_READING,
              shared_ptr<ZLApplication::Action>(new UndoAction(*this, FBReader::ALL_MODES & ~FBReader::BOOK_TEXT_MODE)));

    // addAction(ActionCode::SHOW_OPTIONS, shared_ptr<ZLApplication::Action>(new ShowOptionsDialogAction(*this)));
    addAction(ActionCode::SHOW_CONTENTS, shared_ptr<ZLApplication::Action>(new ShowContentsAction(*this)));
    addAction(ActionCode::UNDO, shared_ptr<ZLApplication::Action>(new UndoAction(*this, FBReader::BOOK_TEXT_MODE)));
    addAction(ActionCode::REDO, shared_ptr<ZLApplication::Action>(new RedoAction(*this)));
    addAction(ActionCode::SEARCH, shared_ptr<ZLApplication::Action>(new SearchPatternAction(*this)));
    addAction(ActionCode::FIND_NEXT, shared_ptr<ZLApplication::Action>(new FindNextAction(*this)));
    addAction(ActionCode::FIND_PREVIOUS, shared_ptr<ZLApplication::Action>(new FindPreviousAction(*this)));
    addAction(ActionCode::SCROLL_TO_HOME, shared_ptr<ZLApplication::Action>(new ScrollToHomeAction(*this)));
    addAction(ActionCode::SCROLL_TO_START_OF_TEXT, shared_ptr<ZLApplication::Action>(new ScrollToStartOfTextAction(*this)));
    addAction(ActionCode::SCROLL_TO_END_OF_TEXT, shared_ptr<ZLApplication::Action>(new ScrollToEndOfTextAction(*this)));
    addAction(ActionCode::LARGE_SCROLL_FORWARD,
              shared_ptr<ZLApplication::Action>(new ScrollingAction(*this, LargeScrollingOptions, true)));
    addAction(ActionCode::LARGE_SCROLL_BACKWARD,
              shared_ptr<ZLApplication::Action>(new ScrollingAction(*this, LargeScrollingOptions, false)));
    addAction(ActionCode::SMALL_SCROLL_FORWARD,
              shared_ptr<ZLApplication::Action>(new ScrollingAction(*this, SmallScrollingOptions, true)));
    addAction(ActionCode::SMALL_SCROLL_BACKWARD,
              shared_ptr<ZLApplication::Action>(new ScrollingAction(*this, SmallScrollingOptions, false)));
    addAction(ActionCode::MOUSE_SCROLL_FORWARD,
              shared_ptr<ZLApplication::Action>(new ScrollingAction(*this, MouseScrollingOptions, true)));
    addAction(ActionCode::MOUSE_SCROLL_BACKWARD,
              shared_ptr<ZLApplication::Action>(new ScrollingAction(*this, MouseScrollingOptions, false)));
    addAction(ActionCode::TAP_SCROLL_FORWARD,
              shared_ptr<ZLApplication::Action>(new ScrollingAction(*this, TapScrollingOptions, true)));
    addAction(ActionCode::TAP_SCROLL_BACKWARD,
              shared_ptr<ZLApplication::Action>(new ScrollingAction(*this, TapScrollingOptions, false)));
    addAction(ActionCode::INCREASE_FONT, shared_ptr<ZLApplication::Action>(new ChangeFontSizeAction(*this, 2)));
    addAction(ActionCode::DECREASE_FONT, shared_ptr<ZLApplication::Action>(new ChangeFontSizeAction(*this, -2)));

    addAction(ActionCode::UPDATE_OPTIONS, shared_ptr<ZLApplication::Action>(new UpdateOptionsAction(*this)));
    addAction("clearSearchResult", shared_ptr<ZLApplication::Action>(new ClearSearchResultAction(*this)));
    addAction("changeEncoding", shared_ptr<ZLApplication::Action>(new ChangeEncodingAction(*this)));

    // addAction(ActionCode::ROTATE_SCREEN, shared_ptr<ZLApplication::Action>(new RotationAction(*this)));
    // addAction(ActionCode::TOGGLE_FULLSCREEN, shared_ptr<ZLApplication::Action>(new FBFullscreenAction(*this)));
    addAction(ActionCode::FULLSCREEN_ON, shared_ptr<ZLApplication::Action>(new FBFullscreenAction(*this)));
    addAction(ActionCode::CANCEL, shared_ptr<ZLApplication::Action>(new CancelAction(*this)));
    addAction(ActionCode::SHOW_HIDE_POSITION_INDICATOR,
              shared_ptr<ZLApplication::Action>(new ToggleIndicatorAction(*this)));
    addAction(ActionCode::QUIT, shared_ptr<ZLApplication::Action>(new QuitAction(*this)));
    // addAction(ActionCode::OPEN_PREVIOUS_BOOK, shared_ptr<ZLApplication::Action>(new OpenPreviousBookAction(*this)));
    // addAction(ActionCode::SHOW_HELP, shared_ptr<ZLApplication::Action>(new ShowHelpAction(*this)));
    addAction(ActionCode::GOTO_NEXT_TOC_SECTION,
              shared_ptr<ZLApplication::Action>(new GotoNextTOCSectionAction(*this)));
    addAction(ActionCode::GOTO_PREVIOUS_TOC_SECTION,
              shared_ptr<ZLApplication::Action>(new GotoPreviousTOCSectionAction(*this)));
    addAction(ActionCode::COPY_SELECTED_TEXT_TO_CLIPBOARD,
              shared_ptr<ZLApplication::Action>(new CopySelectedTextAction(*this)));
    addAction(ActionCode::OPEN_SELECTED_TEXT_IN_DICTIONARY,
              shared_ptr<ZLApplication::Action>(new OpenSelectedTextInDictionaryAction(*this)));
    addAction(ActionCode::CLEAR_SELECTION, shared_ptr<ZLApplication::Action>(new ClearSelectionAction(*this)));
    addAction(ActionCode::GOTO_PAGE_NUMBER,
              shared_ptr<ZLApplication::Action>(new GotoPageNumber(*this, std::string())));
    addAction(ActionCode::GOTO_PAGE_NUMBER_WITH_PARAMETER,
              shared_ptr<ZLApplication::Action>(new GotoPageNumber(*this, PageIndexParameter)));

    myOpenFileHandler.reset(new OpenFileHandler(*this));
    ZLCommunicationManager::instance().registerHandler("openFile",
                                                       myOpenFileHandler);
}

FBReader::~FBReader() {
    if (myModel != 0) {
        delete myModel;
    }
    ZLTextStyleCollection::deleteInstance();
    PluginCollection::deleteInstance();
    ZLTextHyphenator::deleteInstance();
}

bool FBReader::initWindow() {
    ZLApplication::initWindow();

    // John: Don't track stylus
    // trackStylus(true);

    if (!myBookAlreadyOpen) {
        BookDescriptionPtr description;
        if (!myBookToOpen.empty()) {
            if (!createDescription(myBookToOpen, description))
            {
                return false;
            }
        }

        // Load configuration if necessary, have been loaded.
        WritableBookDescription desc(*description);
        if (!conf().options[CONFIG_ENCODING].toString().isEmpty())
        {
            desc.encoding() = conf().options[CONFIG_ENCODING].toString().toStdString();
        }
        ZLApplication::EncodingOption.setValue(desc.encoding());
        openBook(description);
    }
    ZLApplication::content_ready = true;
    refreshWindow();
    return true;
}

bool FBReader::createDescription(const std::string& fileName, BookDescriptionPtr &description) {
    ZLFile bookFile = ZLFile(fileName);

    FormatPlugin *plugin = PluginCollection::instance().plugin(ZLFile(fileName),
                                                               false);
    if (plugin != 0) {
        std::string error = plugin->tryOpen(fileName);
        if (!error.empty()) {
            return false;
            ZLResourceKey boxKey("openBookErrorBox");
            ZLDialogManager::instance().errorBox(
                    boxKey,
                    ZLStringUtil::printf(ZLDialogManager::dialogMessage(boxKey),
                                         error));
        } else {
            description = BookDescription::getDescription(bookFile.path());
        }
        return true;
    }

    if (!bookFile.isArchive()) {
        return false;
    }

    std::queue<std::string> archiveNames;
    archiveNames.push(bookFile.path());

    std::vector<std::string> items;

    while (!archiveNames.empty()) {
        shared_ptr<ZLDir> archiveDir = ZLFile(archiveNames.front()).directory();
        archiveNames.pop();
        if (!archiveDir) {
            continue;
        }
        archiveDir->collectFiles(items, true);
        for (std::vector<std::string>::const_iterator it = items.begin(); it != items.end(); ++it) {
            const std::string itemName = archiveDir->itemPath(*it);
            ZLFile subFile(itemName);
            if (subFile.isArchive()) {
                archiveNames.push(itemName);
            } else if (createDescription(itemName, description)) {
                return true;
            }
        }
        items.clear();
    }

    return 0;
}

class OpenBookRunnable : public ZLRunnable {

  public:
    OpenBookRunnable(FBReader &reader, BookDescriptionPtr description)
            : myReader(reader), myDescription(description) {}
    void run() { myReader.openBookInternal(myDescription); }

  private:
    FBReader &myReader;
    BookDescriptionPtr myDescription;
};

void FBReader::openBook(BookDescriptionPtr description) {
    OpenBookRunnable runnable(*this, description);
    ZLDialogManager::instance().wait(ZLResourceKey("loadingBook"), runnable);
}

void FBReader::openBookInternal(BookDescriptionPtr description) {
    if (description) {
        BookTextView &bookTextView = (BookTextView&)*myBookTextView;
        ContentsView &contentsView = (ContentsView&)*myContentsView;
        FootnoteView &footnoteView = (FootnoteView&)*myFootnoteView;

        bookTextView.saveState();
        bookTextView.setModel(shared_ptr<ZLTextModel>(), "", "");
        bookTextView.setContentsModel(shared_ptr<ZLTextModel>());
        contentsView.setModel(shared_ptr<ZLTextModel>(), "");
        if (myModel != 0) {
            delete myModel;
        }
        myModel = new BookModel(description);
        ZLStringOption(ZLCategoryKey::STATE, STATE, BOOK, std::string()).setValue(myModel->fileName());
        const std::string &lang = description->language();
        ZLTextHyphenator::instance().load(lang);
        bookTextView.setModel(myModel->bookTextModel(), lang, description->fileName());
        bookTextView.setCaption(description->title());
        bookTextView.setContentsModel(myModel->contentsModel());
        footnoteView.setModel(shared_ptr<ZLTextModel>(), lang);
        footnoteView.setCaption(description->title());
        contentsView.setModel(myModel->contentsModel(), lang);
        contentsView.setCaption(description->title());
    }
}

void FBReader::tryShowFootnoteView(const std::string &id, const std::string &type) {
    if (type == "external") {
    } else if (type == "internal") {
        if ((myMode == BOOK_TEXT_MODE) && (myModel != 0)) {
            BookModel::Label label = myModel->label(id);
            if (label.Model) {
                if (label.Model == myModel->bookTextModel()) {
                    bookTextView().gotoParagraph(label.ParagraphNumber);
                } else {
                    FootnoteView &view = ((FootnoteView&)*myFootnoteView);
                    view.setModel(label.Model, myModel->description()->language());
                    setMode(FOOTNOTE_MODE);
                    view.gotoParagraph(label.ParagraphNumber);
                }
                setHyperlinkCursor(false);
                refreshWindow();
            }
        }
    } else if (type == "book") {
    }
}

FBReader::ViewMode FBReader::mode() const {
    return myMode;
}

bool FBReader::isViewFinal() const {
    return myMode == BOOK_TEXT_MODE;
}

void FBReader::setMode(ViewMode mode) {
    if (mode == myMode) {
        return;
    }

    if (mode != BOOK_TEXT_MODE) {
        myActionOnCancel = RETURN_TO_TEXT_MODE;
    }

    myPreviousMode = myMode;
    myMode = mode;

    switch (myMode) {
        case BOOK_TEXT_MODE:
            setHyperlinkCursor(false);
            ((ZLTextView&)*myBookTextView).forceScrollbarUpdate();
            setView(myBookTextView);
            break;
        case CONTENTS_MODE:
            ((ContentsView&)*myContentsView).gotoReference();
            setView(myContentsView);
            break;
        case FOOTNOTE_MODE:
            setView(myFootnoteView);
            break;
        case BOOKMARKS_MODE:
            break;
        case UNDEFINED_MODE:
        case ALL_MODES:
            break;
    }
}

BookTextView &FBReader::bookTextView() const {
    return (BookTextView&)*myBookTextView;
}

void FBReader::showBookTextView() {
    setMode(BOOK_TEXT_MODE);
}

void FBReader::restorePreviousMode() {
    setMode(myPreviousMode);
    myPreviousMode = BOOK_TEXT_MODE;
}

bool FBReader::closeView() {
    if (myMode == BOOK_TEXT_MODE) {
        quit();
        return true;
    } else {
        restorePreviousMode();
        return false;
    }
}

std::string FBReader::helpFileName(const std::string &language) const {
    return ZLibrary::ApplicationDirectory() + ZLibrary::FileNameDelimiter + "help" + ZLibrary::FileNameDelimiter + "MiniHelp." + language + ".fb2";
}

void FBReader::openFile(const std::string &fileName) {
    BookDescriptionPtr description;
    createDescription(fileName, description);
    if (description) {
        openBook(description);
        refreshWindow();
    }
}

void FBReader::clearTextCaches() {
    ((ZLTextView&)*myBookTextView).clearCaches();
    ((ZLTextView&)*myFootnoteView).clearCaches();
    ((ZLTextView&)*myContentsView).clearCaches();
}

shared_ptr<ZLKeyBindings> FBReader::keyBindings() {
    return UseSeparateBindingsOption.value() ?
            keyBindings(rotation()) : myBindings0;
}

shared_ptr<ZLKeyBindings> FBReader::keyBindings(ZLView::Angle angle) {
    switch (angle) {
        case ZLView::DEGREES0:
            return myBindings0;
        case ZLView::DEGREES90:
            return myBindings90;
        case ZLView::DEGREES180:
            return myBindings180;
        case ZLView::DEGREES270:
            return myBindings270;
    }
    return shared_ptr<ZLKeyBindings>();
}

bool FBReader::isDictionarySupported() const {
    return false;
}

void FBReader::openInDictionary(const std::string &word) {
}
