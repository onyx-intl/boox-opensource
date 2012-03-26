/*
 * GlobalDefines.h
 *
 *  Created on: 26 Sep, 2011
 *      Author: joy
 */

#ifndef GLOBALDEFINES_H_
#define GLOBALDEFINES_H_

#include "onyx/data/annotation.h"

#include "PASize.h"
#include "PARect.h"
#include "PageScribble.h"

namespace pdfanno {
    // prototype of function for tranforming scribble from device's coord to PDF's
    typedef bool (*PFunc_ScribbleDeviceCoorTransformer)(const PARect &cropBox, PageScribble &scribble);
    typedef bool (*PFunc_AnnotationDeviceCoorTransformer)(const PARect &cropBox, anno::Annotation &annotation);
}

#endif /* GLOBALDEFINES_H_ */
