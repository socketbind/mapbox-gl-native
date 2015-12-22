#ifndef MBGL_STORAGE_FILE_SOURCE
#define MBGL_STORAGE_FILE_SOURCE

#include <mbgl/storage/response.hpp>
#include <mbgl/util/noncopyable.hpp>

#include <functional>
#include <memory>
#include <string>

namespace mbgl {

class SourceInfo;
class TileID;
typedef std::pair<uint16_t, uint16_t> GlyphRange;

class FileRequest : private util::noncopyable {
public:
    virtual ~FileRequest() = default;
};

class FileSource : private util::noncopyable {
public:
    virtual ~FileSource() = default;

    using Callback = std::function<void (Response)>;

    // Request a resource. The callback will be called asynchronously, in the same
    // thread as the request was made. This thread must have an active RunLoop. The
    // request may be cancelled before completion by releasing the returned FileRequest.
    // If the request is cancelled before the callback is executed, the callback will
    // not be executed.

    virtual std::unique_ptr<FileRequest> requestStyle(const std::string& url, Callback) = 0;
    virtual std::unique_ptr<FileRequest> requestSource(const std::string& url, Callback) = 0;
    virtual std::unique_ptr<FileRequest> requestTile(const SourceInfo&, const TileID&, float pixelRatio, Callback) = 0;
    virtual std::unique_ptr<FileRequest> requestGlyphs(const std::string& urlTemplate, const std::string& fontStack, const GlyphRange&, Callback) = 0;
    virtual std::unique_ptr<FileRequest> requestSpriteJSON(const std::string& urlBase, float pixelRatio, Callback) = 0;
    virtual std::unique_ptr<FileRequest> requestSpriteImage(const std::string& urlBase, float pixelRatio, Callback) = 0;
};

} // namespace mbgl

#endif
