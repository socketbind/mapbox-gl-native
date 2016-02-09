#include <mbgl/annotation/point_annotation_impl.hpp>
#include <mbgl/annotation/annotation_tile.hpp>

namespace mbgl {

PointAnnotationImpl::PointAnnotationImpl(const AnnotationID id_, const PointAnnotation& point_)
: id(id_),
  point(point_) {
}

void PointAnnotationImpl::updateLayer(const TileID& tileID, AnnotationTileLayer& layer) const {
    std::unordered_map<std::string, std::string> featureProperties;
    featureProperties.emplace("sprite", point.icon.empty() ? std::string("default_marker") : point.icon);

    const uint16_t extent = 4096;
    const mbgl::PrecisionPoint pp = point.position.project();
    const auto scale = std::pow(2, tileID.z);
    const auto x = std::floor(pp.x * scale);
    const auto y = std::floor(pp.y * scale);
    const Coordinate coordinate(extent * (pp.x * scale - x), extent * (pp.y * scale - y));

    layer.features.emplace_back(
        std::make_shared<const AnnotationTileFeature>(FeatureType::Point,
                                                      GeometryCollection {{ {{ coordinate }} }},
                                                      featureProperties));
}

} // namespace mbgl
