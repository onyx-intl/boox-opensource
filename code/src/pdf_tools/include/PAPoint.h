#ifndef PAPOINT_H
#define PAPOINT_H

namespace pdfanno {
struct PAPoint {
    double x_, y_;

    PAPoint()
        : x_(0), y_(0)
    {
    }
    PAPoint(double x, double y)
        : x_(x), y_(y)
    {
    }
    PAPoint(const PAPoint &copy)
        : x_(copy.x_), y_(copy.y_)
    {
    }
};

} // namespace

#endif // PAPOINT_H
