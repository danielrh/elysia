#ifndef _BRAIN_PLUGIN_HPP_
#define _BRAIN_PLUGIN_HPP_
namespace Elysia {
class Brain;
class Neuron;
class BRAIN_CORE_EXPORT BrainPlugin {
public:
    enum UpdateReturn{
        RETURN_QUIT = -1,
        RETURN_RELINQUISH_FOCUS = 0,
        RETURN_NOP
    };

    BrainPlugin();
    virtual void initialize(Brain*b)=0;
    // tell if this plugin should get all mouse: returns if the operation succeeded
    virtual bool setFocus(bool focused)=0;
    virtual UpdateReturn update()=0;//called each frame: return false to relinquish focus
    ///called just before a neuron is destructed, so that the plugin can remove references to it
    virtual void notifyNeuronDestruction(Neuron* n)=0;
    virtual ~BrainPlugin();
};

}
#endif
