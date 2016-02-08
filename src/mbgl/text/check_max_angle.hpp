#ifndef MBGL_TEXT_CHECK_MAX_ANGLE
#define MBGL_TEXT_CHECK_MAX_ANGLE

#include <mbgl/geometry/anchor.hpp>
#include <mbgl/util/geo.hpp>

namespace mbgl {

bool checkMaxAngle(const Coordinates &line, Anchor &anchor, const float labelLength,
        const float windowSize, const float maxAngle);

} // namespace mbgl

#endif
