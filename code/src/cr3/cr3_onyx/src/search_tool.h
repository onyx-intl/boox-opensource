#ifndef SEARCHDLG_H
#define SEARCHDLG_H

#include <lvstring.h>
#include <QObject>

class CR3View;

class SearchTool : public QObject {
    Q_OBJECT
public:
    SearchTool(QObject *parent, CR3View * docView);
    ~SearchTool();

    void setSearchPattern(const QString & pattern);
    void setReverse(bool value);

public slots:
    bool FindNext();
    void onCloseSearch();
    bool findText( lString16 pattern, int origin, bool reverse, bool caseInsensitive );

private:
    CR3View * _docview;
    lString16 _lastPattern;
    bool _forwardOption;
};

#endif // SEARCHDLG_H
