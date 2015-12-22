#ifndef MBGL_STORAGE_DEFAULT_FILE_SOURCE
#define MBGL_STORAGE_DEFAULT_FILE_SOURCE

#include <mbgl/storage/file_source.hpp>

namespace mbgl {

class FileCache;

class DefaultFileSource : public FileSource {
public:
    DefaultFileSource(FileCache*, const std::string& root = "");
    ~DefaultFileSource() override;

    void setAccessToken(const std::string&);
    std::string getAccessToken() const;

    std::unique_ptr<FileRequest> requestStyle(const std::string& url, Callback) override;
    std::unique_ptr<FileRequest> requestSource(const std::string& url, Callback) override;
    std::unique_ptr<FileRequest> requestTile(const SourceInfo&, const TileID&, float pixelRatio, Callback) override;
    std::unique_ptr<FileRequest> requestGlyphs(const std::string& urlTemplate, const std::string& fontStack, const GlyphRange&, Callback) override;
    std::unique_ptr<FileRequest> requestSpriteJSON(const std::string& urlBase, float pixelRatio, Callback) override;
    std::unique_ptr<FileRequest> requestSpriteImage(const std::string& urlBase, float pixelRatio, Callback) override;

private:
    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace mbgl

#endif
