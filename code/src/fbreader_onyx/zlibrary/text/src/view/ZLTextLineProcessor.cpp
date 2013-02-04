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

#include <algorithm>

#include <ZLUnicodeUtil.h>

#include <ZLTextHyphenator.h>

#include "ZLTextView.h"
#include "ZLTextLineInfo.h"
#include "ZLTextParagraphCursor.h"
#include "ZLTextStyle.h"
#include "ZLTextWord.h"

struct ZLTextPartialInfo {
    //ZLTextWordCursor Start;
    //ZLTextWordCursor RealStart;
    ZLTextWordCursor End;
    bool IsVisible;
    //int LeftIndent;
    int Width;
    int Height;
    int Descent;
    //int VSpaceAfter;
    int SpaceCounter;
    //ZLTextStylePtr StartStyle;
    //shared_ptr<ZLTextTreeNodeInfo> NodeInfo;

    ZLTextPartialInfo(const ZLTextLineInfo &lineInfo, const ZLTextWordCursor &end);
    void setTo(ZLTextLineInfo &lineInfo) const;
};

ZLTextPartialInfo::ZLTextPartialInfo(const ZLTextLineInfo &lineInfo, const ZLTextWordCursor &end) : End(end), IsVisible(false), Width(lineInfo.Width), Height(lineInfo.Height), Descent(lineInfo.Descent), SpaceCounter(0) {
}

void ZLTextPartialInfo::setTo(ZLTextLineInfo &lineInfo) const {
    lineInfo.End = End;
    lineInfo.IsVisible = IsVisible;
    lineInfo.Width = Width;
    lineInfo.Height = std::max(lineInfo.Height, Height);
    lineInfo.Descent = std::max(lineInfo.Descent, Descent);
    lineInfo.SpaceCounter = SpaceCounter;
}

