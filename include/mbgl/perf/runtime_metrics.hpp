#pragma once

#define MBGL_COLLECT_RUNTIME_METRICS

#ifndef MBGL_COLLECT_RUNTIME_METRICS
#define MBGL_TRACE(x)
#define MBGL_IF_TRACING(x)
#else
#include <mutex>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/logging.hpp>
#include <vector>

#define MBGL_TRACE(x) mbgl::util::Tracer::get().x
#define MBGL_IF_TRACING(x) x

//#define MBGL_TRACER_USE_MUTEX
#ifndef MBGL_TRACER_USE_MUTEX
#define LOCK()
#define UNLOCK()
#else
#define LOCK() std::unique_lock<std::mutex> locker(mutex);
#define UNLOCK() locker.unlock();
#endif

#define NS2MS(x) ((x) * 0.000001)

namespace mbgl {
namespace util {

#define STRUCT_BEGIN(name) \
    struct name \
    { \

#define STRUCT_END() \
    };

#define STRUCT_MEMBER_UINT64(name, initialValue)   uint64_t name = initialValue;

#include <mbgl/perf/trace_data.in>

#undef STRUCT_BEGIN
#undef STRUCT_END
#undef STRUCT_MEMBER_UINT64

class TracerImplBase
{
public:
    static uint64_t nanoseconds() {
        return Clock::now().time_since_epoch().count();
    }
    TracerImplBase() {}
    mbgl::util::TraceData data;
    bool collectData = false;
    inline bool isCollectingData() const {
        return collectData;
    }
    inline void setCollectData(bool enabled) {
        collectData = enabled;
    }

#define STRUCT_BEGIN(name) \
    static void tracing_frameReset(name &data) \
    { \

#define STRUCT_END() \
    };

#define STRUCT_MEMBER_UINT64(name, initialValue)  /*if (frameReset)*/ data.name = initialValue; \

#include <mbgl/perf/trace_data.in>

#undef STRUCT_BEGIN
#undef STRUCT_END
#undef STRUCT_MEMBER_UINT64

#define STRUCT_BEGIN(name) \
    static std::string toJson(const name &data) { \
        std::string res; \
        res += "{\n"; \

#define STRUCT_MEMBER_UINT64(name, initialValue)  res += std::string("\"") + std::string(#name) + std::string("\" : ") + std::to_string(data.name) + std::string(",");

#define STRUCT_END() \
        res.pop_back(); res += "}"; \
        return res; \
    }; \

#include <mbgl/perf/trace_data.in>

// Ignore fromJson for now, not currently needed here.

#undef STRUCT_BEGIN
#undef STRUCT_END
#undef STRUCT_MEMBER_UINT64

/*
    void updateStarts () {}
    void renderStarts () {}
    void prepareStarts () {}
    void renderImplStarts () {}
    void uploadStarts () {}
    void drawStarts () {}
    void extrusionEnds () {}
    void clearEnds () {}
    void opaqueEnds () {}
    void translucentEnds () {}
    void debugEnds () {}
    void renderEnds () {}
    void imageSourceRenderDataStarts () {}
    void imageSourceRenderDataEnds () {}
    void layerRenderItemStarts () {}
    void layerRenderItemEnds () {}
    void tileSourceRenderItemStarts () {}
    void tileSourceRenderItemEnds () {}
    void fillStarts () {}
    void fillExtrusionsStarts () {}
    void lineStarts () {}
    void circleStarts () {}
    void heatmapStarts () {}
    void backgroundStarts () {}
    void rasterStarts () {}
    void symbolStarts () {}
    void hillshadeStarts () {}
    void customStarts () {}
    void fillEnds () {}
    void fillExtrusionsEnds () {}
    void lineEnds () {}
    void circleEnds () {}
    void heatmapEnds () {}
    void backgroundEnds () {}
    void rasterEnds () {}
    void symbolEnds () {}
    void hillshadeEnds () {}
    void customEnds () {}
    void appendFrame () {}
    void clearFrames () {}
*/
};

// Instrumentation-less tracing implementation, possibly requiring mutexing
class TracerImplGeneric : public TracerImplBase
{
public:
    std::mutex mutex; // No need recursive. But possibly split into multiple mutexes, one per thread/task type, e.g., tile decoding, resource provisioning, etc.
                      // that will be locked only on consumption

    TracerImplGeneric() : TracerImplBase() {}
    std::vector<mbgl::util::TraceData> frameList;


