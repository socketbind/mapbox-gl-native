#ifndef TEST_RESOURCES_STUB_FILE_SOURCE
#define TEST_RESOURCES_STUB_FILE_SOURCE

#include <mbgl/storage/file_source.hpp>
#include <mbgl/util/timer.hpp>

#include <unordered_map>

namespace mbgl {

class StubFileSource : public FileSource {
public:
    StubFileSource();
    ~StubFileSource() override;

    std::unique_ptr<FileRequest> requestStyle(const std::string& url, Callback) override;
    std::unique_ptr<FileRequest> requestSource(const std::string& url, Callback) override;
    std::unique_ptr<FileRequest> requestTile(const SourceInfo&, const TileID&, float pixelRatio, Callback) override;
    std::unique_ptr<FileRequest> requestGlyphs(const std::string& urlTemplate, const std::string& fontStack, const GlyphRange&, Callback) override;
    std::unique_ptr<FileRequest> requestSpriteJSON(const std::string& urlBase, float pixelRatio, Callback) override;
    std::unique_ptr<FileRequest> requestSpriteImage(const std::string& urlBase, float pixelRatio, Callback) override;

    std::function<Response (const std::string& url)> styleResponse;
    std::function<Response (const std::string& url)> sourceResponse;
    std::function<Response (const SourceInfo&, const TileID&, float pixelRatio)> tileResponse;
    std::function<Response (const std::string& urlTemplate, const std::string& fontStack, const GlyphRange&)> glyphsResponse;
    std::function<Response (const std::string& urlBase, float pixelRatio)> spriteJSONResponse;
    std::function<Response (const std::string& urlBase, float pixelRatio)> spriteImageResponse;

private:
    std::unique_ptr<FileRequest> respond(Response, Callback);

    friend class StubFileRequest;

    std::unordered_map<FileRequest*, std::pair<Response, Callback>> pending;
    util::Timer timer;
};

}

#endif
