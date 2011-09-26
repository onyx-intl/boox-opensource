/*
 * GlobalDefines.h
 *
 *  Created on: 26 Sep, 2011
 *      Author: joy
 */

#ifndef GLOBALDEFINES_H_
#define GLOBALDEFINES_H_

#include "PASize.h"
#include "PageScribble.h"

namespace pdfanno {
    // prototype of function for tranforming device's coord to PDF's
    typedef bool (*PFunc_DeviceCoorTransformer)(const PASize &pageSize, PageScribble &pageScribble);
}

#endif /* GLOBALDEFINES_H_ */
