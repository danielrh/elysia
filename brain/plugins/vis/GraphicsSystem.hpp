extern "C" {
    union SDL_Event;
}
namespace Polarity {
class Canvas;
}
namespace Elysia {
class Visualization;
namespace GraphicsSystem {
    bool processSDLEvent(Visualization * vis, Polarity::Canvas *, SDL_Event *evt);
}
}
