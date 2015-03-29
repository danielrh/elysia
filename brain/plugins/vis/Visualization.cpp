#include "../../Platform.hpp"
#include "../../BrainPlugins.hpp"
#include "genome.pb.h"
#include "Visualization.hpp"
#include "GraphicsSystem.hpp"
#include "Branch.hpp"
#include "Neuron.hpp"
#include "Brain.hpp"
#include "Synapse.hpp"
#include <graphics/font_manager.hpp>
#include <graphics/color.hpp>

using Polarity::Canvas;

namespace Elysia {
static Polarity::Color toColor(const Vector4f&c) {
    return Polarity::Color(c.x * 255,
                           c.y * 255,
                           c.z * 255,
                           c.w * 255);
}
std::weak_ptr<GraphicsSystem> Visualization::mGlobalGraphics;
BrainPlugin* makeVisualization(Brain*b) {
    BrainPlugin*v=new Visualization;
    v->initialize(b);
    return v;
}
Visualization::Visualization() : mCanvas(BrainPlugins::returnConstructedCanvas()), fontSize(12), fontName("DroidSans"), mPadding(4) {
    mEvent = mCanvas->makeBlankEventUnion();
    mNeuronSize=.5;
    mSynapseSnapToEdge=true;
    mScale=100.0;
    mOffset=Vector3f(0,0,0);
    mInput.init(this);
}

bool Visualization::processWindowEvents() {
    while (mCanvas->getNextEvent(mEvent)) {
        auto canvas = mCanvas;
        if (!GraphicsSystem::processSDLEvent(this, canvas.get(), mEvent)) {
                return false;
        }
    }
    return true;
}

BrainPlugin::UpdateReturn Visualization::update() {
    BrainPlugin::UpdateReturn retval = RETURN_NOP;
    if (mIsFocused) {
        if (!processWindowEvents()) {
            return RETURN_RELINQUISH_FOCUS;
        }
        draw();
    }
    return retval;
}

bool Visualization::setFocus(bool value) {
    mIsFocused = value;
    return true;
}

void Visualization::initialize(Brain*b) {
    this->mBrain=b;
    BoundingBox3f3f bounds = b->getBounds();
    if (bounds==BoundingBox3f3f::null()) {
        bounds=BoundingBox3f3f(Vector3f(-128,-128,-1),Vector3f(128,128,1));
    }
    Vector3f diag=bounds.diag();
    float ratiox=diag.x/mCanvas->width();
    float ratioy=diag.y/mCanvas->height();
    float maxratio=ratiox>ratioy?ratiox:ratioy;
    mScale=1./maxratio;
    mOffset=-bounds.center();
    mCanvasWidth = mCanvas->width();
    mCanvasHeight = mCanvas->height();
}


static float selectiondefaultcol[4]={1,1,1,1};
void Visualization::arrow (float ox,float oy, float oz, float ex, float ey, float ez, float thickness, const Polarity::Color & color) {
  int abs_thickness=(int)(thickness < 0 ? -thickness : thickness);
  if (!abs_thickness) abs_thickness = 1;//arrows are at least thickness one

  float dx =ex-ox;
  float dy =ey-oy;
  float ldx=sqrt(dx*dx+dy*dy);
  dx*=1/ldx;
  dy*=1/ldx;
  float idx=-dy;
  float idy=dx;
  for (int offs = -abs_thickness/2; offs < (abs_thickness + 1) / 2; ++offs) {
      Vector3f endpoint(ex+ offs * idx,ey + offs * idy,oz);
      drawAndScreenTransformLine(Vector3f(ox+ offs * idx,oy + offs * idy,oz),
               endpoint,
               color);
      drawAndScreenTransformLine(endpoint,
               Vector3f(ex-6*dx+3*idx,ey-6*dy+3*idy,ez),
               color);
      drawAndScreenTransformLine(endpoint,
               Vector3f(ex-6*dx-3*idx,ey-6*dy-3*idy,ez),
               color);             
/*                 
  glVertex3f(ex-idx-3*dx,ey-idy-3*dy,ez);
  glVertex3f(ex+idx-3*dx,ey+idy-3*dy,ez);
  glVertex3f(ox+idx,oy+idy,oz);

  glVertex3f(ex-3*dx,ey-3*dy,ez);
  glVertex3f(ex-6*dx+3*idx,ey-6*dy+3*idy,ez);
  glVertex3f(ex,ey,ez);
  glVertex3f(ex-6*dx-3*idx,ey-6*dy-3*idy,ez);
*/    

  }
/*
  glVertex3f(ox-idx,oy-idy,oz);
  glVertex3f(ex-idx-3*dx,ey-idy-3*dy,ez);
  glVertex3f(ex+idx-3*dx,ey+idy-3*dy,ez);
  glVertex3f(ox+idx,oy+idy,oz);

  glVertex3f(ex-3*dx,ey-3*dy,ez);
  glVertex3f(ex-6*dx+3*idx,ey-6*dy+3*idy,ez);
  glVertex3f(ex,ey,ez);
  glVertex3f(ex-6*dx-3*idx,ey-6*dy-3*idy,ez);
*/
}
void Visualization::postInputEvent(const Event&evt){
    mInputEvents.push_back(evt);
}

void Visualization::arrow(Vector3f start, Vector3f finish, float thickness, const Polarity::Color&c) {
//    printf("Drawing arrow from %f %f %f to %f %f %f t %f\m",           start.x,start.y,start.z,finish.x,finish.y,finish.z,thickness);
    arrow(start.x,start.y,start.z,finish.x,finish.y,finish.z,thickness, c);
}
void Visualization::drawRect(Vector3f lower_left,Vector3f upper_right, const Polarity::Color&color) {
    //printf ("drawing from %f %f to %f %f\n",lower_left.x,lower_left.y,upper_right.x,upper_right.y);
    drawAndScreenTransformBox(lower_left, upper_right, color);
}
void Visualization::drawRectOutline(Vector3f lower_left,Vector3f upper_right, float halfx,float halfy) {
    Polarity::Color color(255, 255, 255, 255);
    drawAndScreenTransformBox(lower_left, upper_right, color);

}


int Visualization::stringWidth(const std::string &dat, bool addspace, bool removespace) {
    Polarity::Rect retval=mCanvas->fontManager().textSize(fontName, fontSize, addspace?dat + " " : dat);
    if (removespace) {
        retval.w -= mCanvas->fontManager().textSize(fontName, fontSize, " ").w;
    }
    return retval.w;
}
#define highest_special_char ((char)47)
#define lowest_special_char ((char)35)

bool Visualization::getNeuronWidthHeight(const std::string &text, float&wid,float&hei,bool selected) {    
    float minh=.5;
    bool drawText=text.length()&&(text[0]==' '||(text[0]>=lowest_special_char&&text[0]<=highest_special_char));
    float neuronsize=mNeuronSize*mScale;
    float neuronheight=neuronsize;
    float neuronwidth=neuronsize;
    float textwidth=0;
    if (drawText&&neuronheight<minh)
        neuronheight=minh;
    if (selected||neuronheight<mNeuronSize*mScale*1.625) {
        //we're fine
    }else {
        neuronheight=mNeuronSize*mScale;
        drawText=false;
    }
    if (neuronheight<1) {
        neuronheight=1;
        neuronwidth=1;
    }
    if (drawText) textwidth=stringWidth(text,true,true);
    
    if (drawText&&neuronwidth<textwidth){
        neuronwidth=textwidth;
    }
    wid=neuronwidth;
    hei=neuronheight;
    return drawText;		
}
Vector3f Visualization::getNeuronLocation(Neuron* n)const {
    return (n->getLocation()+mOffset)*mScale;
}

void Visualization::getSynapseStartEnd(Neuron * start, bool startIsSelected, Neuron * end, bool endIsSelected, Vector3f& A, Vector3f &B) {
  float swid,shei,ewid,ehei;
  A=getNeuronLocation(start);
  B=getNeuronLocation(end);
  if (mSynapseSnapToEdge==false) {
    return;
  }

  getNeuronWidthHeight(start->getName(),swid,shei,startIsSelected);
  getNeuronWidthHeight(end->getName(),ewid,ehei,endIsSelected);
  Vector3f delta=B-A;
  Vector3f startdelta(0,0,0),enddelta(0,0,0);
  if (fabs(delta.x)>fabs(delta.y)){          
    if (delta.x>0) {
      startdelta.x+=swid;
      enddelta.x-=ewid;
    }else {
      startdelta.x-=swid;
      enddelta.x+=ewid;
    }
  }else {
    if (delta.y>0) {
      startdelta.y+=shei;
      enddelta.y-=ehei;
    }else {
      startdelta.y-=shei;
      enddelta.y+=ehei;
    }
  }
  if (start->getName().length()&&start->getName()[0]=='!')
    startdelta.x=startdelta.y=0;//not useful right now
  if (end->getName().length()&&end->getName()[0]=='!')
    enddelta.x=enddelta.y=0;//not useful right now
  
  A+=startdelta;
  B+=enddelta;
}
float drawThreshold(Brain*brain,SimTime firedSimTime) {//0.0 = no change 1.0 = draw fully
    int64_t delta=(brain->mCurTime.getRawTime()-firedSimTime.getRawTime());
    if (delta<4)
        return (4-delta)/2.;
    return 0;
}
Vector4f getComponentColor(const Visualization *v, const Neuron * n, const CellComponent*cc, bool isDetailed, bool isSelected) {
    float howOn=drawThreshold(n->getBrain(),cc->getLastActivity());
    Vector4f color(.25/1.5,.35/1.5,1.0/1.5,.75/1.5);
    if (howOn>.25) {
        color.x=howOn;
        color.y=isSelected?1.25-howOn*1.25:.35-howOn*.35;
        color.z=1.25-howOn*1.25;
        color.w=.75;
    }else if (isSelected&&isDetailed) {
        color=Vector4f(.25,1.0,1.0,.75);
    } else if (isSelected) {
        color=Vector4f(.25,.35,1.0,.75);
    }else if (isDetailed) {        
        color=Vector4f(.25,1.0,.35,.75);
    }
    return color;
    
}
Vector4f getComponentColor(const Visualization *v, const Neuron * n, const CellComponent*cc) {
    
    bool isDetailed=v->isDetailed(n);
    bool isSelected=v->isSelected(n);
    return getComponentColor(v,n,cc,isDetailed,isSelected);
}
Vector3f Visualization::drawNeuronBody(Neuron*n) {
    //Vector3f center=n->getLocation();
    //center.z=0; FIXME: why not used
    float wid=0;
    float hei=0;
    bool isSelected=mSelectedNeurons.find(n)!=mSelectedNeurons.end();
    bool isDetailed=mDetailedNeurons.find(n)!=mDetailedNeurons.end();
    bool text=getNeuronWidthHeight(n->getName(), wid,hei,isSelected);
    //printf ("aaawing from %f %f to %f %f\n",((center-Vector3f(wid/2,hei/2,0))).x,((center-Vector3f(wid/2,hei/2,0))).y,((center+Vector3f(wid/2,hei/2,0))).x,(center+Vector3f(wid/2,hei/2,0)).y);
    Vector3f scaledCenter=getNeuronLocation(n);
    Vector4f color = getComponentColor(this,n,n);
    drawRect(scaledCenter-Vector3f(wid/2,hei/2,0),scaledCenter+Vector3f(wid/2,hei/2,0), toColor(color));
    return scaledCenter+Vector3f(0,hei/2,0);
}

void Visualization::drawParallelogramLineSegment(const Vector3f &source, const Vector3f &dest, double thickness, const Polarity::Color & color) {
  int abs_thickness=(int)(thickness < 0 ? -thickness : thickness);
  if (!abs_thickness) abs_thickness = 1;//arrows are at least thickness one
    for (int offs = -abs_thickness/2; offs < (abs_thickness + 1) / 2; ++offs) {
        drawAndScreenTransformLine(Vector3f(source.x + offs, source.y, source.z),
                                   Vector3f(dest.x + offs, dest.y, dest.z),
                                   color);
    }
}

void Visualization::drawBranch(const Neuron * n, const Branch* dendrite, Vector3f top, float scale) {
    for (Branch::SynapseConstIterator i=dendrite->childSynapseBegin(),ie=dendrite->childSynapseEnd();i!=ie;++i) {
        Neuron * destination = (*i)->recipient();
        if (destination) {
            Vector4f color = getComponentColor(this,destination, destination);
            float wid=0;
            float hei=0;
            bool text=getNeuronWidthHeight(destination->getName(), wid,hei,mSelectedNeurons.find(destination)!=mSelectedNeurons.end());
            Vector3f scaledDestination = getNeuronLocation(destination);
            arrow(scaledDestination-Vector3f(0,hei/2,0), top, 1, toColor(color));
        }
    }
}
void Visualization::drawDendrites(const Neuron * n, const CellComponent* dendrite, Vector3f top, float scale, bool isDetailed, bool isSelected) {
    CellComponent::ChildIterator i=dendrite->childBegin(),ie=dendrite->childEnd(),b;
    size_t size = ie-i;
    b=i;    
    if (scale<.25) scale=0.0;
    float width = scale*.125/size;
    float height = mScale;
    //if (height<1.0) height=1.0;
    //if (width<2.0) width=2.0;
    for (;i!=ie;++i) {
        Vector3f dest = Vector3f(top.x-scale*.25+scale*(i-b)/((double)size),
                                 top.y+height,
                                 top.z);        
        if (scale) {
            Vector4f color = getComponentColor(this,n, *i,isDetailed,isSelected);
            drawParallelogramLineSegment(top,dest,width, toColor(color));
        }
        const CellComponent *nextInLine = *i;
        drawDendrites(n,nextInLine,scale?dest:top,scale*.5,isDetailed,isSelected);
    }
    {
        const Branch * b = dynamic_cast<const Branch*>(dendrite);
        if (b) {
            drawBranch(n,b,top,scale);
        }
    }

}
bool Visualization::isDetailed(const Neuron*n) const{
    return mDetailedNeurons.find(const_cast<Neuron*>(n))!=mDetailedNeurons.end();
}
bool Visualization::isSelected(const Neuron*n) const{
    return mSelectedNeurons.find(const_cast<Neuron*>(n))!=mSelectedNeurons.end();
}
void Visualization::drawNeuron(Neuron*n) {
    Vector3f top = drawNeuronBody(n);
    bool isDetailed=this->isDetailed(n);
    bool isSelected=this->isSelected(n);
    bool drawDendrites=isDetailed;
    
    if (drawDendrites) {
        //draw at the same time to keep the same color
        std::vector<Synapse*>::iterator iter,eiter=n->getSynapsesAtAxonTipEnd();
        for (iter=n->getSynapsesAtAxonTipBegin();iter!=eiter;++iter) {
            Synapse * syn=*iter;
            Neuron * destination=syn->mParentBranch->getParentNeuron();
            if (!this->isDetailed(destination)) {
                //draw to the center of the neuron then (otherwise the branch will draw to us)
                Vector3f scaledDestination = getNeuronLocation(destination);
                Vector3f scaledSource = getNeuronLocation(n);
                arrow(scaledSource, scaledDestination,1, toColor(getComponentColor(this, n, n)));
            }
        }
        this->drawDendrites(n, n, top, mScale*2,isDetailed,isSelected);
    }
    
}

Visualization::InputStateMachine::InputStateMachine() {
    memset(mKeyDown,0,sizeof(mKeyDown));
    memset(mSpecialKeyDown,0,sizeof(mSpecialKeyDown));
    memset(mMouseButtons,0,sizeof(mMouseButtons));
    mActiveDrag=0;
    mDragStartX=0;
    mDragStartY=0;
}

void Visualization::clearDetail(){
    this->mDetailedNeurons.clear();
}

void Visualization::addSelectedToDetail(){
    this->mDetailedNeurons.insert(mSelectedNeurons.begin(),mSelectedNeurons.end());
}
void Visualization::addAllToDetail(){
    this->mDetailedNeurons.insert(mBrain->mAllNeurons.begin(),mBrain->mAllNeurons.end());
}
void Visualization::selectAll(){
    this->mSelectedNeurons.insert(mBrain->mAllNeurons.begin(),mBrain->mAllNeurons.end());
}
void Visualization::subtractSelectedFromDetail(){
    for (SelectedNeuronMap::iterator i=mSelectedNeurons.begin();
         i!=mSelectedNeurons.end();++i) {
        SelectedNeuronMap::iterator where=this->mDetailedNeurons.find(*i);
        if (where!=mDetailedNeurons.end()) {
            mDetailedNeurons.erase(where);
        }
    }

}
void Visualization::intersectSelectedWithDetail(){
    SelectedNeuronMap::iterator i=mDetailedNeurons.begin();
    while (i!=mDetailedNeurons.end()) {
        if (mSelectedNeurons.find(*i)==mSelectedNeurons.end()) {
            SelectedNeuronMap::iterator j=i;
            ++j;
            if (j!=mDetailedNeurons.end()) {
                Neuron * test=*j;
                mDetailedNeurons.erase(i);
                i=mDetailedNeurons.find(test);
            }else {
                mDetailedNeurons.erase(i);
                break;
            }
        }else ++i;
    }
}

std::pair<int, int> Visualization::transformToScreenSpace(const Vector3f& v) const{
    int x0 = v.x + (mCanvasWidth >> 1);
    int y0 = -v.y + (mCanvasHeight >> 1);
    return std::pair<int,int>(x0, y0);
}

void Visualization::drawAndScreenTransformLine(const Vector3f &v0,
                                              const Vector3f &v1,
                                              const Polarity::Color &color) const{
    std::pair<int,int> t0 = transformToScreenSpace(v0);
    std::pair<int,int> t1 = transformToScreenSpace(v1);
    mCanvas->drawLine(t0.first, t0.second, t1.first, t1.second, color);
}

void Visualization::drawAndScreenTransformBox(const Vector3f &v0,
                                              const Vector3f &v1,
                                              const Polarity::Color &color) const{
    std::pair<int,int> t0 = transformToScreenSpace(v0);
    std::pair<int,int> t1 = transformToScreenSpace(v1);
    mCanvas->drawLine(t0.first, t0.second, t0.first, t1.second, color);
    mCanvas->drawLine(t0.first, t1.second, t1.first, t1.second, color);
    mCanvas->drawLine(t1.first, t1.second, t1.first, t0.second, color);
    mCanvas->drawLine(t1.first, t0.second, t0.first, t0.second, color);
}

void Visualization::InputStateMachine::init(Visualization*parent) {
    mButtons.push_back(Button(0,20,20,35,"Add To Detail",std::bind(&Visualization::addSelectedToDetail,parent)));
    mButtons.push_back(Button(0,40,20,55,"Subtract from Detail",std::bind(&Visualization::subtractSelectedFromDetail,parent)));
    mButtons.push_back(Button(0,60,20,75,"Intersect with Detail",std::bind(&Visualization::intersectSelectedWithDetail,parent)));
    mButtons.push_back(Button(0,0,10,15,"Clear Detail Display",std::bind(&Visualization::clearDetail,parent)));
    mButtons.push_back(Button(0,80,20,95,"Add All To Detail Display",std::bind(&Visualization::addAllToDetail,parent)));
    mButtons.push_back(Button(0,100,15,115,"Select All",std::bind(&Visualization::selectAll,parent)));
}

void Visualization::InputStateMachine::draw(Visualization*parent) {
    for (size_t i=0;i<mButtons.size();++i) {
        mButtons[i].draw(parent);
    }
    if (mActiveDrag&&(mMouseX!=mDragStartX||mMouseY!=mDragStartY)) {
        Polarity::Color color(127, 127, 127, 127);
        parent->drawAndScreenTransformBox(Vector3f(mDragStartX, mDragStartY, 0),
                                          Vector3f(mMouseX, mMouseY, 0),
                                          color);
    }
}
void Visualization::InputStateMachine::processPersistentState(const Visualization::Event&evt) {
    const Visualization::Event* i=&evt;
    switch(i->event) {
      case Event::MOUSE_CLICK:
        mMouseX=i->mouseX;
        mMouseY=i->mouseY;
        if (i->button<(int)(sizeof(mMouseButtons)/sizeof(mMouseButtons[0]))) {
            mMouseButtons[i->button]=1;
        }
        break;
      case Event::MOUSE_UP:

        mMouseX=i->mouseX;
        mMouseY=i->mouseY;
        if (i->button<(int)(sizeof(mMouseButtons)/sizeof(mMouseButtons[0]))) {
            mMouseButtons[i->button]=0;
        }
        break;
      case Event::KEYBOARD:
        mKeyDown[i->button]=1;
        break;
      case Event::KEYBOARD_UP:
        mKeyDown[i->button]=0;
        break;
      case Event::KEYBOARD_SPECIAL:
        if (i->button<256)
            mSpecialKeyDown[i->button]=1;
        break;
      case Event::KEYBOARD_SPECIAL_UP:
        if (i->button<256)
            mSpecialKeyDown[i->button]=0;
        break;
      case Event::MOUSE_DRAG:
        mMouseX=i->mouseX;
        mMouseY=i->mouseY;
        break;
      case Event::MOUSE_MOVE:
        mMouseX=i->mouseX;
        mMouseY=i->mouseY;
        break;
    }

}

bool Visualization::getCurrentNeuronWidthHeight(Neuron * n, float&width,float&hei) {
    getNeuronWidthHeight(n->getName(),width,hei,mSelectedNeurons.find(n)!=mSelectedNeurons.end());
    return false;
}
void Visualization::Button::doClick(Visualization*vis, const Visualization::Event&evt) const{
    if (click(vis,evt)) {
        mClick();
    }
}
bool Visualization::Button::click(Visualization*parent, const Visualization::Event&evt) const{
    if (evt.mouseX<=maxX-parent->mCanvas->width()/2&&evt.mouseX>=minX-parent->mCanvas->width()/2&&
        evt.mouseY<=maxY-parent->mCanvas->height()/2&&evt.mouseY>=minY-parent->mCanvas->height()/2) {
        return true;
    }    
    return false;
}

bool Visualization::click (Neuron * n, float x, float y) {
    Vector3f where=getNeuronLocation(n);
    float wid=0,hei=0;
    getCurrentNeuronWidthHeight(n,wid,hei);
    wid/=2;
    hei/=2;
    return x<=where.x+wid&&x>=where.x-wid&&
        y<=where.y+hei&&y>=where.y-hei;
}
bool Visualization::dragSelect (Neuron * n, float x1,float y1, float x2, float y2) {
    float minx=(x1<x2?x1:x2);
    float maxx=(x1<x2?x2:x1);
    float miny=(y1<y2?y1:y2);
    float maxy=(y1<y2?y2:y1);
    Vector3f where=getNeuronLocation(n);
    float wid=0,hei=0;
    getCurrentNeuronWidthHeight(n,wid,hei);
    wid/=2;
    hei/=2;
    bool to_the_right=where.x+wid<=maxx;
    bool to_the_left=where.x-wid>=minx;
    bool to_the_top=where.y+hei<=maxy;
    bool to_the_bot=where.y-hei>=miny;
    //printf ("(%f,%f) [%f %f %f %f] R %d L %d T %d B %d\n",where.x,where.y,maxx,minx,maxy,miny,to_the_right,to_the_left,to_the_top,to_the_bot);
    return to_the_right&&to_the_left&&to_the_top&&to_the_bot;
}

void Visualization::InputStateMachine::drag(Visualization *vis, const Visualization::Event&evt){
    std::vector<Neuron*>dragged;
    for (Brain::NeuronSet::iterator i=vis->mBrain->mAllNeurons.begin(),
             ie=vis->mBrain->mAllNeurons.end();
         i!=ie;
         ++i) {
        if (vis->dragSelect(*i,mDragStartX,mDragStartY,evt.mouseX,evt.mouseY)){
            dragged.push_back(*i);
        }
    }
    vis->mSelectedNeurons.clear();
    vis->mSelectedNeurons.insert(dragged.begin(),dragged.end());
}
Visualization::Button::Button(float minX,
                              float minY,
                              float maxX,
                              float maxY,
                              const std::string &text,
                              const std::function<void()> &click,
                              float scale):minX(minX),
                                                                      maxX(maxX),
                                                                      minY(minY),
                                                                      maxY(maxY),
                                                                      mText(text),
                                                                      mClick(click){
    renderedOnce=false;
    mScale=scale;
}
void Visualization::drawString(Vector3f lower_left, float scale, const std::string &text, bool addspace) {
    Polarity::Color color(255, 255, 255, 255);
    std::pair<int, int> ll = transformToScreenSpace(lower_left);
    ll.second -= fontSize;
    mCanvas->fontManager().drawText(mCanvas.get(), ll.first, ll.second, fontName, fontSize, color, addspace ? " " + text : text);
}
void Visualization::Button::draw(Visualization * parent) {
    if (!renderedOnce) {
        renderedOnce=true;
        bool removespace=false;
        bool addspace=false;    
        float width = parent->stringWidth(mText, addspace, removespace) + parent->mPadding;
        if ((maxX-minX)<width) {
            this->maxX=this->minX+width;
        }

    }
    float recenterMinX=minX;
    float recenterMinY=minY;
    float recenterMaxX=maxX;
    float recenterMaxY=maxY;
    
    Vector3f lower_left(recenterMinX-parent->mCanvas->width()/2,recenterMinY-parent->mCanvas->height()/2,0);
    Vector3f upper_right(recenterMaxX-parent->mCanvas->width()/2,recenterMaxY-parent->mCanvas->height()/2,0);
    lower_left.x += parent->mPadding >> 1;
    upper_right.x += parent->mPadding >> 1;
    parent->drawString(lower_left,mScale,mText,false);
}

void Visualization::InputStateMachine::click(Visualization *vis, const Visualization::Event&evt){
    bool hasClicked=false;
    for (ptrdiff_t i=mButtons.size()-1;i>=0;--i) {
        if (mButtons[i].click(vis,evt)) {
            mButtons[i].doClick(vis,evt);
            hasClicked=true;
        }
    }
    std::vector<Neuron*>clicked;
    if (!hasClicked) {
        for (Brain::NeuronSet::iterator i=vis->mBrain->mAllNeurons.begin(),
                 ie=vis->mBrain->mAllNeurons.end();
             i!=ie;
             ++i) {
            if (vis->click(*i,evt.mouseX,evt.mouseY)){
                clicked.push_back(*i);
                vis->mSelectedNeurons.clear();
                vis->mSelectedNeurons.insert(*i);
                
            }
        }
    }
    
}

void Visualization::InputStateMachine::processEvent(Visualization*parent, const Event&evt) {
    if (evt.event==Event::MOUSE_CLICK&&evt.button==0) {
        mDragStartX=evt.mouseX;
        mDragStartY=evt.mouseY;
        //nop: if they wanted to click they would mouseup in the same place
        mActiveDrag=true;
    }

    if (evt.event==Event::MOUSE_UP) {
        if (evt.button==0) {
            if (mDragStartX==evt.mouseX&&mDragStartY==evt.mouseY) {
                click(parent,evt);
            }else {
                if(mActiveDrag==true) {
                    drag(parent,evt);
                }
            }
            mActiveDrag=false;
        }
    }
}


void Visualization::doInput() {
    std::vector<Event> inputEvents;
    mInputEvents.swap(inputEvents);
    for (std::vector<Event>::iterator i=inputEvents.begin(),ie=inputEvents.end();i!=ie;++i) {
        mInput.processPersistentState(*i);
        mInput.processEvent(this,*i);
    }
    float speed=20;
    if (mInput.mKeyDown['d']) {
        mOffset.x-=speed/mScale;
    }
    if (mInput.mKeyDown['a']) {
        mOffset.x+=speed/mScale;
    }
    if (mInput.mKeyDown['w']) {
        mOffset.y-=speed/mScale;
    }
    if (mInput.mKeyDown['s']) {
        mOffset.y+=speed/mScale;
    }
    if (mInput.mKeyDown['q']) {
        mScale*=.95;
    }
    if (mInput.mKeyDown['e']) {
        mScale*=1.05;
    }
}
void Visualization::purgeMarkedForDeathNeurons() {
   
    for (std::vector<Neuron*>::iterator i=mfd.begin(),ie=mfd.end();i!=ie;++i) {
        SelectedNeuronMap::iterator where =mSelectedNeurons.find(*i);
        if (where!=mSelectedNeurons.end()) {
            mSelectedNeurons.erase(where);
        }
        where =mDetailedNeurons.find(*i);
        if (where!=mDetailedNeurons.end()) {
            mDetailedNeurons.erase(where);
        }
    }
}

void Visualization::draw() {
    mCanvasWidth = mCanvas->width();
    mCanvasHeight = mCanvas->height();
    purgeMarkedForDeathNeurons();
    doInput();
    if (mIsFocused) {
        mCanvas->beginFrame();
        mCanvas->clear();
        // do we need a cursor here?
        // glVertex3f(mInput.mMouseX,mInput.mMouseY,0);
        // glVertex3f(mInput.mMouseX,mInput.mMouseY-10,0);
        // glVertex3f(mInput.mMouseX-6,mInput.mMouseY-12,0);
        // glVertex3f(mInput.mMouseX-12,mInput.mMouseY-10,0);
        for (Brain::NeuronSet::iterator i=mBrain->mAllNeurons.begin(),
                 ie=mBrain->mAllNeurons.end();
             i!=ie;
             ++i) {
            drawNeuron(*i);
        }
        mInput.draw(this);
        mCanvas->endFrame();
    }
}
Visualization::~Visualization() {
    mCanvas->destroyEventUnion(mEvent);
}
void Visualization::notifyNeuronDestruction(Neuron*n){
    mfd.push_back(n);
}



}
