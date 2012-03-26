/*
 * DeviceScribbleReader.h
 *
 *  Created on: 22 Sep, 2011
 *      Author: joy@onyx
 */

#ifndef DEVICESCRIBBLEREADER_H_
#define DEVICESCRIBBLEREADER_H_

#include <string>
#include <vector>

#include "onyx/data/annotation.h"

#include "GlobalDefines.h"
#include "PageScribble.h"
#include "PASize.h"

namespace pdfanno {

class DeviceScribbleReader {
public:
    DeviceScribbleReader(const std::string &docPath);

    bool getDocumentScribbles(std::vector<PageScribble> &pageScribbles);
    bool getDocumentAnnotations(std::vector<anno::Annotation> &pageAnnotations);

    // if coords from device need to be transformed to PDF's, then provide transformer function, else return 0
    PFunc_ScribbleDeviceCoorTransformer getScribbleTransformer();
    PFunc_AnnotationDeviceCoorTransformer getAnnotationTransformer();

private:
    std::string doc_path_;
}; // class

} // namespace


#endif /* DEVICESCRIBBLEREADER_H_ */
