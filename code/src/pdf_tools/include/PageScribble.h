#ifndef PAGESCRIBBLE_H
#define PAGESCRIBBLE_H

#include <vector>

#include "PAPoint.h"
#include "PARect.h"

namespace pdfanno {
struct PageScribble {
    struct Stroke {
        PARect rect_;
        std::vector<PAPoint> points_;
        double thickness_;
        // number from 0.0 to 1.0, where 0.0 corresponds to black, 1.0 to white
        // see 8.6 "Color Spaces" section of the pdf spec
        double gray_;

        Stroke(std::vector<PAPoint> points, double thickness = 1.0, double gray = 0.0)
        {
            rect_ = PARect(points.front(), points.front());
            for (std::vector<PAPoint>::iterator it = points.begin(); it != points.end(); it++) {
                points_.push_back(*it);
                rect_.inflateTo(*it);
            }
            thickness_ = thickness;
            gray_ = gray;
        }
    };

    int page_;
    std::vector<Stroke> strokes_;
};

} // namespace

#endif // PAGESCRIBBLE_H
