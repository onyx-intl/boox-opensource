#ifndef ABSTRACTPDFANNOTATIONWRITER_H
#define ABSTRACTPDFANNOTATIONWRITER_H

#include <string>
#include <vector>

#include "GlobalDefines.h"
#include "PageScribble.h"

namespace pdfanno {

class AbstractPDFAnnotationWriter {

public:
    virtual bool openPDF(std::string docPath) = 0;
    virtual bool writeScribbles(std::vector<PageScribble> pageScribbles, PFunc_DeviceCoorTransformer pFuncDevCoortransformer = 0) = 0;
    virtual bool saveAs(std::string dstPath) = 0;
}; // class

} // namespace

#endif // ABSTRACTPDFANNOTATIONWRITER_H
