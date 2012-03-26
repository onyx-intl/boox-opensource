/*
 * DeviceScribbleReader.cpp
 *
 *  Created on: 26 Sep, 2011
 *      Author: joy
 */

#include <cassert>
#include <vector>
#include <string>

#include <QList>
#include <QRect>

#include "onyx/data/sketch_document.h"
#include "onyx/data/sketch_io.h"
#include "onyx/data/sketch_stroke.h"
#include "onyx/data/sketch_graphic_context.h"

#include "onyx/data/annotation.h"
#include "onyx/data/annotation_agent.h"

#include "../include/DeviceScribbleReader.h"
#include "../include/GlobalDefines.h"
#include "../include/PAUtil.h"
#include "../include/PageScribble.h"
#include "../include/PAPoint.h"
#include "../include/PARect.h"

using namespace pdfanno;
using namespace sketch;
using namespace anno;

static bool transformPointHelper(const PARect &cropBox, PAPoint &point);
static bool transformScribbleFromDeviceCoorToPDF(const PARect &cropBox, PageScribble &scribble);
static bool transformAnnotationFromDeviceCoorToPDF(const PARect &cropBox, anno::Annotation &annotation);

static bool getDeviceScribblePages(SketchDocument &sketch_document, sketch::Pages &pages);
static bool parseDeviceScribblePages(const sketch::Pages &pages, std::vector<PageScribble> &pageScribbles);
static bool parseDeviceScribblePage(const sketch::PageKey &key, const sketch::SketchPagePtr &page, PageScribble &parsedScribble);

DeviceScribbleReader::DeviceScribbleReader(const std::string &docPath)
    : doc_path_(docPath)
{
}

bool DeviceScribbleReader::getDocumentScribbles(std::vector<PageScribble> &pageScribbles)
{
    pageScribbles.clear();

    QString doc_path = QString::fromLocal8Bit(doc_path_.c_str());

    std::cout<<"sketch file path will be: "<<doc_path.toStdString()<<std::endl;

    QFile f(doc_path);
    if (!f.exists()) {
        return true;
    }

    SketchDocument sketch_document;
    if (!sketch_document.open(doc_path)) {
        std::cerr<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"open sketch data failed: "<<doc_path_<<std::endl;
        return false;
    }

    sketch::Pages pages;
    if (!getDeviceScribblePages(sketch_document, pages)) {
        return false;
    }

    if (!parseDeviceScribblePages(pages, pageScribbles)) {
        return false;
    }

    return true;
}

bool DeviceScribbleReader::getDocumentAnnotations(std::vector<Annotation> &pageAnnotations)
{
    pageAnnotations.clear();

    QString doc_path = QString::fromLocal8Bit(doc_path_.c_str());

    AnnotationAgent *annot_agent = new AnnotationAgent();
    if (!annot_agent->loadAllPages(doc_path)) {
        std::cerr<<"getDocumentAnnotations: open annotation data failed: " <<doc_path_<<std::endl;
        return false;
    }
    AnnotationDocumentPtr annot_doc = annot_agent->getDocument(doc_path);
    if (!annot_doc.get()) {
        assert(false);
        annot_agent->close();
        delete annot_agent;
        std::cerr<<"getDocumentAnnotations: get annotation doc failed: " <<doc_path_<<std::endl;
        return false;
    }

    // get the annotations page by page
    QList<AnnotationPagePtr> pages = annot_doc->pages().values();
    for ( QList<AnnotationPagePtr>::iterator pit = pages.begin();
          pit != pages.end();
          pit++) {
        AnnotationPagePtr page = *pit;

        if ( page == 0 || page->annotations().empty() ) {
            qDebug("empty page, continue");
            continue;
        }

        for (AnnotationIter iter = page->annotations().begin();
             iter != page->annotations().end();
             iter++ ) {
            pageAnnotations.push_back(*iter);
            qDebug("add annotation ok.");
        }
    }
    qDebug("read annotations ok.");

    annot_agent->close();
    delete annot_agent;

    return true;
}

PFunc_ScribbleDeviceCoorTransformer DeviceScribbleReader::getScribbleTransformer()
{
    return (PFunc_ScribbleDeviceCoorTransformer)transformScribbleFromDeviceCoorToPDF;
}

PFunc_AnnotationDeviceCoorTransformer DeviceScribbleReader::getAnnotationTransformer()
{
    return (PFunc_AnnotationDeviceCoorTransformer)transformAnnotationFromDeviceCoorToPDF;
}

static bool getDeviceScribblePages(SketchDocument &sketch_document, sketch::Pages &pages)
{
    SketchIOPtr sketch_io = SketchIO::getIO(sketch_document.path(), false);
    if (sketch_io == 0) {
        std::cerr<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"open sketch data failed: "<<sketch_document.path().toStdString()<<std::endl;
        return false;
    }

    if (!sketch_document.loadAllPages(sketch_io)) {
        std::cerr<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"open sketch data failed: "<<sketch_document.path().toStdString()<<std::endl;
        return false;
    }

    pages = sketch_document.pages();
    std::cout<<"pages: "<<pages.size()<<std::endl;

    int i = 0;
    QMapIterator<PageKey, SketchPagePtr> it(pages);
    while (it.hasNext()) {
        it.next();
        i++;

        SketchPagePtr page = it.value();

        if (!sketch_io->loadPageData(page)) {
            std::cerr<<"getDeviceScribblePages: loading page failed"<<std::endl;
        }
    }

    sketch_io.get()->close();
    sketch_io.reset(0);

    return true;
}

