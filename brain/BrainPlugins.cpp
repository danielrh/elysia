#include "Platform.hpp"
#include "BrainPlugins.hpp"
using Polarity::Canvas;
namespace Elysia {
class BrainPlugin;
static BrainPlugins gBrainPlugins;

/**
 *	@param const std::string &name - name of the plugin
 *	@param const std::function<BrainPlugin*(Brain*)> &constructor - brain constructor
 *
 *	Description:	Registers a (new) brain plugin
**/
void BrainPlugins::registerBrainPlugin(const std::string&name, 
                                  const std::function<BrainPlugin*(Brain*)>&constructor){
    gBrainPlugins.mPlugins[name]=constructor;
}

/**
 *	@param const std::string &name - name of the plugin
 *	@param const CanvasConstructor - the funtion that takes a renderer config param, a width and a height and returns a Canvas
 *
 *	Description:	Registers a (new) brain plugin
**/
void BrainPlugins::registerGraphicsPlugin(const std::string&name, 
                                       const CanvasConstructor&constructor){
    gBrainPlugins.mCanvasPlugins[name]=constructor;
}

/**
 *	@param Brain *b - pointer to some brain for which you want to construct plugins
 *	@returns a vector of pointers to brain plugins
 *
 *	Description:	Constructs all plugins for this brain
**/
std::vector<BrainPlugin *>BrainPlugins::constructAll(Brain*b) {
    std::vector<BrainPlugin*> retval;
    for (std::map<std::string,std::function<BrainPlugin*(Brain*)> >::iterator i=gBrainPlugins.mPlugins.begin(),ie=gBrainPlugins.mPlugins.end();i!=ie;++i) {
        retval.push_back(i->second(b));
    }
    return retval;
}



/**
 *	@param const std::string &name - plugin name
 *	@param Brain *b - pointer to a brain object
 *	@returns the call to the constructed plugin in question, instantiated with the Brain as an argument
 *
 *	Description:	BrainPlugin constructor
**/
BrainPlugin *BrainPlugins::construct(const std::string &name, Brain *b) {
    std::map<std::string,std::function<BrainPlugin*(Brain*)> >::iterator i=gBrainPlugins.mPlugins.find(name);
    if (i!=gBrainPlugins.mPlugins.end()){
        return i->second(b);
    }
    return NULL;
}

/**
 *	@param const std::string &name - plugin name -- blank for first item in the std::map
 *	@param const std::string &renderer_type - information for the graphics system to initialize
 *	@param int width -- the horizontal size of the window
 *  @param int height -- the vertical size of the window
 *
 *	Description:	function to get the currently constructed graphics system, or to make one
**/
std::shared_ptr<Canvas>&BrainPlugins::constructCanvasOrGetLast(const std::string &name, const std::string& renderer_type, int width, int height) {
    if (!gBrainPlugins.mCanvasSingleton) {
        std::map<std::string,CanvasConstructor>::iterator i=gBrainPlugins.mCanvasPlugins.find(name);
        if (name.empty()&&i==gBrainPlugins.mCanvasPlugins.end()){
            i=gBrainPlugins.mCanvasPlugins.begin(); // go for the default constructor
        }
        if (i!=gBrainPlugins.mCanvasPlugins.end()){
            gBrainPlugins.mCanvasSingleton = i->second(renderer_type, width, height);
        }
    }
    return gBrainPlugins.mCanvasSingleton;
}

std::shared_ptr<Canvas>& BrainPlugins::returnConstructedCanvas() {
    return gBrainPlugins.mCanvasSingleton;
}

void BrainPlugins::shutdownGraphics() {
    gBrainPlugins.mCanvasSingleton.reset();
    gBrainPlugins.mCanvasPlugins.clear();
}
}
