// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_SINGLETON__
#define ONYX_FEED_READER_SINGLETON__

#include "onyx/base/base.h"

namespace onyx {
namespace feed_reader {

template <typename T>
class Singleton {
  public:
    static T& instance();

  private:
    NO_COPY_AND_ASSIGN(Singleton);
};

// Naive, non-thread-safe implementation.

template <typename T>
T& Singleton<T>::instance() {
    static T t;
    return t;
}

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_SINGLETON__