    // Instrumentation-less tracing implementation, requiring mutexing
    inline void updateStarts() { LOCK(); tracing_frameReset(data); data.renderer_tsStartUpdate = nanoseconds(); }
    inline void renderStarts() { LOCK(); data.renderer_tsStartFrame = nanoseconds(); }
    inline void prepareStarts() { LOCK(); data.renderer_tsStartPrepare = nanoseconds(); }
    inline void renderImplStarts() { LOCK(); data.renderer_tsStartRenderImpl = nanoseconds(); }
    inline void uploadStarts() { LOCK(); data.renderer_tsStartUpload = nanoseconds(); }
    inline void drawStarts() { LOCK(); data.renderer_tsStartDraw = nanoseconds(); }
    inline void extrusionEnds() { LOCK(); data.renderer_tsEndExtrusion = nanoseconds(); }
    inline void clearEnds() { LOCK(); data.renderer_tsEndClear = nanoseconds(); }
    inline void opaqueEnds() { LOCK(); data.renderer_tsEndOpaque = nanoseconds(); }
    inline void translucentEnds() { LOCK(); data.renderer_tsEndTranslucent = nanoseconds(); }
    inline void debugEnds() { LOCK(); data.renderer_tsEndDebug = nanoseconds(); }
    inline void renderEnds() { LOCK(); data.renderer_tsEndRender = nanoseconds(); appendFrame(); }

    inline void imageSourceRenderDataStarts() { LOCK(); data.renderer_tsStartImageSourceRenderData = nanoseconds(); }
    inline void imageSourceRenderDataEnds() { LOCK(); data.renderer_totalImageSourceRenderData += nanoseconds() - data.renderer_tsStartImageSourceRenderData; }
    inline void layerRenderItemStarts() { LOCK(); data.renderer_tsStartLayerRenderItem = nanoseconds(); }
    inline void layerRenderItemEnds() { LOCK(); data.renderer_totalLayerRenderItem += nanoseconds() - data.renderer_tsStartLayerRenderItem; }
    inline void tileSourceRenderItemStarts() { LOCK(); data.renderer_tsStartTileSourceRenderItem = nanoseconds(); }
    inline void tileSourceRenderItemEnds() { LOCK(); data.renderer_totalTileSourceRenderItem += nanoseconds() - data.renderer_tsStartTileSourceRenderItem; }

    inline void fillStarts() { LOCK(); data.layers_tsStartFills = nanoseconds(); }
    inline void fillExtrusionsStarts() { LOCK(); data.layers_tsStartFillExtrusions = nanoseconds(); }
    inline void lineStarts() { LOCK(); data.layers_tsStartLines = nanoseconds(); }
    inline void circleStarts() { LOCK(); data.layers_tsStartCircles = nanoseconds(); }
    inline void heatmapStarts() { LOCK(); data.layers_tsStartHeatmaps = nanoseconds(); }
    inline void backgroundStarts() { LOCK(); data.layers_tsStartBackgrounds = nanoseconds(); }
    inline void rasterStarts() { LOCK(); data.layers_tsStartRasters = nanoseconds(); }
    inline void symbolStarts() { LOCK(); data.layers_tsStartSymbols = nanoseconds(); }
    inline void hillshadeStarts() { LOCK(); data.layers_tsStartHillshades = nanoseconds(); }
    inline void customStarts() { LOCK(); data.layers_tsStartCustoms = nanoseconds(); }

    inline void fillEnds() { LOCK(); data.layers_totalFills += nanoseconds() - data.layers_tsStartFills; data.layers_numFills += 1; }
    inline void fillExtrusionsEnds() { LOCK(); data.layers_totalFillExtrusions += nanoseconds() - data.layers_tsStartFillExtrusions; data.layers_numFillExtrusions += 1; }
    inline void lineEnds() { LOCK(); data.layers_totalLines += nanoseconds() - data.layers_tsStartLines; data.layers_numLines += 1; }
    inline void circleEnds() { LOCK(); data.layers_totalCircles += nanoseconds() - data.layers_tsStartCircles; data.layers_numCircles += 1; }
    inline void heatmapEnds() { LOCK(); data.layers_totalHeatmaps += nanoseconds() - data.layers_tsStartHeatmaps; data.layers_numHeatmaps += 1; }
    inline void backgroundEnds() { LOCK(); data.layers_totalBackgrounds += nanoseconds() - data.layers_tsStartBackgrounds; data.layers_numBackgrounds += 1; }
    inline void rasterEnds() { LOCK(); data.layers_totalRasters += nanoseconds() - data.layers_tsStartRasters; data.layers_numRasters += 1; }
    inline void symbolEnds() { LOCK(); data.layers_totalSymbols += nanoseconds() - data.layers_tsStartSymbols; data.layers_numSymbols += 1; }
    inline void hillshadeEnds() { LOCK(); data.layers_totalHillshades += nanoseconds() - data.layers_tsStartHillshades; data.layers_numHillshades += 1; }
    inline void customEnds() { LOCK(); data.layers_totalCustoms += nanoseconds() - data.layers_tsStartCustoms; data.layers_numCustoms += 1; }
    inline void appendFrame() {
        LOCK();
        if (collectData)
            frameList.push_back(data);
    }
    inline const std::vector<mbgl::util::TraceData>& frames() {
        LOCK();
        return frameList;
    }

