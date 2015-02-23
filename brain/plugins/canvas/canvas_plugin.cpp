#include "../../Platform.hpp"
#include "BrainPlugins.hpp"
#include <graphics/init.hpp>
#include <graphics/canvas.hpp>


namespace Elysia {
std::shared_ptr<Polarity::Canvas> MakeGraphicsSystem(const std::string &renderer_type, int w, int h) {
    Polarity::initGraphicsSystem();
    std::shared_ptr<Polarity::Canvas> retval(Polarity::makeGraphicsCanvas(Polarity::createAsyncIoTask(),
                                                                          renderer_type.c_str(),
                                                                          w, h));
    return retval;
}
}
static int core_plugin_refcount = 0;

ELYSIA_PLUGIN_EXPORT_C void init() {
    core_plugin_refcount++;
    Elysia::BrainPlugins::registerGraphicsPlugin("canvas",&Elysia::MakeGraphicsSystem);
}

ELYSIA_PLUGIN_EXPORT_C void destroy() {
    Elysia::BrainPlugins::shutdownGraphics();
    Polarity::destroyGraphicsSystem();
    core_plugin_refcount--;
}

ELYSIA_PLUGIN_EXPORT_C const char* name() {
    return "canvas";
}

ELYSIA_PLUGIN_EXPORT_C int refcount() {
    return core_plugin_refcount;
}
