#include <cassert>
#include <cstdlib>

#include <string>
#include <iostream>
#include <vector>

#include "PAUtil.h"
#include "PageScribble.h"
#include "DeviceScribbleReader.h"
#include "AbstractPDFAnnotationWriter.h"
#include "PDFAnnotationWriterFactory.h"

using namespace pdfanno;

void printUsage()
{
    std::cout<<"app xxx.pdf"<<std::endl;
}

bool testCombinePageScribbles(std::string docPath)
{
    std::vector<PageScribble> page_scribbles;
    std::vector<anno::Annotation> page_annotations;

    DeviceScribbleReader device_reader(docPath);
    if (!device_reader.getDocumentScribbles(page_scribbles)) {
        return false;
    }
    if (!device_reader.getDocumentAnnotations(page_annotations)) {
        return false;
    }

    PDFAnnotationWriterFactory factory;
    AbstractPDFAnnotationWriter* annot_writer = factory.getAnnotationWriter();
    if (!annot_writer) {
        assert(false);
        return false;
    }
    if (!annot_writer->openPDF(docPath)) {
        return false;
    }
    if (!annot_writer->writeScribbles(page_scribbles, device_reader.getScribbleTransformer())) {
        return false;
    }
    if (!annot_writer->writeAnnotations(page_annotations, device_reader.getAnnotationTransformer())) {
        return false;
    }
    if (!annot_writer->saveAs(PAUtil::getSaveAsPath(docPath))) {
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        printUsage();
        return -1;
    }

    std::string doc_path = std::string(argv[1]);
    if (!testCombinePageScribbles(doc_path)) {
        return -1;
    }

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<"testCombinePageScribbles finished"<<std::endl;

    return 0;
}
