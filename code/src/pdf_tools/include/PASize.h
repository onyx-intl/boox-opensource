/*
 * PASize.h
 *
 *  Created on: 26 Sep, 2011
 *      Author: joy
 */

#ifndef PASIZE_H_
#define PASIZE_H_

namespace pdfanno {

struct PASize {
    double width_, height_;

    PASize(double width, double height)
        : width_(width), height_(height)
    {
    }

    PASize(const PASize &copy)
        : width_(copy.width_), height_(copy.height_)
    {
    }
}; // PASize

} // namespace

#endif /* PASIZE_H_ */
