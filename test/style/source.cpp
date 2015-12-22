#include "../fixtures/util.hpp"
#include "../fixtures/mock_file_source.hpp"
#include "../fixtures/mock_view.hpp"
#include "../fixtures/stub_style_observer.hpp"

#include <mbgl/map/source.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/platform/log.hpp>

#include <mbgl/map/transform.hpp>
#include <mbgl/map/map_data.hpp>
#include <mbgl/util/worker.hpp>
#include <mbgl/util/texture_pool.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/style_update_parameters.hpp>
#include <mbgl/layer/line_layer.hpp>

using namespace mbgl;

class SourceTest {
public:
    util::ThreadContext context { "Map", util::ThreadType::Map, util::ThreadPriority::Regular };
    util::RunLoop loop;
    StubFileSource fileSource;
    StubStyleObserver observer;
    MockView view;
    Transform transform { view, ConstrainMode::HeightOnly };
    TransformState transformState;
    Worker worker { 1 };
    TexturePool texturePool;
    MapData mapData { MapMode::Still, GLContextMode::Unique, 1.0 };
    Style style { mapData };
    Source source;

    StyleUpdateParameters updateParameters {
        1.0,
        MapDebugOptions(),
        TimePoint(),
        transformState,
        worker,
        texturePool,
        true,
        MapMode::Continuous,
        mapData,
        style
    };

    SourceTest() {
        // Squelch logging.
        Log::setObserver(std::make_unique<Log::NullObserver>());

        util::ThreadContext::Set(&context);
        util::ThreadContext::setFileSource(&fileSource);

        source.setObserver(&observer);

        transform.resize({{ 512, 512 }});
        transform.setLatLngZoom({0, 0}, 0);

        transformState = transform.getState();
    }

    void run() {
        loop.run();
    }

    void end() {
        loop.stop();
    }
};

TEST(Source, LoadingFail) {
    SourceTest test;

    test.fileSource.sourceResponse = [&] (const std::string&) {
        Response response;
        response.error = std::make_unique<Response::Error>(
            Response::Error::Reason::Other,
            "Failed by the test case");
        return response;
    };

    test.observer.sourceError = [&] (Source& source, std::exception_ptr error) {
        EXPECT_EQ(source.info.url, "source");
        EXPECT_EQ(util::toString(error), "Failed by the test case");
        test.end();
    };

    test.source.info.url = "source";
    test.source.load();

    test.run();
}

TEST(Source, LoadingCorrupt) {
    SourceTest test;

    test.fileSource.sourceResponse = [&] (const std::string&) {
        Response response;
        response.data = std::make_unique<std::string>("CORRUPTED");
        return response;
    };

    test.observer.sourceError = [&] (Source& source, std::exception_ptr error) {
        EXPECT_EQ(source.info.url, "source");
        EXPECT_EQ(util::toString(error), "0 - Invalid value.");
        test.end();
    };

    test.source.info.url = "source";
    test.source.load();

    test.run();
}

TEST(Source, RasterTileFail) {
    SourceTest test;

    test.fileSource.tileResponse = [&] (const SourceInfo& source, const TileID& tileID, float pixelRatio) {
        EXPECT_EQ(source.type, SourceType::Raster);
        EXPECT_EQ(std::string(tileID), "0/0/0");
        EXPECT_EQ(pixelRatio, 1.0);
        Response response;
        response.error = std::make_unique<Response::Error>(
            Response::Error::Reason::Other,
            "Failed by the test case");
        return response;
    };

    test.observer.tileError = [&] (Source& source, const TileID& tileID, std::exception_ptr error) {
        EXPECT_EQ(source.info.type, SourceType::Raster);
        EXPECT_EQ(std::string(tileID), "0/0/0");
        EXPECT_EQ(util::toString(error), "Failed by the test case");
        test.end();
    };

    test.source.info.type = SourceType::Raster;
    test.source.info.tiles = { "tiles" };
    test.source.load();
    test.source.update(test.updateParameters);

    test.run();
}

