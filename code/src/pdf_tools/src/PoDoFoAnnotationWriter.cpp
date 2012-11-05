/*
 * PoDoFoAnnotationWriter.cpp
 *
 *  Created on: 26 Sep, 2011
 *      Author: joy
 */

#include <cassert>

#include <string>
#include <vector>
#include <iostream>

#include <QWidget>
#include <QDebug>

#include "podofo/podofo.h"

#include "../include/PoDoFoAnnotationWriter.h"
#include "../include/PAUtil.h"
#include "../include/AbstractPDFAnnotationWriter.h"
#include "../include/PASize.h"
#include "../include/PageScribble.h"
#include "../include/DeviceScribbleReader.h"

using namespace pdfanno;

static bool createAnnotationPolyLine(PoDoFo::PdfDocument *document, PoDoFo::PdfPage *page, const PageScribble::Stroke &stroke);
static bool createAnnotationHightlight(PoDoFo::PdfDocument *document, PoDoFo::PdfPage *page,
                                       const anno::Annotation &annotation);

PoDoFoAnnotationWriter::PoDoFoAnnotationWriter()
{
    doc_ = 0;
}

PoDoFoAnnotationWriter::~PoDoFoAnnotationWriter()
{
    if (doc_) {
        this->closeCore();
    }
}

bool PoDoFoAnnotationWriter::openPDF(const std::string &docPath)
{
    if (doc_) {
        if (!this->closeCore()) {
            return false;
        }
    }

    doc_ = new PoDoFo::PdfMemDocument(docPath.c_str());

    if (!doc_->GetInfo()) {
        delete doc_;
        doc_ = 0;
        return false;
    }

    this->docPath_ = docPath;

    return true;
}

bool PoDoFoAnnotationWriter::writeScribbles(std::vector<PageScribble> &pageScribbles,
                                            PFunc_ScribbleDeviceCoorTransformer pFuncScribbleTransformer)
{
    if (!doc_) {
        assert(false);
        return false;
    }

    std::vector<PageScribble>::size_type num_pages = pageScribbles.size();
    for (std::vector<PageScribble>::size_type i = 0; i < num_pages; i++) {
        PoDoFo::PdfPage *page = doc_->GetPage(pageScribbles[i].page_);
        if (!page) {
            std::cerr<< "["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]" << "get page failed: " << i << std::endl;
            continue;
        }

        if (pFuncScribbleTransformer) {
            const PoDoFo::PdfRect crop_box = page->GetCropBox();
            const PARect pdf_rect(PAPoint(crop_box.GetLeft(), crop_box.GetBottom()),
                    PAPoint(crop_box.GetLeft() + crop_box.GetWidth(), crop_box.GetBottom() + crop_box.GetHeight()));
            if (!pFuncScribbleTransformer(pdf_rect, pageScribbles[i])) {
                std::cerr<<"writeScribbles: transform scribble failed"<<std::endl;
                continue;
            }
        }

        for (std::vector<PageScribble::Stroke>::const_iterator it = pageScribbles[i].strokes_.begin();
                it != pageScribbles[i].strokes_.end();
                it++) {
            if (!createAnnotationPolyLine(doc_, page, *it)) {
                continue;
            }
        }
    }

    return true;
}

bool PoDoFoAnnotationWriter::writeAnnotations(std::vector<anno::Annotation> &pageAnnotations,
                                              PFunc_AnnotationDeviceCoorTransformer pFuncAnnotationTransformer)
{
    qDebug("writeAnnotations begins.");
    if (!doc_) {
        assert(false);
        return false;
    }

    for (std::vector<anno::Annotation>::iterator it = pageAnnotations.begin();
         it != pageAnnotations.end();
         it++) {
        qDebug("iterating annotation");
        anno::Annotation annot = *it;

        PoDoFo::PdfPage *page = doc_->GetPage(annot.page());
        if (!page) {
            std::cerr<< "["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]" << "get page failed: " << annot.page() << std::endl;
            continue;
        }

        if (pFuncAnnotationTransformer) {
            qDebug("transforming annotation");
            const PoDoFo::PdfRect crop_box = page->GetCropBox();
            const PARect pdf_rect(PAPoint(crop_box.GetLeft(), crop_box.GetBottom()),
                    PAPoint(crop_box.GetLeft() + crop_box.GetWidth(), crop_box.GetBottom() + crop_box.GetHeight()));
            if (!pFuncAnnotationTransformer(pdf_rect, annot)) {
                std::cerr<<"writeAnnotations: transform annotation failed"<<std::endl;
                continue;
            }
        }

        if (!createAnnotationHightlight(doc_, page, annot)) {
            continue;
        }
    }

    return true;
}

bool PoDoFoAnnotationWriter::saveAs(const std::string &dstPath)
{
    assert(doc_);

    if (doc_->GetInfo() && doc_->GetInfo()->GetTitle().IsValid()) {
        std::string utf8_title = doc_->GetInfo()->GetTitle().GetStringUtf8();

        std::string new_title = std::string(utf8_title) + " - " + PAUtil::getMergeMarkAsPostfix();

        PoDoFo::PdfString dst_title((PoDoFo::pdf_utf8 *)new_title.c_str());
        doc_->GetInfo()->SetTitle(dst_title);
    }

    doc_->Write(dstPath.c_str());

    return true;
}