    inline std::string dumpJson(bool steal = false) const {
        LOCK();
        std::vector<mbgl::util::TraceData> frameList_;
        if (steal)
            frameList_ = std::move(frameList);
        else
            frameList_ = frameList;
        UNLOCK();

        std::string res;
        res += "[\n";
        for (const auto &f: frameList)
            res += toJson(f) + std::string(",");
        res.pop_back();
        res += "]\n";
        return res;
    }

    inline void clearFrames() {
        LOCK();
        frameList.clear();
    }
};

template<class T>
class TracerBase
{
    public:

        T impl;

        static TracerBase& get() {
            static TracerBase<T> i; // Guaranteed to be destroyed.
                                // Instantiated on first use.
            return i;
        }
    private:
        TracerBase() {}

    public:
        TracerBase(TracerBase<T> const&)               = delete;
        void operator=(TracerBase<T> const&)           = delete;

        bool isCollectingData() const {
            return impl.isCollectingData();
        }
        void setCollectData(bool enabled) {
            impl.setCollectData(enabled);
        }

        // This must be called at the beginning of updating the map view. See GLFWRendererFrontend::render
        void updateStarts() {
            impl.updateStarts();
        }
        void renderStarts() {
            impl.renderStarts();
        }
        void prepareStarts() {
            impl.prepareStarts();
        }
        void renderImplStarts() {
            impl.renderImplStarts();
        }
        void uploadStarts() {
            impl.uploadStarts();
        }
        void drawStarts() {
            impl.drawStarts();
        }
        void extrusionEnds() {
            impl.extrusionEnds();
        }
        void clearEnds() {
            impl.clearEnds();
        }
        void opaqueEnds() {
            impl.opaqueEnds();
        }
        void translucentEnds() {
            impl.translucentEnds();
        }
        void debugEnds() {
            impl.debugEnds();
        }
        void renderEnds() { // Currently the same as debug ends
            impl.renderEnds();
        }

        // Render Items
        void imageSourceRenderDataStarts() {
            impl.imageSourceRenderDataStarts();
        }
        void imageSourceRenderDataEnds() {
            impl.imageSourceRenderDataEnds();
        }
        void layerRenderItemStarts() {
            impl.layerRenderItemStarts();
        }
        void layerRenderItemEnds() {
            impl.layerRenderItemEnds();
        }
        void tileSourceRenderItemStarts() {
            impl.tileSourceRenderItemStarts();
        }
        void tileSourceRenderItemEnds() {
            impl.tileSourceRenderItemEnds();
        }

        // Layers
        void fillStarts() {
            impl.fillStarts();
        }
        void fillExtrusionsStarts() {
            impl.fillExtrusionsStarts();
        }
        void lineStarts() {
            impl.lineStarts();
        }
        void circleStarts() {
            impl.circleStarts();
        }
        void heatmapStarts() {
            impl.heatmapStarts();
        }
        void backgroundStarts() {
            impl.backgroundStarts();
        }
        void rasterStarts() {
            impl.rasterStarts();
        }
        void symbolStarts() {
            impl.symbolStarts();
        }
        void hillshadeStarts() {
            impl.hillshadeStarts();
        }
        void customStarts() {
            impl.customStarts();
        }
        // Layer ends
        void fillEnds() {
            impl.fillEnds();
        }
        void fillExtrusionsEnds() {
            impl.fillExtrusionsEnds();
        }
        void lineEnds() {
            impl.lineEnds();
        }
        void circleEnds() {
            impl.circleEnds();
        }
        void heatmapEnds() {
            impl.heatmapEnds();
        }
        void backgroundEnds() {
            impl.backgroundEnds();
        }
        void rasterEnds() {
            impl.rasterEnds();
        }
        void symbolEnds() {
            impl.symbolEnds();
        }
        void hillshadeEnds() {
            impl.hillshadeEnds();
        }
        void customEnds() {
            impl.customEnds();
        }

        const std::vector<mbgl::util::TraceData>& frames() {
            return impl.frames();
        }

        std::string dumpJson(bool steal = false) const {
            return impl.dumpJson(steal);
        }

        void clearFrames() {
            impl.clearFrames();
        }
};

typedef TracerBase<TracerImplGeneric> Tracer;

} // namespace util
} // namespace mbgl
#endif
