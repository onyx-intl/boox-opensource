#include "cr3widget.h"
#include "search_tool.h"

SearchTool::SearchTool(QObject *parent, CR3View * docView) :
    QObject(parent),
    _docview( docView )
{
}

SearchTool::~SearchTool()
{
}

bool SearchTool::findText( lString16 pattern, int origin, bool reverse, bool caseInsensitive )
{
    if ( pattern.empty() )
        return false;
    if ( pattern!=_lastPattern && origin==1 )
        origin = 0;
    _lastPattern = pattern;
    LVArray<ldomWord> words;
    lvRect rc;
    _docview->getDocView()->GetPos( rc );
    int pageHeight = rc.height();
    int start = -1;
    int end = -1;
    if ( reverse ) {
        // reverse
        if ( origin == 0 ) {
            // from end current page to first page
            end = rc.bottom;
        } else if ( origin == -1 ) {
            // from last page to end of current page
            start = rc.bottom;
        } else { // origin == 1
            // from prev page to first page
            end = rc.top;
        }
    } else {
        // forward
        if ( origin == 0 ) {
            // from current page to last page
            start = rc.top;
        } else if ( origin == -1 ) {
            // from first page to current page
            end = rc.top;
        } else { // origin == 1
            // from next page to last
            start = rc.bottom;
        }
    }
    CRLog::debug("CRViewDialog::findText: Current page: %d .. %d", rc.top, rc.bottom);
    CRLog::debug("CRViewDialog::findText: searching for text '%s' from %d to %d origin %d", LCSTR(pattern), start, end, origin );
    if ( _docview->getDocView()->getDocument()->findText( pattern, caseInsensitive, reverse, start, end, words, 200, pageHeight ) ) {
        CRLog::debug("CRViewDialog::findText: pattern found");
        _docview->getDocView()->clearSelection();
        _docview->getDocView()->selectWords( words );
        ldomMarkedRangeList * ranges = _docview->getDocView()->getMarkedRanges();
        if ( ranges ) {
            if ( ranges->length()>0 ) {
                int pos = ranges->get(0)->start.y;
                _docview->getDocView()->SetPos(pos);
            }
        }
        return true;
    }
    CRLog::debug("CRViewDialog::findText: pattern not found");
    return false;
}

bool SearchTool::FindNext()
{
    bool found = false;

    found = findText(_lastPattern, 1, _forwardOption , true);
    if ( !found )
        found = findText(_lastPattern, -1, _forwardOption, true);
    if ( !found ) {
        return false;
    } else {
        _docview->update();
        return true;
    }
}

void SearchTool::onCloseSearch()
{
    _docview->getDocView()->clearSelection();
}

void SearchTool::setSearchPattern(const QString & pattern)
{
    _lastPattern = qt2cr(pattern);
}
void SearchTool::setReverse(bool value)
{
    _forwardOption = value;
}
