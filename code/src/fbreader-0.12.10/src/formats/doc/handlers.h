#ifndef ONYX_DOC_HANDLER_H_
#define ONYX_DOC_HANDLER_H_

// libwv2
#include <wv2-0.4.2/src/parser.h>
#include <wv2-0.4.2/src/handlers.h>
#include <wv2-0.4.2/src/parserfactory.h>
#include <wv2-0.4.2/src/word97_generated.h>
#include <wv2-0.4.2/src/paragraphproperties.h>
#include <wv2-0.4.2/src/lists.h>
#include <wv2-0.4.2/src/ustring.h>
#include <wv2-0.4.2/src/fields.h>

#include <iostream>
#include <string>

#include <QString>

class DocBookReader;
void setBookReader(DocBookReader *reader);
DocBookReader *bookReader();



/// Define doc parser stack.
enum DocTextKind
{
    INVALID_KIND = -1,
    DOC_TOTAL_PAGE,
    DOC_UNKNOWN,
    DOC_TOC,
    DOC_TOC_ENTRY,
    DOC_TOC_REFERENCE,
    DOC_EXTERNAL_HYPERLINK,
    DOC_TEXT,
};

struct StackItem
{
public:
    StackItem();
    StackItem(DocTextKind k);
    StackItem(DocTextKind k, const QString &t);
    StackItem(const StackItem & right);
    ~StackItem();

    StackItem & operator =(const StackItem & right);

public:
    DocTextKind kind;
    QString text;
};

class DocParserStack
{
public:
    DocParserStack();
    ~DocParserStack();

public:
    void push(DocTextKind kind, const QString & t = QString());
    StackItem pop();
    StackItem top();
    DocTextKind topKind();
    bool isEmpty();

    void appendText(const QString &);

    void reset();
private:
    std::vector<StackItem> stack_;
};

class MyInlineReplacementHandler : public wvWare::InlineReplacementHandler
{
public:
    virtual wvWare::U8 tab();
    virtual wvWare::U8 hardLineBreak();
    virtual wvWare::U8 columnBreak();
    virtual wvWare::U8 nonBreakingHyphen();
    virtual wvWare::U8 nonRequiredHyphen();
    virtual wvWare::U8 nonBreakingSpace();
};


class MySubDocumentHandler : public wvWare::SubDocumentHandler
{
public:
    virtual void bodyStart();
    virtual void bodyEnd();

    virtual void footnoteStart();
    virtual void footnoteEnd();

    virtual void headersStart();
    virtual void headersEnd();
    virtual void headerStart( wvWare::HeaderData::Type type );
    virtual void headerEnd();
};


class MyTableHandler : public wvWare::TableHandler
{
public:
    virtual void tableRowStart( wvWare::SharedPtr<const wvWare::Word97::TAP> tap );
    virtual void tableRowEnd();
    virtual void tableCellStart();
    virtual void tableCellEnd();
};


class MyPictureHandler : public wvWare::PictureHandler
{
public:
    virtual void bitmapData( wvWare::OLEImageReader& reader, wvWare::SharedPtr<const wvWare::Word97::PICF> picf );
    virtual void wmfData( wvWare::OLEImageReader& reader, wvWare::SharedPtr<const wvWare::Word97::PICF> picf );
    virtual void externalImage( const wvWare::UString& name, wvWare::SharedPtr<const wvWare::Word97::PICF> picf );
};


class MyTextHandler : public wvWare::TextHandler
{
public:
    virtual void sectionStart( wvWare::SharedPtr<const wvWare::Word97::SEP> sep );
    virtual void sectionEnd();
    virtual void pageBreak();

    virtual void headersFound( const wvWare::HeaderFunctor& parseHeaders );

    virtual void paragraphStart( wvWare::SharedPtr<const wvWare::ParagraphProperties> paragraphProperties );
    virtual void paragraphEnd();

    virtual void runOfText( const wvWare::UString& text, wvWare::SharedPtr<const wvWare::Word97::CHP> chp );

    virtual void specialCharacter( SpecialCharacter character, wvWare::SharedPtr<const wvWare::Word97::CHP> chp );

    virtual void footnoteFound( wvWare::FootnoteData::Type type, wvWare::UChar character,
                                wvWare::SharedPtr<const wvWare::Word97::CHP> chp, const wvWare::FootnoteFunctor& parseFootnote );
    virtual void footnoteAutoNumber( wvWare::SharedPtr<const wvWare::Word97::CHP> chp );

    virtual void fieldStart( const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> chp );
    virtual void fieldSeparator( const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> chp );
    virtual void fieldEnd( const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> chp );

    virtual void tableRowFound( const wvWare::TableRowFunctor& tableRow, wvWare::SharedPtr<const wvWare::Word97::TAP> tap );

    virtual void pictureFound( const wvWare::PictureFunctor& picture, wvWare::SharedPtr<const wvWare::Word97::PICF> picf,
                               wvWare::SharedPtr<const wvWare::Word97::CHP> chp );

    void parseHyperlink(QString &t, QString * pos);
    void addText(const QString &t,  wvWare::SharedPtr<const wvWare::Word97::CHP> chp);


private:
    bool empty_paragraph_;
    QString text_;
};


#endif
