/*
 * PDFAnnotationWriterFactory.h
 *
 *  Created on: 23 Sep, 2011
 *      Author: joy
 */

#ifndef PDFANNOTATIONWRITERFACTORY_H_
#define PDFANNOTATIONWRITERFACTORY_H_

#include <AbstractPDFAnnotationWriter.h>

namespace pdfanno {

class PDFAnnotationWriterFactory {
public :
    PDFAnnotationWriterFactory();
    virtual ~PDFAnnotationWriterFactory();

    AbstractPDFAnnotationWriter* getAnnotationWriter(void);
private:
    class Impl;
    Impl *pImpl_;
}; // PDFAnnotationWriterFactory

} // namespace


#endif /* PDFANNOTATIONWRITERFACTORY_H_ */
