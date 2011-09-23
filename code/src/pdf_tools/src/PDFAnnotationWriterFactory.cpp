/*
 * PDFAnnotationWriterFactory.cpp
 *
 *  Created on: 23 Sep, 2011
 *      Author: joy
 */

#include "PDFAnnotationWriterFactory.h"
#include "PoDoFoAnnotationWriter.h"

using namespace pdfanno;

class PDFAnnotationWriterFactory::Impl {
public:
    Impl()
    {
        writer_ = new PoDoFoAnnotationWriter();
    };
    virtual ~Impl()
    {
        delete writer_;
        writer_ = 0;
    };

    PoDoFoAnnotationWriter *getWriter()
    {
        assert(writer_);
        return writer_;
    }

private:
    PoDoFoAnnotationWriter *writer_;
};

PDFAnnotationWriterFactory::PDFAnnotationWriterFactory()
{
    pImpl_ = new Impl();
}

PDFAnnotationWriterFactory::~PDFAnnotationWriterFactory()
{
    delete pImpl_;
    pImpl_ = 0;
}

AbstractPDFAnnotationWriter* PDFAnnotationWriterFactory::getAnnotationWriter(void)
{
    assert(pImpl_);
    return pImpl_->getWriter();
}
