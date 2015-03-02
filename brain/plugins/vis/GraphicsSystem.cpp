#include <Platform.hpp>
#include "GraphicsSystem.hpp"
#include "MainThread.hpp"
#ifndef _WIN32
#include <unistd.h>
#endif
#ifdef __APPLE__
#include "glut.h"
#else
#include "GL/glut.h"
//#include <GL/freeglut.h>
#endif
#include "Visualization.hpp"
#ifdef USE_SDL2
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif


int GAngle=0;
volatile bool gKillGraphics=false;
volatile bool gShutDown=false;
volatile bool gInvalidDisplayFunction=false;
volatile bool gShutdownComplete=false;
int gGlutWindowId=0;
std::shared_ptr<std::thread> gRenderThread;
namespace Elysia {
std::vector<Visualization*> gToRender;
std::auto_ptr<std::mutex>gRenderLock (new std::mutex);
std::condition_variable gRenderCompleteCondition;
std::condition_variable gRenderCondition;


void Deinitialize() {
        {
            std::unique_lock<std::mutex> renderLock(*Elysia::gRenderLock);    
            Elysia::gToRender.clear();
            Elysia::gRenderCondition.notify_one();
            Elysia::gRenderCompleteCondition.notify_one();
        }
        gShutDown=true;
        gKillGraphics=false;
        fflush(stdout);
        fflush(stderr);
//        while(!gShutdownComplete);
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);//need to sleep to allow the gl context to destroy itself (mac requires the main thread exit by itself after the window is destroyed)
#endif
}
}
void GlutNop(){}

