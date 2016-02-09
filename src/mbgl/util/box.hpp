#ifndef MBGL_UTIL_BOX
#define MBGL_UTIL_BOX

#include <mbgl/map/tile_id.hpp>

namespace mbgl {

struct box {
    box(const TileID& tl_, const TileID& tr_, const TileID& br_, const TileID& bl_) :
        tl(tl_), tr(tr_), br(br_), bl(bl_) {}
    TileID tl, tr, br, bl;
};

} // namespace mbgl

#endif
