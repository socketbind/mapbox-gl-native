#include <mbgl/storage/default_file_source.hpp>
#include <mbgl/storage/online_file_source.hpp>
#include <mbgl/map/tile_id.hpp>
#include <mbgl/map/source_info.hpp>

namespace mbgl {

class DefaultFileSource::Impl {
public:
    Impl(FileCache* cache, const std::string& root)
        : onlineFileSource(cache, root) {
    }

    OnlineFileSource onlineFileSource;
};

DefaultFileSource::DefaultFileSource(FileCache* cache, const std::string& root)
    : impl(std::make_unique<DefaultFileSource::Impl>(cache, root)) {
}

DefaultFileSource::~DefaultFileSource() = default;

void DefaultFileSource::setAccessToken(const std::string& accessToken) {
    impl->onlineFileSource.setAccessToken(accessToken);
}

std::string DefaultFileSource::getAccessToken() const {
    return impl->onlineFileSource.getAccessToken();
}

std::unique_ptr<FileRequest> DefaultFileSource::requestStyle(const std::string& url, Callback callback) {
    return impl->onlineFileSource.requestStyle(url, callback);
}

std::unique_ptr<FileRequest> DefaultFileSource::requestSource(const std::string& url, Callback callback) {
    return impl->onlineFileSource.requestSource(url, callback);
}

std::unique_ptr<FileRequest> DefaultFileSource::requestTile(const SourceInfo& source, const TileID& id, float pixelRatio, Callback callback) {
    return impl->onlineFileSource.requestTile(source, id, pixelRatio, callback);
}

std::unique_ptr<FileRequest> DefaultFileSource::requestGlyphs(const std::string& urlTemplate, const std::string& fontStack, const GlyphRange& glyphRange, Callback callback) {
    return impl->onlineFileSource.requestGlyphs(urlTemplate, fontStack, glyphRange, callback);
}

std::unique_ptr<FileRequest> DefaultFileSource::requestSpriteJSON(const std::string& urlBase, float pixelRatio, Callback callback) {
    return impl->onlineFileSource.requestSpriteJSON(urlBase, pixelRatio, callback);
}

std::unique_ptr<FileRequest> DefaultFileSource::requestSpriteImage(const std::string& urlBase, float pixelRatio, Callback callback) {
    return impl->onlineFileSource.requestSpriteImage(urlBase, pixelRatio, callback);
}

} // namespace mbgl
