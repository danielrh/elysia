#include <Platform.hpp>
#include "GraphicsSystem.hpp"
#include "MainThread.hpp"
#ifndef _WIN32
#include <unistd.h>
#endif
#include "Visualization.hpp"
#ifdef USE_SDL2
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif


namespace Elysia {

//FIXME: where do we get 1024 from
int getHeightPartition() {
    int heightPartition=1;
    return heightPartition;
}
int getWidthPartition() {
    return 1/getHeightPartition()+(1%getHeightPartition()?1:0);
}
float getUpperLeftX(size_t whichSystem, int width) {
    return (whichSystem%getWidthPartition())*width;
}
float getLowerRightX(size_t whichSystem, int width) {
    int wid=getWidthPartition();
    return (whichSystem%wid)*width+width/wid;
}
float getUpperLeftY(size_t whichSystem, int height) {
    return (whichSystem/getWidthPartition())*height;
}
float getLowerRightY(size_t whichSystem, int height) {
    int wid=getWidthPartition();
    return (whichSystem/wid)*height+height/getHeightPartition();
}
void getCoordsFromMouse(size_t whichWindow, int x, int y, float&newCoordX, float&newCoordY, Polarity::Canvas *canvas) {
    int w = canvas->width();
    int h = canvas->height();
    y = h - y;
    size_t i=whichWindow;
    float xl=getUpperLeftX(i, w);
    float yl=getUpperLeftY(i, h);
    float xu=getLowerRightX(i, w);
    float yu=getLowerRightY(i, h);
    newCoordX=(float)(x-(xl+xu)/2);
    newCoordY=(float)(y-(yl+yu)/2);
}
size_t getSystemWindowAndCoordsFromMouse(int x, int y, float&newCoordX, float&newCoordY, Polarity::Canvas *canvas) {
    int canvasWidth = canvas->width();
    int canvasHeight = canvas->height();
    int origx=x;
    int origy=y;
    y = canvasHeight-y;
    if (x>=canvasWidth) x=canvasWidth-1;
    if (y>=canvasHeight) y=canvasHeight-1;
    if (x<0) x=0;
    if (y<0) y=0;
    for (size_t i=0;i<1;++i) {
        float xl=getUpperLeftX(i, canvasWidth);
        float yl=getUpperLeftY(i, canvasHeight);
        float xu=getLowerRightX(i, canvasWidth);
        float yu=getLowerRightY(i, canvasHeight);
        if (x>=xl && xu > x &&y>=yl && yu >y ) {
            getCoordsFromMouse(i,origx,origy,newCoordX,newCoordY, canvas);
            return i;
        }
    }
    fprintf(stderr,"Cannot locate window under cursor right now");
    return 0;
}
namespace GraphicsSystem {
bool processSDLEvent(Visualization * vis, Polarity::Canvas *canvas,
                                     SDL_Event *event) {
#ifndef USE_SDL2
    if (event->type == SDL_VIDEORESIZE) {
        //event->type == SDL_WINDOWEVENT_RESIZED // <- for SDL2, which we don't need
        canvas->resize(event->resize.w, event->resize.h);
    } else
#endif
    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
        return false;
    } else {
        Visualization::Event evt;
        switch(event->type) {
          case SDL_KEYDOWN:
          case SDL_KEYUP: {
              auto keyData = event->key.keysym.sym;            
              evt.button = keyData % 256;
              
              evt.modCodes = event->key.keysym.mod;
              evt.mouseX = 0;
              evt.mouseY = 0;
              if (keyData < 256) {
                  if (event->type == SDL_KEYDOWN) {
                      evt.event = Visualization::Event::KEYBOARD;
                  } else {
                      evt.event = Visualization::Event::KEYBOARD_UP;
                  }
              }else {
                  if (event->type == SDL_KEYDOWN) {
                      evt.event = Visualization::Event::KEYBOARD_SPECIAL;
                  } else {
                      evt.event = Visualization::Event::KEYBOARD_SPECIAL_UP;
                  }
              }
              break;
          }
          case SDL_MOUSEBUTTONDOWN:
          case SDL_MOUSEBUTTONUP:
            evt.button = event->button.button - 1;
            evt.modCodes = 0;
            getSystemWindowAndCoordsFromMouse(event->button.x,event->button.y,
                                              evt.mouseX, evt.mouseY, canvas);
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                evt.event=Visualization::Event::MOUSE_CLICK;
            }else if (event->type == SDL_MOUSEBUTTONUP) {
                evt.event=Visualization::Event::MOUSE_UP;
            } else {
                assert(false && "this should only activate for mouse clicks");
            }
            break;
          case SDL_MOUSEMOTION:
            evt.event=Visualization::Event::MOUSE_MOVE;
            evt.modCodes = 0;
            for (int i = 0; i < 8; ++i) {
                if (event->motion.state & SDL_BUTTON(i + 1)) {
                    evt.button = i;
                    evt.event=Visualization::Event::MOUSE_DRAG;
                    break;
                }
            }
            getSystemWindowAndCoordsFromMouse(event->motion.x,event->motion.y,
                                              evt.mouseX, evt.mouseY, canvas);
            break;
        }
        vis->postInputEvent(evt);
    }
    return true;
}

}
}

