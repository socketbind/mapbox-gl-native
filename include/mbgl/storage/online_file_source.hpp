#ifndef MBGL_STORAGE_ONLINE_FILE_SOURCE
#define MBGL_STORAGE_ONLINE_FILE_SOURCE

#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/file_cache.hpp>

namespace mbgl {

namespace util {
template <typename T> class Thread;
} // namespace util

class OnlineFileSource : public FileSource {
public:
    OnlineFileSource(FileCache *cache, const std::string &root = "");
    ~OnlineFileSource() override;

    void setAccessToken(const std::string& t) { accessToken = t; }
    std::string getAccessToken() const { return accessToken; }

    std::unique_ptr<FileRequest> requestStyle(const std::string& url, Callback) override;
    std::unique_ptr<FileRequest> requestSource(const std::string& url, Callback) override;
    std::unique_ptr<FileRequest> requestTile(const SourceInfo&, const TileID&, float pixelRatio, Callback) override;
    std::unique_ptr<FileRequest> requestGlyphs(const std::string& urlTemplate, const std::string& fontStack, const GlyphRange&, Callback) override;
    std::unique_ptr<FileRequest> requestSpriteJSON(const std::string& urlBase, float pixelRatio, Callback) override;
    std::unique_ptr<FileRequest> requestSpriteImage(const std::string& urlBase, float pixelRatio, Callback) override;

private:
    friend class OnlineFileRequest;

    std::unique_ptr<FileRequest> request(const std::string& url, Callback);
    void cancel(const std::string& url, FileRequest*);

    class Impl;
    const std::unique_ptr<util::Thread<Impl>> thread;
    std::string accessToken;
};

} // namespace mbgl

#endif
