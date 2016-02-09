#ifndef MBGL_MAP_SOURCE_INFO
#define MBGL_MAP_SOURCE_INFO

#include <mbgl/style/types.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/vec.hpp>

#include <vector>
#include <string>

namespace mbgl {

class TileID;

class SourceInfo {
public:
    std::vector<std::string> tiles;
    double minZoom = util::MIN_ZOOM;
    double maxZoom = util::MAX_ZOOM;
    std::string attribution;
    vec3<> center = { 0, 0, 0 };
    vec4<> bounds = { -util::LONGITUDE_MAX, -util::LATITUDE_MAX, util::LONGITUDE_MAX, util::LATITUDE_MAX };
};

} // namespace mbgl

#endif // MBGL_MAP_SOURCE_INFO
