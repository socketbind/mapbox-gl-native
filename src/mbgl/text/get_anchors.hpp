#ifndef MBGL_TEXT_GETANCHORS
#define MBGL_TEXT_GETANCHORS

#include <mbgl/geometry/anchor.hpp>
#include <mbgl/util/geo.hpp>

namespace mbgl {

Anchors getAnchors(const Coordinates &line, float spacing,
        const float maxAngle, const float textLeft, const float textRight,
        const float iconLeft, const float iconRight,
        const float glyphSize, const float boxScale, const float overscaling);
} // namespace mbgl

#endif
