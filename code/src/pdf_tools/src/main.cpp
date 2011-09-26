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

    DeviceScribbleReader device_reader;
    if (!device_reader.getDocumentScribbles(docPath, page_scribbles)) {
        return false;
    }

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<std::endl;

    PDFAnnotationWriterFactory factory;
    AbstractPDFAnnotationWriter* annot_writer = factory.getAnnotationWriter();
    if (!annot_writer) {
        assert(false);
        return false;
    }

    if (!annot_writer->openPDF(docPath)) {
        return false;
    }

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<std::endl;
    if (device_reader.getTransformer()) {
        if (!annot_writer->writeScribbles(page_scribbles, device_reader.getTransformer())) {
            return false;
        }
    }
    else {
        if (!annot_writer->writeScribbles(page_scribbles)) {
            return false;
        }
    }

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<std::endl;
    if (!annot_writer->saveAs(PAUtil::getSaveAsPath(docPath))) {
        return false;
    }

    std::cout<<"["<<__FILE__<<", "<<__func__<<", "<<__LINE__<<"]"<<std::endl;

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
