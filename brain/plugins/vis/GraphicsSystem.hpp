#include <thread>
extern "C" {
    union SDL_Event;
}
namespace Polarity {
class Canvas;
}
namespace Elysia {
class Visualization;
class GraphicsSystem {
    std::shared_ptr<std::thread> mRenderThread;
public:
    int getWidth()const;
    int getHeight()const;
    GraphicsSystem();
    ~GraphicsSystem();
    static bool processSDLEvent(Visualization * vis, Polarity::Canvas *, SDL_Event *evt);
};
}