TEST(Source, VectorTileFail) {
    SourceTest test;

    test.fileSource.tileResponse = [&] (const SourceInfo& source, const TileID& tileID, float pixelRatio) {
        EXPECT_EQ(source.type, SourceType::Vector);
        EXPECT_EQ(std::string(tileID), "0/0/0");
        EXPECT_EQ(pixelRatio, 1.0);
        Response response;
        response.error = std::make_unique<Response::Error>(
            Response::Error::Reason::Other,
            "Failed by the test case");
        return response;
    };

    test.observer.tileError = [&] (Source& source, const TileID& tileID, std::exception_ptr error) {
        EXPECT_EQ(source.info.type, SourceType::Vector);
        EXPECT_EQ(std::string(tileID), "0/0/0");
        EXPECT_EQ(util::toString(error), "Failed by the test case");
        test.end();
    };

    test.source.info.type = SourceType::Vector;
    test.source.info.tiles = { "tiles" };
    test.source.load();
    test.source.update(test.updateParameters);

    test.run();
}

TEST(Source, RasterTileCorrupt) {
    SourceTest test;

    test.fileSource.tileResponse = [&] (const SourceInfo& source, const TileID& tileID, float pixelRatio) {
        EXPECT_EQ(source.type, SourceType::Raster);
        EXPECT_EQ(std::string(tileID), "0/0/0");
        EXPECT_EQ(pixelRatio, 1.0);
        Response response;
        response.data = std::make_unique<std::string>("CORRUPTED");
        return response;
    };

    test.observer.tileError = [&] (Source& source, const TileID& tileID, std::exception_ptr error) {
        EXPECT_EQ(source.info.type, SourceType::Raster);
        EXPECT_EQ(std::string(tileID), "0/0/0");
        EXPECT_TRUE(bool(error));
        // Not asserting on platform-specific error text.
        test.end();
    };

    test.source.info.type = SourceType::Raster;
    test.source.info.tiles = { "tiles" };
    test.source.load();
    test.source.update(test.updateParameters);

    test.run();
}

TEST(Source, VectorTileCorrupt) {
    SourceTest test;

    test.fileSource.tileResponse = [&] (const SourceInfo& source, const TileID& tileID, float pixelRatio) {
        EXPECT_EQ(source.type, SourceType::Vector);
        EXPECT_EQ(std::string(tileID), "0/0/0");
        EXPECT_EQ(pixelRatio, 1.0);
        Response response;
        response.data = std::make_unique<std::string>("CORRUPTED");
        return response;
    };

    test.observer.tileError = [&] (Source& source, const TileID& tileID, std::exception_ptr error) {
        EXPECT_EQ(source.info.type, SourceType::Vector);
        EXPECT_EQ(std::string(tileID), "0/0/0");
        EXPECT_EQ(util::toString(error), "pbf unknown field type exception");
        test.end();
    };

    // Need to have at least one layer that uses the source.
    test.source.info.source_id = "vector";
    auto layer = std::make_unique<LineLayer>();
    layer->source = "vector";
    layer->sourceLayer = "water";
    test.style.addLayer(std::move(layer));

    test.source.info.type = SourceType::Vector;
    test.source.info.tiles = { "tiles" };
    test.source.load();
    test.source.update(test.updateParameters);

    test.run();
}

TEST(Source, RasterTileCancel) {
    SourceTest test;

    test.fileSource.tileResponse = [&] (const SourceInfo&, const TileID&, float) {
        test.end();
        return Response();
    };

    test.observer.tileLoaded = [&] (Source&, const TileID&, bool) {
        FAIL() << "Should never be called";
    };

    test.observer.tileError = [&] (Source&, const TileID&, std::exception_ptr) {
        FAIL() << "Should never be called";
    };

    test.source.info.type = SourceType::Raster;
    test.source.info.tiles = { "tiles" };
    test.source.load();
    test.source.update(test.updateParameters);

    test.run();
}

TEST(Source, VectorTileCancel) {
    SourceTest test;

    test.fileSource.tileResponse = [&] (const SourceInfo&, const TileID&, float) {
        test.end();
        return Response();
    };

    test.observer.tileLoaded = [&] (Source&, const TileID&, bool) {
        FAIL() << "Should never be called";
    };

    test.observer.tileError = [&] (Source&, const TileID&, std::exception_ptr) {
        FAIL() << "Should never be called";
    };

    test.source.info.type = SourceType::Vector;
    test.source.info.tiles = { "tiles" };
    test.source.load();
    test.source.update(test.updateParameters);

    test.run();
}
