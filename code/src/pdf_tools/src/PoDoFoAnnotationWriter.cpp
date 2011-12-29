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

#include "podofo/podofo.h"

#include "../include/PoDoFoAnnotationWriter.h"
#include "../include/PAUtil.h"
#include "../include/AbstractPDFAnnotationWriter.h"
#include "../include/PASize.h"
#include "../include/PageScribble.h"

using namespace pdfanno;

static bool createAnnotationPolyLine(PoDoFo::PdfDocument *document, PoDoFo::PdfPage *page, const PageScribble::Stroke &stroke);

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

bool PoDoFoAnnotationWriter::openPDF(std::string docPath)
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

bool PoDoFoAnnotationWriter::writeScribbles(std::vector<PageScribble> pageScribbles, PFunc_DeviceCoorTransformer pFuncDevCoortransformer)
{
    if (!doc_) {
        assert(false);
        return false;
    }

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"combinePageScribbles begins"<<std::endl;

    std::vector<PageScribble>::size_type num_pages = pageScribbles.size();
    std::cerr<< "["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]" << "scribbled pages: " << num_pages << std::endl;

    for (std::vector<PageScribble>::size_type i = 0; i < num_pages; i++) {
        std::cerr<< "["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]" << "get page: " << i << std::endl;

        PoDoFo::PdfPage *page = doc_->GetPage(pageScribbles[i].page_);
        if (!page) {
            std::cerr<< "["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]" << "get page failed: " << i << std::endl;
            return false;
        }

        if (pFuncDevCoortransformer) {
            const PoDoFo::PdfRect &crop_box = page->GetCropBox();
            std::cout<<"CropBox ("<<crop_box.GetLeft()<<", "<<crop_box.GetBottom()<<
                    "), width: "<<crop_box.GetWidth()<<", height: "<<crop_box.GetHeight()<<std::endl;

            const PARect pdf_rect(PAPoint(crop_box.GetLeft(), crop_box.GetBottom()),
                    PAPoint(crop_box.GetLeft() + crop_box.GetWidth(), crop_box.GetBottom() + crop_box.GetHeight()));
            if (!pFuncDevCoortransformer(pdf_rect, pageScribbles[i])) {
                continue;
            }
        }

        for (std::vector<PageScribble::Stroke>::iterator it = pageScribbles[i].strokes_.begin();
                it != pageScribbles[i].strokes_.end();
                it++) {
            if (!createAnnotationPolyLine(doc_, page, *it)) {
                continue;
            }
        }
    }

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"combinePageScribbles finished"<<std::endl;

    return true;
}

bool PoDoFoAnnotationWriter::saveAs(std::string dstPath)
{
    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"SaveAs begins"<<std::endl;

    assert(doc_);

    if (doc_->GetInfo() && doc_->GetInfo()->GetTitle().IsValid()) {
        std::string utf8_title = doc_->GetInfo()->GetTitle().GetStringUtf8();
        std::cout<<"original title: " << utf8_title <<std::endl;

        std::string new_title = std::string(utf8_title) + " - " + PAUtil::getMergeMarkAsPostfix();

        PoDoFo::PdfString dst_title((PoDoFo::pdf_utf8 *)new_title.c_str());
        doc_->GetInfo()->SetTitle(dst_title);
    }

    doc_->Write(dstPath.c_str());

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"SaveAs finished"<<std::endl;

    return true;
}

// substitute of virtual method #close()#
bool PoDoFoAnnotationWriter::closeCore()
{
    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"closeCore begins"<<std::endl;

    if (!doc_) {
        assert(false);
        return true;
    }

    delete doc_;
    doc_ = 0;

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"closeCore finished"<<std::endl;

    return true;
}

static bool createAnnotationPolyLine(PoDoFo::PdfDocument *document, PoDoFo::PdfPage *page, const PageScribble::Stroke &stroke)
{
    assert(document && page);

    using namespace PoDoFo;

    const PARect &rect = stroke.rect_;
    const std::vector<PAPoint> &points = stroke.points_;
    const double thickness = stroke.thickness_;

    PdfRect pdf_rect(rect.ll_.x_, rect.ll_.y_, rect.getWidth(), rect.getHeight());

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"Create PolyLine"<<std::endl;

    PdfAnnotation *annot_polyline = page->CreateAnnotation(ePdfAnnotation_PolyLine, pdf_rect);
    annot_polyline->SetColor(128, 0, 0);
    //annot_polyline->SetFlags(4);

    PdfDictionary &dict = annot_polyline->GetObject()->GetDictionary();
    PdfArray *vertices = new PdfArray();
    for (std::vector<PAPoint>::const_iterator it = points.begin(); it != points.end(); ++it) {
        vertices->push_back(it->x_);
        vertices->push_back(it->y_);
    }
    dict.AddKey(PdfName("Vertices"), *vertices);

    std::cout<<"PolyLine begins"<<std::endl;
    std::cout<<"Rect: "<<rect.ll_.x_<<","<<rect.ll_.y_<<","<<rect.ur_.x_<<","<<rect.ur_.y_<<std::endl;

    PdfXObject *xobj = new PdfXObject(pdf_rect, document);
    PdfPainter pnt;
    pnt.SetPage(xobj);
    pnt.SetStrokeWidth(thickness);
    for (int i = 0; i < points.size() - 1; i++) {
        std::cout<<"Line " <<i<<": ("<<points[i].x_<<","<<points[i].y_<<"), ("<<points[i + 1].x_<<","<<points[i + 1].y_<<")"<<std::endl;
        pnt.DrawLine(points[i].x_, points[i].y_, points[i + 1].x_, points[i + 1].y_);
    }
    pnt.FinishPage();

    std::cout<<"PolyLine finished"<<std::endl;

    annot_polyline->SetAppearanceStream(xobj);

    return true;
}
