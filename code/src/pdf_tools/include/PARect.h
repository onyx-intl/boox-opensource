#ifndef PARECT_H
#define PARECT_H

#include "PAPoint.h"

namespace pdfanno {
struct PARect {
    PAPoint ll_, ur_;

    PARect()
        : ll_(), ur_()
    {
    }

    PARect(PAPoint ll, PAPoint ur)
        : ll_(ll), ur_(ur)
    {
    }

    PARect &inflateTo(const PAPoint &dst)
    {
        if (dst.x_ < ll_.x_) {
            ll_.x_ = dst.x_;
        }
        else if (dst.x_ > ur_.x_) {
            ur_.x_ = dst.x_;
        }

        if (dst.y_ < ll_.y_) {
            ll_.y_ = dst.y_;
        }
        else if (dst.y_ > ur_.y_) {
            ur_.y_ = dst.y_;
        }

        return *this;
    }

    PARect &unite(const PARect &rect)
    {
        return this->inflateTo(rect.ll_).inflateTo(rect.ur_);
    }

    const double getWidth() const
    {
        return ur_.x_ - ll_.x_;
    }

    const double getHeight() const
    {
        return ur_.y_ - ll_.y_;
    }
};

} // namespace

#endif // PARECT_H