static bool parseDeviceScribblePage(const sketch::PageKey &key, const sketch::SketchPagePtr &page, PageScribble &parsedScribble)
{
    bool ok = false;
    int num_page = key.toInt(&ok);
    if (!ok) {
        std::cerr<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"parse PageKey to decimal failed: "<<key.toStdString()<<std::endl;
        assert(false);
        return false;
    }
    parsedScribble.page_ = num_page;

    Strokes strokes = page.get()->strokes();
    std::cout<<strokes.size()<<" strokes in page"<<std::endl;

    int i = 0;
    for (StrokesIter it = strokes.begin(); it != strokes.end(); it++) {
        std::cout<<"parsing stroke "<<i<<std::endl;
        i++;

        const ZoomFactor stroke_zoom_factor = it->get()->zoom();

        std::vector<PAPoint> pa_points;

        Points points = (*it).get()->points();
        std::cout<<points.size()<<" points in stroke"<<std::endl;

        int j = 0;
        for (PointsIter pit = points.begin(); pit != points.end(); pit++) {
            j++;

            pa_points.push_back(PAPoint(pit->x() / stroke_zoom_factor, pit->y() / stroke_zoom_factor));
        }

        if (pa_points.size() == 0) {
            std::cerr<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"0 points in SketchStroke"<<std::endl;
            assert(false);
            continue;
        }

        const double stroke_thickness = sketch::getPointSize(it->get()->shape(), 1.0 / stroke_zoom_factor);
        parsedScribble.strokes_.push_back(PageScribble::Stroke(pa_points, stroke_thickness));

        const PARect &rect = parsedScribble.strokes_.back().rect_;
    }

    return true;
}

static bool parseDeviceScribblePages(const sketch::Pages &pages, std::vector<PageScribble> &pageScribbles)
{
    int i = 0;
    QMapIterator<PageKey, SketchPagePtr> it(pages);
    while (it.hasNext()) {
        it.next();

        std::cout<<"parsing page "<<i<<std::endl;
        i++;

        PageKey key = it.key();
        SketchPagePtr page = it.value();

        if (!page->dataLoaded()) {
            std::cout<<"page not loaded yet"<<std::endl;
            assert(false);
            continue;
        }

        PageScribble to_parse;
        if (!parseDeviceScribblePage(key, page, to_parse)) {
            return false;
        }

        pageScribbles.push_back(to_parse);
    }

    return true;
}

static bool transformPointHelper(const PARect &cropBox, PAPoint &point)
{
    // device's origin is (left, top) = (0, 0)
    // need transform to PDF's origin(cropBox.ll_.x_, cropBox.ll_.y_)
    const int crop_box_height = cropBox.ur_.y_ - cropBox.ll_.y_;
    if (point.y_ > crop_box_height) {
        std::cerr<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<
                "point's coor (" <<point.x_ <<", " << point.y_ <<") out of CropBox [ "<<
                cropBox.ll_.x_<<" "<<cropBox.ll_.y_<<" "<<cropBox.ur_.x_<<" "<<cropBox.ur_.y_<<" ]"<<std::endl;
        return false;
    }

    point.y_ = crop_box_height - point.y_;

    point.x_ += cropBox.ll_.x_;
    point.y_ += cropBox.ll_.y_;

    return true;
}

// implementer of PFunc_ScribbleDeviceCoorTransformer
static bool transformScribbleFromDeviceCoorToPDF(const PARect &cropBox, PageScribble &scribble)
{
    for (std::vector<PageScribble::Stroke>::iterator it = scribble.strokes_.begin();
            it != scribble.strokes_.end();
            it++) {
        if (!transformPointHelper(cropBox, it->rect_.ll_)) {
            return false;
        }
        if (!transformPointHelper(cropBox, it->rect_.ur_)) {
            return false;
        }

        PAUtil::internalSwap<double>(&(it->rect_.ll_.y_), &(it->rect_.ur_.y_));

        for (std::vector<PAPoint>::iterator pit = (*it).points_.begin();
                pit != it->points_.end();
                pit++) {
            if (!transformPointHelper(cropBox, *pit)) {
                return false;
            }
        }
    }

    return true;
}

// implementer of PFunc_AnnotationDeviceCoorTransformer
static bool transformAnnotationFromDeviceCoorToPDF(const PARect &cropBox, anno::Annotation &annotation)
{
    for (QList<QRect>::iterator it = annotation.mutable_rect_list().begin();
         it != annotation.mutable_rect_list().end();
         it++) {
        QRect &rect = *it;
        qDebug("device rect: [%d %d %d %d]", rect.left(), rect.bottom(), rect.right(), rect.top());

        PAPoint ll(rect.bottomLeft().x(), rect.bottomLeft().y());
        if (!transformPointHelper(cropBox, ll)) {
            return false;
        }
        PAPoint ur(rect.topRight().x(), rect.topRight().y());
        if (!transformPointHelper(cropBox, ur)) {
            return false;
        }

        (*it).setBottomLeft(QPoint(ll.x_, ll.y_));
        (*it).setTopRight(QPoint(ur.x_, ur.y_));
        qDebug("pdf rect: [%d %d %d %d]", rect.left(), rect.bottom(), rect.right(), rect.top());
    }

    return true;
}
