#include "../fixtures/util.hpp"
#include "../fixtures/mock_file_source.hpp"
#include "../fixtures/stub_style_observer.hpp"

#include <mbgl/text/font_stack.hpp>
#include <mbgl/text/glyph_store.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/platform/log.hpp>

using namespace mbgl;

class GlyphStoreTest {
public:
    util::ThreadContext context { "Map", util::ThreadType::Map, util::ThreadPriority::Regular };
    util::RunLoop loop;
    StubFileSource fileSource;
    StubStyleObserver observer;
    GlyphStore glyphStore;

    void run(const std::string& url, const std::string& fontStack, const std::set<GlyphRange>& glyphRanges) {
        // Squelch logging.
        Log::setObserver(std::make_unique<Log::NullObserver>());

        util::ThreadContext::Set(&context);
        util::ThreadContext::setFileSource(&fileSource);

        glyphStore.setObserver(&observer);
        glyphStore.setURL(url);
        glyphStore.hasGlyphRanges(fontStack, glyphRanges);

        loop.run();
    }

    void end() {
        loop.stop();
    }
};

TEST(GlyphStore, LoadingSuccess) {
    GlyphStoreTest test;

    test.fileSource.glyphsResponse = [&] (const std::string& url,
                                          const std::string& fontStack,
                                          const GlyphRange&) {
        EXPECT_EQ("test/fixtures/resources/glyphs.pbf", url);
        EXPECT_EQ("Test Stack", fontStack);
        Response response;
        response.data = std::make_shared<std::string>(util::read_file(url));
        return response;
    };

    test.observer.glyphsError = [&] (const std::string&, const GlyphRange&, std::exception_ptr) {
        FAIL();
        test.end();
    };

    test.observer.glyphsLoaded = [&] (const std::string&, const GlyphRange&) {
        if (!test.glyphStore.hasGlyphRanges("Test Stack", {{0, 255}, {256, 511}}))
            return;

        auto fontStack = test.glyphStore.getFontStack("Test Stack");
        EXPECT_FALSE(fontStack->getMetrics().empty());
        EXPECT_FALSE(fontStack->getSDFs().empty());

        test.end();
    };

    test.run(
        "test/fixtures/resources/glyphs.pbf",
        "Test Stack",
        {{0, 255}, {256, 511}});
}

TEST(GlyphStore, LoadingFail) {
    GlyphStoreTest test;

    test.fileSource.glyphsResponse = [&] (const std::string&,
                                          const std::string&,
                                          const GlyphRange&) {
        Response response;
        response.error = std::make_unique<Response::Error>(
            Response::Error::Reason::Other,
            "Failed by the test case");
        return response;
    };

    test.observer.glyphsError = [&] (const std::string& fontStack, const GlyphRange& glyphRange, std::exception_ptr error) {
        EXPECT_EQ(fontStack, "Test Stack");
        EXPECT_EQ(glyphRange, GlyphRange(0, 255));

        EXPECT_TRUE(error != nullptr);
        EXPECT_EQ(util::toString(error), "Failed by the test case");

        auto stack = test.glyphStore.getFontStack("Test Stack");
        EXPECT_TRUE(stack->getMetrics().empty());
        EXPECT_TRUE(stack->getSDFs().empty());
        EXPECT_FALSE(test.glyphStore.hasGlyphRanges("Test Stack", {{0, 255}}));

        test.end();
    };

    test.run(
        "test/fixtures/resources/glyphs.pbf",
        "Test Stack",
        {{0, 255}});
}

TEST(GlyphStore, LoadingCorrupted) {
    GlyphStoreTest test;

    test.fileSource.glyphsResponse = [&] (const std::string&,
                                          const std::string&,
                                          const GlyphRange&) {
        Response response;
        response.data = std::make_unique<std::string>("CORRUPTED");
        return response;
    };

    test.observer.glyphsError = [&] (const std::string& fontStack, const GlyphRange& glyphRange, std::exception_ptr error) {
        EXPECT_EQ(fontStack, "Test Stack");
        EXPECT_EQ(glyphRange, GlyphRange(0, 255));

        EXPECT_TRUE(error != nullptr);
        EXPECT_EQ(util::toString(error), "pbf unknown field type exception");

        auto stack = test.glyphStore.getFontStack("Test Stack");
        EXPECT_TRUE(stack->getMetrics().empty());
        EXPECT_TRUE(stack->getSDFs().empty());
        EXPECT_FALSE(test.glyphStore.hasGlyphRanges("Test Stack", {{0, 255}}));

        test.end();
    };

    test.run(
        "test/fixtures/resources/glyphs.pbf",
        "Test Stack",
        {{0, 255}});
}

TEST(GlyphStore, LoadingCancel) {
    GlyphStoreTest test;

    test.fileSource.glyphsResponse = [&] (const std::string&,
                                          const std::string&,
                                          const GlyphRange&) {
        test.end();
        return Response();
    };

    test.observer.glyphsLoaded = [&] (const std::string&, const GlyphRange&) {
        FAIL() << "Should never be called";
    };

    test.run(
        "test/fixtures/resources/glyphs.pbf",
        "Test Stack",
        {{0, 255}});
}
