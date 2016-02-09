#include <mbgl/map/tile_id.hpp>
#include <mbgl/util/string.hpp>

#include <cassert>

namespace mbgl {

TileID TileID::parent(double parent_z, double sourceMaxZoom) const {
    assert(parent_z < z);
    auto newX = x;
    auto newY = y;
    for (auto newZ = z; newZ > parent_z; newZ--) {
        if (newZ > sourceMaxZoom) {
            // the id represents an overscaled tile, return the same coordinates with a lower z
            // do nothing
        } else {
            newX = newX / 2;
            newY = newY / 2;
        }
    }

    return TileID{parent_z, newX, newY, parent_z > sourceMaxZoom ? sourceMaxZoom : parent_z};
}

std::forward_list<TileID> TileID::children(double sourceMaxZoom) const {
    auto childZ = z + 1;

    std::forward_list<TileID> child_ids;
    if (z >= sourceMaxZoom) {
        // return a single tile id representing a an overscaled tile
        child_ids.emplace_front(childZ, x, y, sourceMaxZoom);

    } else {
        auto childX = x * 2;
        auto childY = y * 2;
        child_ids.emplace_front(childZ, childX, childY, childZ);
        child_ids.emplace_front(childZ, childX + 1, childY, childZ);
        child_ids.emplace_front(childZ, childX, childY + 1, childZ);
        child_ids.emplace_front(childZ, childX + 1, childY + 1, childZ);
    }

    return child_ids;
}

TileID TileID::normalized() const {
    double dim = std::pow(2, sourceZ);
    double nx = x, ny = y;
    while (nx < 0) nx += dim;
    while (nx >= dim) nx -= dim;
    return TileID { z, nx, ny, sourceZ};
}

bool TileID::isChildOf(const TileID &parent_id) const {
    if (parent_id.z >= z || parent_id.w != w) {
        return false;
    }
    double scale = std::pow(2, z - parent_id.z);
    return parent_id.x == std::floor(x / scale) &&
           parent_id.y == std::floor(y / scale);
}

TileID::operator std::string() const {
    return util::toString(z) + "/" + util::toString(x) + "/" + util::toString(y);
}

} // namespace mbgl
