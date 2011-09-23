/*
 * PoDoFoAnnotationWriter.h
 *
 *  Created on: 22 Sep, 2011
 *      Author: joy@onyx
 */


#ifndef PODOFOANNOTATIONWRITER_H_
#define PODOFOANNOTATIONWRITER_H_

#include <cassert>

#include <string>
#include <vector>
#include <iostream>

#include "podofo/podofo.h"

#include "PAUtil.h"
#include "AbstractPDFAnnotationWriter.h"
#include "PageScribble.h"

namespace pdfanno {

class PoDoFoAnnotationWriter : public AbstractPDFAnnotationWriter {

public:
    PoDoFoAnnotationWriter()
    {
        doc_ = 0;
    }

    virtual ~PoDoFoAnnotationWriter()
    {
        if (doc_) {
            this->closeCore();
        }
    }

public:
    bool openPDF(std::string docPath)
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

    bool writeScribbles(std::vector<PageScribble> pageScribbles)
    {
        if (!doc_) {
            assert(false);
            return false;
        }

        std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"combinePageScribbles begins"<<std::endl;

        if (!this->combinePageScribbles(doc_, pageScribbles)) {
            return false;
        }

        std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"combinePageScribbles finished"<<std::endl;

        return true;
    }

    bool saveAs(std::string dstPath)
    {
        std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"SaveAs begins"<<std::endl;

        assert(doc_);
        doc_->Write(dstPath.c_str());

        std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"SaveAs finished"<<std::endl;

        return true;
    }

    // not necessary, just do it if you like
    bool close()
    {
        return this->closeCore();
    }

private:

    // substitute of virtual method #close()#
    bool closeCore()
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

    bool tranformDeviceCoorToPDF(const int pdfPageHeight, PageScribble &pageScribble)
    {
        for (std::vector<PageScribble::Stroke>::iterator it = pageScribble.strokes_.begin();
             it != pageScribble.strokes_.end();
             it++) {
            if (it->rect_.ll_.y_ > pdfPageHeight) {
                std::cerr<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"point's coor (" <<
                        it->rect_.ll_.x_ <<", " << it->rect_.ll_.y_ <<
                        ") out of page range height: "<<pdfPageHeight<<std::endl;
                assert(false);
                return false;
                continue;
            }
            it->rect_.ll_.y_ = pdfPageHeight - it->rect_.ll_.y_;

            if (it->rect_.ur_.y_ > pdfPageHeight) {
                std::cerr<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"point's coor (" <<
                        it->rect_.ur_.x_ <<", " << it->rect_.ur_.y_ <<
                        ") out of page range height: "<<pdfPageHeight<<std::endl;
                assert(false);
                return false;
                continue;
            }
            it->rect_.ur_.y_ = pdfPageHeight - it->rect_.ur_.y_;
            PAUtil::internalSwap<double>(&(it->rect_.ll_.y_), &(it->rect_.ur_.y_));

            for (std::vector<PAPoint>::iterator pit = (*it).points_.begin();
                 pit != it->points_.end();
                 pit++) {
                if (pit->y_ > pdfPageHeight) {
                    std::cerr<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"point's coor (" <<
                            (*pit).x_ <<", " << (*pit).y_ << ") out of page range height: "<<pdfPageHeight<<std::endl;
                    assert(false);
                    return false;
                    continue;
                }
                pit->y_ = pdfPageHeight - pit->y_;
            }
        }

        return true;
    }

    bool createAnnotationPolyLine(PoDoFo::PdfDocument *document, PoDoFo::PdfPage *page, const PARect &rect, const std::vector<PAPoint> &points)
    {
        assert(document && page);

        using namespace PoDoFo;

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
        pnt.SetStrokeWidth(1);
        for (int i = 0; i < points.size() - 1; i++) {
            std::cout<<"Line " <<i<<": ("<<points[i].x_<<","<<points[i].y_<<"), ("<<points[i + 1].x_<<","<<points[i + 1].y_<<")"<<std::endl;
            pnt.DrawLine(points[i].x_, points[i].y_, points[i + 1].x_, points[i + 1].y_);
        }
        pnt.FinishPage();

        std::cout<<"PolyLine finished"<<std::endl;

        annot_polyline->SetAppearanceStream(xobj);

        return true;
    }

    bool combinePageScribbles(PoDoFo::PdfDocument *doc, std::vector<PageScribble> &pageScribbles)
    {
        assert(doc);

        std::vector<PageScribble>::size_type num_pages = pageScribbles.size();
        std::cerr<< "["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]" << "scribbled pages: " << num_pages << std::endl;

        for (std::vector<PageScribble>::size_type i = 0; i < num_pages; i++) {
            std::cerr<< "["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]" << "get page: " << i << std::endl;

            PoDoFo::PdfPage *page = doc->GetPage(pageScribbles[i].page_);
            if (!page) {
                std::cerr<< "["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]" << "get page failed: " << i << std::endl;
                return false;
            }

            if (!this->tranformDeviceCoorToPDF(page->GetCropBox().GetHeight(), pageScribbles[i])) {
                continue;
            }

            for (std::vector<PageScribble::Stroke>::iterator it = pageScribbles[i].strokes_.begin();
                 it != pageScribbles[i].strokes_.end();
                 it++) {
                if (!this->createAnnotationPolyLine(doc, page, it->rect_, it->points_)) {
                    continue;
                }
            }
        }

        return true;
    }

private:
    std::string docPath_;
    PoDoFo::PdfMemDocument *doc_;
}; // class

} // namespace

#endif /* PODOFOANNOTATIONCREATOR_H_ */
