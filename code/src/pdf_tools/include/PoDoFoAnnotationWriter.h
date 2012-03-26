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

#include "onyx/data/annotation.h"

#include "podofo/podofo.h"

#include "GlobalDefines.h"
#include "PAUtil.h"
#include "AbstractPDFAnnotationWriter.h"
#include "PageScribble.h"

namespace pdfanno {

class PoDoFoAnnotationWriter : public AbstractPDFAnnotationWriter {

public:
    PoDoFoAnnotationWriter();

    virtual ~PoDoFoAnnotationWriter();

public:
    bool openPDF(const std::string &docPath);
    // write page scribbles infomation to PDF
    // because page scribbles coming from device, so may need transforming device's coor to PDF's
    // if so, pFuncDevCoortransformer is provided
    bool writeScribbles(std::vector<PageScribble> &pageScribbles,
                        PFunc_ScribbleDeviceCoorTransformer pFuncScribbleTransformer = 0);
    bool writeAnnotations(std::vector<anno::Annotation> &pageAnnotations,
                          PFunc_AnnotationDeviceCoorTransformer pFuncAnnotationTransformer = 0);
    bool saveAs(const std::string &dstPath);

private:
    bool closeCore();

private:
    std::string docPath_;
    PoDoFo::PdfMemDocument *doc_;
}; // class

} // namespace

#endif /* PODOFOANNOTATIONCREATOR_H_ */
