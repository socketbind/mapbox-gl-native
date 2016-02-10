#ifndef MBGL_MAP_TILE_ID
#define MBGL_MAP_TILE_ID

#include <cstdint>
#include <cmath>
#include <string>
#include <functional>
#include <forward_list>
#include <limits>

namespace mbgl {

// Represents tile z/x/y coordinates.
class TileID {
public:
    const int16_t w = 0;
    const double z = 0;
    const double x = 0, y = 0;
    const double sourceZ;
    const double overscaling;

    inline explicit TileID(double z_, double x_, double y_, double sourceZ_ = 0)
        : w((x_ < 0 ? x_ - std::pow(2, z_) + 1 : x_) / std::pow(2, z_)), z(z_), x(x_), y(y_),
        sourceZ(sourceZ_), overscaling(std::pow(2, z_ - sourceZ_)) {}

    inline uint64_t to_uint64() const {
        return ((std::pow(2, z) * y + x) * 32) + z;
    }

    struct Hash {
        std::size_t operator()(const TileID& id) const {
            return std::hash<uint64_t>()(id.to_uint64());
        }
    };

    inline bool operator==(const TileID& rhs) const {
        return w == rhs.w && z == rhs.z && x == rhs.x && y == rhs.y;
    }

    inline bool operator!=(const TileID& rhs) const {
        return !operator==(rhs);
    }

    inline bool operator<(const TileID& rhs) const {
        if (w != rhs.w) return w < rhs.w;
        if (z != rhs.z) return z < rhs.z;
        if (x != rhs.x) return x < rhs.x;
        return y < rhs.y;
    }

    inline TileID zoomTo(double target) const {
        double scale = std::pow(2, target - z);
        return TileID { target, x * scale, y * scale };
    }

    TileID operator-(const TileID& rhs) const {
        TileID zoomed = rhs.zoomTo(z);
        return TileID { z, x - zoomed.x, y - zoomed.y };
    };

    TileID parent(double z, double sourceMaxZoom) const;
    TileID normalized() const;
    std::forward_list<TileID>
    children(double sourceMaxZoom = std::numeric_limits<double>::max()) const;
    bool isChildOf(const TileID&) const;
    operator std::string() const;

};

} // namespace mbgl

#endif