ZLTextLineInfoPtr ZLTextView::processTextLine(const ZLTextWordCursor &start, const ZLTextWordCursor &end) {
    const bool useHyphenator =
        ZLTextStyleCollection::instance().baseStyle().AutoHyphenationOption.value();

    ZLTextLineInfoPtr infoPtr = new ZLTextLineInfo(start, myStyle.textStyle(), myStyle.bidiLevel());

    std::set<ZLTextLineInfoPtr>::const_iterator it = myLineInfoCache.find(infoPtr);
    if (it != myLineInfoCache.end()) {
        const ZLTextLineInfoPtr &storedInfo = *it;
        myStyle.applyControls(storedInfo->Start, storedInfo->End);
        return storedInfo;
    }

    ZLTextLineInfo &info = *infoPtr;
    ZLTextWordCursor current = start;
    const ZLTextParagraphCursor &paragraphCursor = current.paragraphCursor();
    const bool isFirstLine = current.isStartOfParagraph();

    if (paragraphCursor.paragraph().kind() == ZLTextParagraph::TREE_PARAGRAPH) {
          info.NodeInfo.reset(new ZLTextTreeNodeInfo());
        ZLTextTreeNodeInfo &nodeInfo = *info.NodeInfo;
        const ZLTextTreeParagraph &treeParagraph = (const ZLTextTreeParagraph&)paragraphCursor.paragraph();
        nodeInfo.IsLeaf = treeParagraph.children().empty();
        nodeInfo.IsOpen = treeParagraph.isOpen();
        nodeInfo.IsFirstLine = isFirstLine;
        nodeInfo.ParagraphIndex = paragraphCursor.index();

        nodeInfo.VerticalLinesStack.reserve(treeParagraph.depth() - 1);
        if (treeParagraph.depth() > 1) {
            const ZLTextTreeParagraph *ctp = treeParagraph.parent();
            nodeInfo.VerticalLinesStack.push_back(ctp->children().back() != &treeParagraph);
            for (int i = 1; i < treeParagraph.depth() - 1; ++i) {
                const ZLTextTreeParagraph *parent = ctp->parent();
                nodeInfo.VerticalLinesStack.push_back(ctp != parent->children().back());
                ctp = parent;
            }
        }
    }

    if (isFirstLine) {
        ZLTextElement::Kind elementKind = paragraphCursor[current.elementIndex()].kind();
        while ((elementKind == ZLTextElement::CONTROL_ELEMENT) ||
                     (elementKind == ZLTextElement::FORCED_CONTROL_ELEMENT)) {
            myStyle.applySingleControl(paragraphCursor[current.elementIndex()]);
            current.nextWord();
            if (current.equalElementIndex(end)) {
                break;
            }
            elementKind = paragraphCursor[current.elementIndex()].kind();
        }
        info.StartStyle = myStyle.textStyle();
        info.StartBidiLevel = myStyle.bidiLevel();
        info.RealStart = current;
    }

    ZLTextStylePtr storedStyle = myStyle.textStyle();
    unsigned char storedBidiLevel = myStyle.bidiLevel();

    const int fontSize = myStyle.textStyle()->fontSize();
    // TODO: change metrics at font change
    const ZLTextStyleEntry::Metrics metrics(fontSize, fontSize / 2, viewWidth(), textAreaHeight());
    info.LeftIndent = myStyle.textStyle()->leftIndent(metrics);
    if (isFirstLine) {
        info.LeftIndent += myStyle.textStyle()->firstLineIndentDelta(metrics);

        // test if chinese. TODO: Need a better way to check character width
        bool is_cn = (paragraphCursor.getLanguage() == "zh" );
        double  divisor = 2;
        if (is_cn)//four char
        {
            divisor = 1;
        }
        else // two char
        {
            divisor = 1.5;
        }

        info.LeftIndent += (fontSize * metrics.FullWidth + 50) / 100;
        if (metrics.FullWidth > metrics.FullHeight)
        {
            info.LeftIndent = (int)(info.LeftIndent * 1.6 / 7 / divisor);
        }
        else
        {
            //                 {"hide margin", 0},
            //                 {"small", 8},
            //                 {"medium", 30},
            //                 {"big", 60}
            float value;
            switch (leftMargin()) {
            case 0:
                value = 1.6;
                break;
            case 8:
                value = 1.7;
                break;
            case 30:
                value = 1.8;
                break;
            case 60:
                value = 2.1;
                break;
            default:
                value = 1.7;
                break;
            }
            info.LeftIndent = (int)(info.LeftIndent * value / 7 /divisor);
        }

    }

    if (info.NodeInfo) {
        info.LeftIndent += (myStyle.context().stringHeight() + 2) / 3 * 4 * (info.NodeInfo->VerticalLinesStack.size() + 1);
    }
    info.Width = info.LeftIndent;

    if (info.RealStart.equalElementIndex(end)) {
      info.End = info.RealStart;
        return infoPtr;
    }

    ZLTextPartialInfo newInfo(info, current);
    bool allowBreakAtNBSpace = true;
    const int maxWidth = metrics.FullWidth - myStyle.textStyle()->rightIndent(metrics);
    bool wordOccured = false;
    int lastSpaceWidth = 0;
    int removeLastSpace = false;

    ZLTextElement::Kind elementKind = paragraphCursor[newInfo.End.elementIndex()].kind();

    bool breakedAtFirstWord = false;
    do {
        const ZLTextElement &element = paragraphCursor[newInfo.End.elementIndex()];
        newInfo.Width += myStyle.elementWidth(element, newInfo.End.charIndex(), metrics);
        newInfo.Height = std::max(newInfo.Height, myStyle.elementHeight(element, metrics));
        newInfo.Descent = std::max(newInfo.Descent, myStyle.elementDescent(element));
        myStyle.applySingleControl(element);
        switch (elementKind) {
            case ZLTextElement::WORD_ELEMENT:
            case ZLTextElement::IMAGE_ELEMENT:
                wordOccured = true;
                newInfo.IsVisible = true;
                break;
            case ZLTextElement::HSPACE_ELEMENT:
            case ZLTextElement::NB_HSPACE_ELEMENT:
                if (wordOccured) {
                    wordOccured = false;
                    ++newInfo.SpaceCounter;
                    lastSpaceWidth = myStyle.context().spaceWidth();
                    newInfo.Width += lastSpaceWidth;
                }
                break;
            case ZLTextElement::EMPTY_LINE_ELEMENT:
                newInfo.IsVisible = true;
                break;
            default:
                break;
        }

        if (newInfo.Width > maxWidth) {
            if (!info.End.equalElementIndex(start)) {
                break;
            }
            if (useHyphenator && myStyle.textStyle()->allowHyphenations() &&
                    (elementKind == ZLTextElement::WORD_ELEMENT)) {
                breakedAtFirstWord = true;
                break;
            }
        }

        ZLTextElement::Kind previousKind = elementKind;
        newInfo.End.nextWord();
        bool allowBreak = newInfo.End.equalElementIndex(end);
        bool nbspaceBreak = false;
        if (!allowBreak) {
            elementKind = paragraphCursor[newInfo.End.elementIndex()].kind();
            if (elementKind == ZLTextElement::NB_HSPACE_ELEMENT) {
                if (allowBreakAtNBSpace) {
                    allowBreak = true;
                    nbspaceBreak = true;
                }
            } else if (elementKind == ZLTextElement::WORD_ELEMENT) {
                allowBreak = previousKind == ZLTextElement::WORD_ELEMENT;
            } else if ((elementKind == ZLTextElement::START_REVERSED_SEQUENCE_ELEMENT) &&
                                 (previousKind == ZLTextElement::WORD_ELEMENT)) {
                allowBreak = false;
            } else if ((elementKind == ZLTextElement::END_REVERSED_SEQUENCE_ELEMENT) &&
                                 (previousKind == ZLTextElement::WORD_ELEMENT)) {
                allowBreak = false;
            } else {
                allowBreak =
                    (elementKind != ZLTextElement::IMAGE_ELEMENT) &&
                    (elementKind != ZLTextElement::CONTROL_ELEMENT);
            }
        }
        if (allowBreak) {
            newInfo.setTo(info);
            allowBreakAtNBSpace = nbspaceBreak;
            storedStyle = myStyle.textStyle();
            storedBidiLevel = myStyle.bidiLevel();
            removeLastSpace = !wordOccured && (info.SpaceCounter > 0);
        }
    } while (!newInfo.End.equalElementIndex(end));

    if (!newInfo.End.equalElementIndex(end) && useHyphenator &&
         myStyle.textStyle()->allowHyphenations()) {
        const ZLTextElement &element = paragraphCursor[newInfo.End.elementIndex()];
        if (element.kind() == ZLTextElement::WORD_ELEMENT) {
            const int startCharIndex = newInfo.End.charIndex();
            newInfo.Width -= myStyle.elementWidth(element, startCharIndex, metrics);
            const ZLTextWord &word = (ZLTextWord&)element;
            int spaceLeft = maxWidth - newInfo.Width;
            if (breakedAtFirstWord ||
                    ((word.Length > 3) && (spaceLeft > 2 * myStyle.context().spaceWidth()))) {
                ZLUnicodeUtil::Ucs4String ucs4string;
                ZLUnicodeUtil::utf8ToUcs4(ucs4string, word.Data, word.Size);
                ZLTextHyphenationInfo hyphenationInfo = ZLTextHyphenator::instance().info(word);
                int hyphenationPosition = word.Length - 1;
                int subwordWidth = 0;
                for (; hyphenationPosition > startCharIndex; --hyphenationPosition) {
                    if (hyphenationInfo.isHyphenationPossible(hyphenationPosition) || enable_hyphenation_) {
                        subwordWidth = myStyle.wordWidth(word, startCharIndex, hyphenationPosition - startCharIndex, ucs4string[hyphenationPosition - 1] != '-');
                        if (subwordWidth <= spaceLeft) {
                            break;
                        }
                    }
                }
                if ((hyphenationPosition == startCharIndex) &&
                        (info.End.elementIndex() <= info.RealStart.elementIndex())) {
                    hyphenationPosition = word.Length;
                    subwordWidth = myStyle.elementWidth(element, startCharIndex, metrics);
                }
                if (hyphenationPosition > startCharIndex) {
                    newInfo.Width += subwordWidth;
                    newInfo.setTo(info);
                    storedStyle = myStyle.textStyle();
                    storedBidiLevel = myStyle.bidiLevel();
                    removeLastSpace = false;
                    info.End.setCharIndex(hyphenationPosition);
                }
            }
        }
    }

    if (removeLastSpace) {
        info.Width -= lastSpaceWidth;
        --info.SpaceCounter;
    }

    myStyle.setTextStyle(storedStyle, storedBidiLevel);

    if (isFirstLine) {
        info.Height += info.StartStyle->spaceBefore(metrics);
    }
    if (info.End.isEndOfParagraph()) {
        info.VSpaceAfter = myStyle.textStyle()->spaceAfter(metrics);
    }

    if (!info.End.equalElementIndex(end) || end.isEndOfParagraph()) {
        myLineInfoCache.insert(infoPtr);
    }

    return infoPtr;
}