// substitute of virtual method #close()#
bool PoDoFoAnnotationWriter::closeCore()
{
    if (!doc_) {
        assert(false);
        return true;
    }

    delete doc_;
    doc_ = 0;

    return true;
}

static bool createAnnotationPolyLine(PoDoFo::PdfDocument *document, PoDoFo::PdfPage *page, const PageScribble::Stroke &stroke)
{
    assert(document && page);

    using namespace PoDoFo;

    const PARect &rect = stroke.rect_;
    const std::vector<PAPoint> &points = stroke.points_;
    const double thickness = stroke.thickness_;
    const double gray = static_cast<double>(stroke.gray_);

    PdfRect pdf_rect(rect.ll_.x_, rect.ll_.y_, rect.getWidth(), rect.getHeight());

    PdfAnnotation *annot_polyline = page->CreateAnnotation(ePdfAnnotation_PolyLine, pdf_rect);
    annot_polyline->SetColor(gray);

    PdfDictionary &dict = annot_polyline->GetObject()->GetDictionary();
    PdfArray *vertices = new PdfArray();
    for (std::vector<PAPoint>::const_iterator it = points.begin(); it != points.end(); ++it) {
        vertices->push_back(it->x_);
        vertices->push_back(it->y_);
    }
    dict.AddKey(PdfName("Vertices"), *vertices);

    PdfXObject *xobj = new PdfXObject(pdf_rect, document);
    PdfPainter pnt;
    pnt.SetPage(xobj);
    pnt.SetStrokeWidth(thickness);
    pnt.SetStrokingGray(gray);
    for (int i = 0; i < points.size() - 1; i++) {
        pnt.DrawLine(points[i].x_, points[i].y_, points[i + 1].x_, points[i + 1].y_);
    }
    pnt.FinishPage();

    annot_polyline->SetAppearanceStream(xobj);

    return true;
}

static bool createAnnotationHightlight(PoDoFo::PdfDocument *document, PoDoFo::PdfPage *page,
                                       const anno::Annotation &annotation)
{
    qDebug("createAnnotationHightlight begins.");
    assert(document && page);

    using namespace PoDoFo;

    QList<QRect> rect_list = annotation.rect_list();
    if (rect_list.isEmpty()) {
        assert(false);
        return false;
    }

    PARect bound_rect(PAPoint(rect_list.first().bottomLeft().x(), rect_list.first().bottomLeft().y()),
                      PAPoint(rect_list.first().topRight().x(), rect_list.first().topRight().y()));
    for (QList<QRect>::const_iterator it = annotation.rect_list().begin();
         it != annotation.rect_list().end();
         it++) {
        const QRect r = *it;
        bound_rect.unite(PARect(PAPoint(r.bottomLeft().x(), r.bottomLeft().y()),
                                PAPoint(r.topRight().x(), r.topRight().y())));
    }

    PdfRect pdf_rect(bound_rect.ll_.x_, bound_rect.ll_.y_,
                     bound_rect.getWidth(), bound_rect.getHeight());
    PdfAnnotation *annot_highlight = page->CreateAnnotation(ePdfAnnotation_Highlight, pdf_rect);
    if (!annot_highlight) {
        qDebug() << "can not create highlight.";
        return false;
    }

    annot_highlight->SetContents(PdfString((pdf_utf8*)annotation.title().toUtf8().constData()));
    qDebug("SetContents");

    // default (1.0, 1.0, 0.0) is very obscure on device,
    // so we choose darker color instead
    const PdfColor color(0.25, 0.25, 0.0);
    annot_highlight->SetColor(color.GetRed(), color.GetGreen(), color.GetBlue());
    qDebug("SetColor");

    PdfXObject *xobj = new PdfXObject(pdf_rect, document);
    PdfPainter pnt;
    pnt.SetPage(xobj);
    PdfExtGState *ext_gstate = new PdfExtGState((document));
    ext_gstate->SetFillOpacity(0.5);
    pnt.SetExtGState(ext_gstate);
    pnt.SetColor(color);

    PdfArray quads;
    for (QList<QRect>::const_iterator it = annotation.rect_list().begin();
         it != annotation.rect_list().end();
         it++) {
        const QRect r = *it;

        quads.push_back((double)r.left());
        quads.push_back((double)r.top());
        quads.push_back((double)r.right());
        quads.push_back((double)r.top());
        quads.push_back((double)r.left());
        quads.push_back((double)r.bottom());
        quads.push_back((double)r.right());
        quads.push_back((double)r.bottom());

        // when SetExtGState on PdfPainter, we have to use r.top() as rect.bottom()
        // may be bug in podofo
        const int rect_bottom = r.top();
        pnt.FillRect(r.left(), rect_bottom, r.width(), r.height());
    }

    annot_highlight->SetQuadPoints(quads);

    pnt.FinishPage();
    annot_highlight->SetAppearanceStream(xobj);

    return true;
}
