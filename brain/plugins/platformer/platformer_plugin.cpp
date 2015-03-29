#include "../../Platform.hpp"
#include "BrainPlugins.hpp"
#include "BrainPlugin.hpp"
#include <main/game.hpp>
#include <graphics/canvas.hpp>

namespace Elysia {
class SubGame : public BrainPlugin {
    Brain * brain;
    std::shared_ptr<Polarity::Canvas> canvas;
    SDL_Event *event;
    bool isFocused;
public:
    SubGame(const std::shared_ptr<Polarity::Canvas> &global_canvas):canvas(global_canvas){
        isFocused = true;
        brain = NULL;
        event = canvas->makeBlankEventUnion();
    }
    void initialize(Brain *b) {
        this->brain = b;
        Polarity::Game::getSingleton().startGame(canvas, "assets/levels/level2.tmx");
    }
    bool setFocus(bool focused) {
        isFocused = focused;
        return true;
    }
    UpdateReturn update() {
        if (!canvas) {
            return RETURN_NOP;      
        }
        UpdateReturn retval;
        if (isFocused) {
            while (canvas->getNextEvent(event)) {
                if (!Polarity::Game::getSingleton().injectInput(event)) {
                    return RETURN_RELINQUISH_FOCUS;
                }
            }
        }
        Polarity::Game::getSingleton().performTick();
        if (isFocused) {
            Polarity::Game::getSingleton().drawFrame();
        }
        return RETURN_NOP;
    }
    void notifyNeuronDestruction(Neuron* n) {
    }
    ~SubGame() {
        canvas->destroyEventUnion(event);
        event = NULL;
        Polarity::Game::getSingleton().stopGame();
        
    }
};

BrainPlugin* makePlatformer(Brain * b) {
    auto canvas = BrainPlugins::returnConstructedCanvas();
    if (canvas){
        BrainPlugin * retval = new SubGame(canvas);
        retval->initialize(b);
        return retval;
    }
    return NULL;
}

}

static int core_plugin_refcount = 0;

ELYSIA_PLUGIN_EXPORT_C void init() {
    core_plugin_refcount++;
    fprintf(stderr, "LOADING PLATFORMER PLUGIN\n");
    Elysia::BrainPlugins::registerBrainPlugin("platformer", &Elysia::makePlatformer);
}

ELYSIA_PLUGIN_EXPORT_C void destroy() {
//    Elysia::Deinitialize();
    core_plugin_refcount--;
}

ELYSIA_PLUGIN_EXPORT_C const char* name() {
    return "platformer";
}

ELYSIA_PLUGIN_EXPORT_C int refcount() {
    return core_plugin_refcount;
}