namespace Elysia {
static int gDisplayWidth=1024;
static int gDisplayHeight=768;
void Reshape(int w, int h) {
    gDisplayWidth=w;
    gDisplayHeight=h;
}

int GraphicsSystem::getWidth()const{
    return gDisplayWidth;

}
int GraphicsSystem::getHeight()const{
    return gDisplayHeight;
}
int getHeightPartition() {
    int heightPartition=1;
    if (gToRender.size()>3) {
        heightPartition=2;
    }
    if (gToRender.size()>10) {
        heightPartition=3;
    }
    return heightPartition;
}
int getWidthPartition() {
    return gToRender.size()/getHeightPartition()+(gToRender.size()%getHeightPartition()?1:0);
}
float getUpperLeftX(size_t whichSystem) {
    return (whichSystem%getWidthPartition())*gDisplayWidth;
}
float getLowerRightX(size_t whichSystem) {
    int wid=getWidthPartition();
    return (whichSystem%wid)*gDisplayWidth+gDisplayWidth/wid;
}
float getUpperLeftY(size_t whichSystem) {
    return (whichSystem/getWidthPartition())*gDisplayHeight;
}
float getLowerRightY(size_t whichSystem) {
    int wid=getWidthPartition();
    return (whichSystem/wid)*gDisplayHeight+gDisplayHeight/getHeightPartition();
}
void getCoordsFromMouse(size_t whichWindow, int x, int y, float&newCoordX, float&newCoordY) {
    y = gDisplayHeight-y;
    size_t i=whichWindow;
    float xl=getUpperLeftX(i);
    float yl=getUpperLeftY(i);
    float xu=getLowerRightX(i);
    float yu=getLowerRightY(i);
    newCoordX=(float)(x-(xl+xu)/2);
    newCoordY=(float)(y-(yl+yu)/2);
}
size_t getSystemWindowAndCoordsFromMouse(int x, int y, float&newCoordX, float&newCoordY) {
    int origx=x;
    int origy=y;
    y = gDisplayHeight-y;
    if (x>=gDisplayWidth) x=gDisplayWidth-1;
    if (y>=gDisplayHeight) y=gDisplayHeight-1;
    if (x<0) x=0;
    if (y<0) y=0;
    for (size_t i=0;i<gToRender.size();++i) {
        float xl=getUpperLeftX(i);
        float yl=getUpperLeftY(i);
        float xu=getLowerRightX(i);
        float yu=getLowerRightY(i);
        if (x>=xl && xu > x &&y>=yl && yu >y ) {
            getCoordsFromMouse(i,origx,origy,newCoordX,newCoordY);
            return i;
        }
    }
    fprintf(stderr,"Cannot locate window under cursor right now");
    return 0;
}
void Display(void) {
    if (!gKillGraphics) {
        glViewport(0,0,gDisplayWidth,gDisplayHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        {
            std::unique_lock<std::mutex> lok(*gRenderLock);
            gRenderCompleteCondition.notify_one();
            if (gToRender.size()) {
                gRenderCondition.wait(lok);
                int heightPartition=getHeightPartition();
                int i=0;
                for (int h=0;h<heightPartition;++h) {
                    int wlim=gToRender.size()/heightPartition;
                    if (gToRender.size()%heightPartition) wlim++;
                    for (int w=0;w<wlim;++w,++i) { 
                        assert(w*gDisplayWidth/wlim==getUpperLeftX(i));
                        assert(h*gDisplayHeight/heightPartition==getUpperLeftY(i));
                        assert((w+1)*gDisplayWidth/wlim==getLowerRightX(i));
                        assert((h+1)*gDisplayHeight/heightPartition==getLowerRightY(i));
                        glViewport(getUpperLeftX(i),
                                   getUpperLeftY(i),
                                   getLowerRightX(i),
                                   getLowerRightY(i));
                        glMatrixMode(GL_MODELVIEW);
                        glLoadIdentity();
                        glScalef(2*wlim/(float)(gDisplayWidth),2*heightPartition/(float)(gDisplayHeight),1.0);
                        if (i<(int)gToRender.size())
                            gToRender[i]->draw();
                    }
                }
            }
        }
        {
            std::unique_lock<std::mutex> lok(*gRenderLock);
            gRenderCompleteCondition.notify_one();
        }
        glFlush();
    }else {
    }
    glutSwapBuffers();
    if (gShutDown ) {
        gInvalidDisplayFunction=true;
        glutDestroyWindow(gGlutWindowId);
#ifdef __APPLE__
    exit(0);
#endif

    }
}
int glutKeyDown[256]={0};
int glutSpecialKeyDown[256]={0};
}
void Timer(int extra)
	{
        if (!gInvalidDisplayFunction) {
            glutPostRedisplay();
            glutTimerFunc(30,Timer,0);
        }else {
            gShutdownComplete=true;
        }
	}

void Idly() {
    if (!gInvalidDisplayFunction) {
        glutPostRedisplay();
    }
}
void kbdOrSpecial(unsigned char key, int x, int y, bool special) {
    std::unique_lock<std::mutex> renderLock(*Elysia::gRenderLock);
    using namespace Elysia;
    Visualization::Event evt;
    evt.event=special?Visualization::Event::KEYBOARD_SPECIAL:Visualization::Event::KEYBOARD;
    float newX=0,newY=0;
    size_t which=getSystemWindowAndCoordsFromMouse(x,y,newX,newY);
    evt.button=key;
    evt.modCodes=glutGetModifiers();
    if (gToRender.size()>which) {
        evt.mouseX=newX;
        evt.mouseY=newY;
        gToRender[which]->postInputEvent(evt);//FIXME is a lock necessary?
    }
    glutKeyDown[key]=1;
}
void kbdOrSpecialUp(int key, int x, int y, bool special) {
    std::unique_lock<std::mutex> renderLock(*Elysia::gRenderLock);
    using namespace Elysia;
    Visualization::Event evt;
    evt.event=special?Visualization::Event::KEYBOARD_SPECIAL_UP:Visualization::Event::KEYBOARD_UP;
    float newX=0,newY=0;
    size_t which=getSystemWindowAndCoordsFromMouse(x,y,newX,newY);
    evt.button=key;
    evt.modCodes=glutGetModifiers();
    for (size_t i=0;i<gToRender.size();++i) {
        getCoordsFromMouse(i,x,y,newX,newY);
        evt.mouseX=newX;
        evt.mouseY=newY;
        gToRender[i]->postInputEvent(evt);//FIXME is a lock necessary?
    }
    Elysia::glutKeyDown[key]=0;
}
static size_t elysiaMouseLastDrag=0;
void mouseFunc(int button, int up, int x, int y) {
    std::unique_lock<std::mutex> renderLock(*Elysia::gRenderLock);
    using namespace Elysia;
    Visualization::Event evt;
    evt.event=(up==GLUT_UP?Visualization::Event::MOUSE_UP:Visualization::Event::MOUSE_CLICK);
    float newX=0,newY=0;
    size_t which=getSystemWindowAndCoordsFromMouse(x,y,newX,newY);
    evt.button=button;
    evt.modCodes=glutGetModifiers();
    if (up==GLUT_UP) {
    for (size_t i=0;i<gToRender.size();++i) {
        getCoordsFromMouse(i,x,y,newX,newY);
        evt.mouseX=newX;
        evt.mouseY=newY;
        gToRender[i]->postInputEvent(evt);//FIXME is a lock necessary?
    }
    }else if (which<gToRender.size()) {
        getCoordsFromMouse(which,x,y,newX,newY);
        evt.mouseX=newX;
        evt.mouseY=newY;
        elysiaMouseLastDrag=which;
        gToRender[which]->postInputEvent(evt);//FIXME is a lock necessary?
    }
}
void mouseDrag(int x, int y) {
    std::unique_lock<std::mutex> renderLock(*Elysia::gRenderLock);
    using namespace Elysia;
    Visualization::Event evt;
    evt.event=Visualization::Event::MOUSE_DRAG;
    float newX=0,newY=0;
    size_t which=getSystemWindowAndCoordsFromMouse(x,y,newX,newY);
    evt.button=0;
    evt.modCodes=glutGetModifiers();
    if (elysiaMouseLastDrag<gToRender.size()){
        getCoordsFromMouse(elysiaMouseLastDrag,x,y,newX,newY);
        evt.mouseX=newX;
        evt.mouseY=newY;
        gToRender[elysiaMouseLastDrag]->postInputEvent(evt);//FIXME is a lock necessary?
    }
}

void mouseMove(int x, int y) {
    std::unique_lock<std::mutex> renderLock(*Elysia::gRenderLock);
    using namespace Elysia;
    Visualization::Event evt;
    evt.event=Visualization::Event::MOUSE_DRAG;
    float newX=0,newY=0;
    evt.button=0;
    evt.modCodes=glutGetModifiers();
    size_t which=getSystemWindowAndCoordsFromMouse(x,y,newX,newY);
    if (which<gToRender.size()){
        evt.mouseX=newX;
        evt.mouseY=newY;
        gToRender[which]->postInputEvent(evt);//FIXME is a lock necessary?
    }
}
void kbd(unsigned char c, int x, int y) {
    kbdOrSpecial(c,x,y,false);
}
void kbdUp(unsigned char c, int x, int y) {
    kbdOrSpecialUp(c,x,y,false);
}
void specialkbd(int c, int x, int y) {
    kbdOrSpecial(c,x,y,true);
}
void specialkbdUp(int c, int x, int y) {
    kbdOrSpecialUp(c,x,y,true);
}
volatile bool myfuncInitialized=false;
void myfunc() {
        // Enable Front Face
        //glEnable(GL_CULL_FACE);
    const char *argv[1];
    argv[0]="elysia";
    int argc=1;
    static bool firstTime=true;
    {
        std::unique_lock<std::mutex> lok(*Elysia::gRenderLock);
        if (gKillGraphics ){
            gKillGraphics=false;
        }else {
            
        }
        if (firstTime)
            glutInit(&argc, (char**)argv);
        firstTime=false;
        glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
        glutInitWindowSize(Elysia::gDisplayWidth,Elysia::gDisplayHeight);

        gGlutWindowId=glutCreateWindow("This is the window title");
        //glutIdleFunc(Idly);
        glutDisplayFunc(Elysia::Display);
        glutReshapeFunc(Elysia::Reshape);
        glutKeyboardFunc(&kbd);
        glutKeyboardUpFunc(&kbdUp);
        glutPassiveMotionFunc(&mouseMove);
        glutMotionFunc(&mouseDrag);
        glutMouseFunc(&mouseFunc);
        glutSpecialFunc(&specialkbd);
        glutSpecialUpFunc(&specialkbdUp);
        glutTimerFunc(0,Timer,0);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        Elysia::gRenderCondition.notify_one();
    }
    myfuncInitialized=true;
	glutMainLoop();

    for (int i=0;i<10;++i) {
        printf("Testing thread: %d/10\n",i+1);
    }
    gKillGraphics=false;
}
namespace Elysia {

GraphicsSystem::GraphicsSystem () {
    if (gRenderThread) {
        mRenderThread=gRenderThread;
        gKillGraphics=false;
    }else {
        std::unique_lock<std::mutex> renderLock(*Elysia::gRenderLock);    
        mRenderThread=gRenderThread=MainThread::wrestMainThread(&myfunc);
        //std::shared_ptr<std::thread> x(new std::thread(&myfunc));
        gRenderCondition.wait(renderLock);        
        gKillGraphics=false;

    }
}
GraphicsSystem::~GraphicsSystem () {
    gKillGraphics=true;
          
}

bool GraphicsSystem::processSDLEvent(Visualization * vis, Polarity::Canvas *canvas,
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
            assert(gToRender.size() == 1); // we don't need this any longer
            getSystemWindowAndCoordsFromMouse(event->button.x,event->button.y,
                                              evt.mouseX, evt.mouseY);
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
                                              evt.mouseX, evt.mouseY);
            break;
        }
        vis->postInputEvent(evt);
    }
    return true;
}

}
