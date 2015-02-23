#ifndef _LOADED_BRAIN_PLUGINS_HPP_
#define _LOADED_BRAIN_PLUGINS_HPP_
namespace Polarity {
class Canvas;
}

namespace Elysia{
class Brain;
class BrainPlugin;
class BRAIN_CORE_EXPORT BrainPlugins {
    std::map<std::string,std::function<BrainPlugin*(Brain*)> > mPlugins;
    typedef std::function<std::shared_ptr<Polarity::Canvas>(const std::string &renderer_type, int width, int height)> CanvasConstructor;
    std::map<std::string,CanvasConstructor> mCanvasPlugins;
    std::shared_ptr<Polarity::Canvas> mCanvasSingleton;
public:
    static void registerBrainPlugin(const std::string&name, const std::function<BrainPlugin*(Brain*)>&constructor);
    static void registerGraphicsPlugin(const std::string&name, const CanvasConstructor&constructor);
    static BrainPlugin*construct(const std::string &pluginName, Brain*);
    static std::vector<BrainPlugin*>constructAll(Brain*);

    static std::shared_ptr<Polarity::Canvas>&constructCanvasOrGetLast(const std::string &pluginName,
                                                                      const std::string& renderer_type, int width, int height);
    static std::shared_ptr<Polarity::Canvas>& returnConstructedCanvas();
    static void shutdownGraphics();
};
}

#endif
