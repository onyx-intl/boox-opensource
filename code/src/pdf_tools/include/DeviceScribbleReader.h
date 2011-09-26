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

#include "GlobalDefines.h"
#include "PageScribble.h"
#include "PASize.h"

namespace pdfanno {

class DeviceScribbleReader {
public:
    bool getDocumentScribbles(std::string docPath, std::vector<PageScribble> &pageScribbles);
    // if corrds from device need to be transformed to PDF's, then provide transformer function, else return 0
    PFunc_DeviceCoorTransformer getTransformer();
}; // class

} // namespace


#endif /* DEVICESCRIBBLEREADER_H_ */
