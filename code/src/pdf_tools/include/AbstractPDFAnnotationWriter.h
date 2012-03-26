#ifndef ABSTRACTPDFANNOTATIONWRITER_H
#define ABSTRACTPDFANNOTATIONWRITER_H

#include <string>
#include <vector>

#include "onyx/data/annotation.h"

#include "GlobalDefines.h"
#include "PageScribble.h"

namespace pdfanno {

class AbstractPDFAnnotationWriter {

public:
    virtual bool openPDF(const std::string &docPath) = 0;
    virtual bool writeScribbles(std::vector<PageScribble> &pageScribbles, PFunc_ScribbleDeviceCoorTransformer pFuncDevCoortransformer = 0) = 0;
    virtual bool writeAnnotations(std::vector<anno::Annotation> &pageAnnotations, PFunc_AnnotationDeviceCoorTransformer pFuncAnnotationTransformer = 0) = 0;
    virtual bool saveAs(const std::string &dstPath) = 0;
}; // class

} // namespace

#endif // ABSTRACTPDFANNOTATIONWRITER_H
