#include "mock_file_source.hpp"

namespace mbgl {

class StubFileRequest : public FileRequest {
public:
    StubFileRequest(StubFileSource& fileSource_)
        : fileSource(fileSource_) {
    }

    ~StubFileRequest() {
        fileSource.pending.erase(this);
    }

    StubFileSource& fileSource;
};

StubFileSource::StubFileSource() {
    timer.unref();
    timer.start(std::chrono::milliseconds(10), std::chrono::milliseconds(10), [this] {
        // Explicit move to avoid iterator invalidation if ~StubFileRequest gets called within the loop.
        auto pending_ = std::move(pending);
        for (auto& pair : pending_) {
            pair.second.second(pair.second.first);
        }
    });
}

StubFileSource::~StubFileSource() {
    timer.stop();
}

std::unique_ptr<FileRequest> StubFileSource::requestStyle(const std::string& url, Callback callback) {
    return respond(styleResponse(url), callback);
}

std::unique_ptr<FileRequest> StubFileSource::requestSource(const std::string& url, Callback callback) {
    return respond(sourceResponse(url), callback);
}

std::unique_ptr<FileRequest> StubFileSource::requestTile(const SourceInfo& source, const TileID& tileID,
                                                         float pixelRatio, Callback callback) {
    return respond(tileResponse(source, tileID, pixelRatio), callback);
}

std::unique_ptr<FileRequest> StubFileSource::requestGlyphs(const std::string& urlTemplate, const std::string& fontStack,
                                                           const GlyphRange& glyphRange, Callback callback) {
    return respond(glyphsResponse(urlTemplate, fontStack, glyphRange), callback);
}

std::unique_ptr<FileRequest> StubFileSource::requestSpriteJSON(const std::string& urlBase, float pixelRatio,
                                                               Callback callback) {
    return respond(spriteJSONResponse(urlBase, pixelRatio), callback);
}

std::unique_ptr<FileRequest> StubFileSource::requestSpriteImage(const std::string& urlBase, float pixelRatio,
                                                                Callback callback) {
    return respond(spriteImageResponse(urlBase, pixelRatio), callback);
}

std::unique_ptr<FileRequest> StubFileSource::respond(Response response, Callback callback) {
    auto req = std::make_unique<StubFileRequest>(*this);
    pending.emplace(req.get(), std::make_pair(response, callback));
    return std::move(req);
}

} // namespace mbgl
