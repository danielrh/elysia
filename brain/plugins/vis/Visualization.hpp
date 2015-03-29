#include "BrainPlugin.hpp"
#include <graphics/canvas.hpp>
namespace Polarity {
class Canvas;
}
namespace Elysia {
class Brain;
class CellComponent;
class GraphicsSystem;
class Neuron;
class Branch;
class Visualization:public BrainPlugin  {
    SDL_Event *mEvent;
    std::shared_ptr<Polarity::Canvas> mCanvas;
    bool mIsFocused;
    bool processWindowEvents();
    ///Neurons marked for death by the main plugin
    std::vector<Neuron*> mfd;
    void clearDetail();
    void addAllToDetail();
    void selectAll();
    void addSelectedToDetail();
    void subtractSelectedFromDetail();
    void intersectSelectedWithDetail();
    std::shared_ptr<GraphicsSystem> mGraphics;
    static std::weak_ptr<GraphicsSystem> mGlobalGraphics;
    Brain * mBrain;
    float mNeuronSize;
    float mScale;
    Vector3f mOffset;
    int mCanvasWidth;
    int mCanvasHeight; // updated once per frame
    bool mSynapseSnapToEdge;
    int fontSize;
    std::string fontName;
    int mPadding; //padding around buttons
    Vector3f getNeuronLocation(Neuron*n)const;
    bool getNeuronWidthHeight(const std::string&text, float&width, float&hei, bool selected);
    bool getCurrentNeuronWidthHeight(Neuron*n, float&width, float&height);
    void getSynapseStartEnd(Neuron * start, bool startIsSelected, Neuron * end, bool endIsSelected, Vector3f& A, Vector3f &B);
    ///Returns top location from where dendrites can start branching out
    Vector3f drawNeuronBody(Neuron*n);
    void drawBranch(const Neuron * n, const Branch* dendrite, Vector3f top, float scale) ;
    void drawDendrites(const Neuron* n, const CellComponent* dendrite, Vector3f startLocation, float scale, bool isDetailed, bool isSelected);
    void drawNeuron(Neuron*n);
    void drawString(Vector3f lower_left, float scale, const std::string &text, bool addspace);
    std::pair<int, int> transformToScreenSpace(const Vector3f&v) const;
    void drawAndScreenTransformLine(const Vector3f&v0, const Vector3f&v1, const Polarity::Color&color) const;
    void drawAndScreenTransformBox(const Vector3f&v0, const Vector3f&v1, const Polarity::Color&color) const;

    void doInput();
    typedef std::unordered_set<Neuron*> SelectedNeuronMap;
    SelectedNeuronMap mSelectedNeurons;
    SelectedNeuronMap mDetailedNeurons;

    void purgeMarkedForDeathNeurons();
    ///Returns if a single neuron may be selected by the given mouse coordinates
    bool click (Neuron*n,float mousex, float mousey);
    ///Returns if a single neuron may be selected by the given drag select box
    bool dragSelect(Neuron *n, float bbminx, float bbminy, float bbmaxx, float bbmaxy);
    void arrow(Vector3f start, Vector3f finish, float thickness, const Polarity::Color &c);
    void arrow (float ox,float oy, float oz, float ex, float ey, float ez, float thickness, const Polarity::Color &c);
    void drawRect(Vector3f lower_left,Vector3f upper_right, const Polarity::Color&);
    void drawRectOutline(Vector3f lower_left,Vector3f upper_right, float halfx,float halfy);
    int stringWidth(const std::string &dat, bool addspace, bool removespace);
    void drawParallelogramLineSegment(const Vector3f &source, const Vector3f &dest, double width, const Polarity::Color&);
public:
    Visualization();
    void draw();
    BrainPlugin::UpdateReturn update();
    void initialize(Brain*b);
    bool setFocus(bool focused);
    void notifyNeuronDestruction(Neuron*n);
    ~Visualization();
    struct Event {
        enum {
            MOUSE_CLICK,
            MOUSE_UP,
            MOUSE_DRAG,
            MOUSE_MOVE,
            KEYBOARD,
            KEYBOARD_UP,
            KEYBOARD_SPECIAL,
            KEYBOARD_SPECIAL_UP
        } event;
        float mouseX;
        float mouseY;
        int button;
        int modCodes;
    };
    void postInputEvent(const Event&evt);
    bool isDetailed(const Neuron*)const;
    bool isSelected(const Neuron*)const;
private:    
    //process a single event and make it adjust the input state machine
    
    std::vector<Event>mInputEvents;
    class Button {
        
        float minX;
        float maxX;
        float minY;
        float maxY;
        float mScale;//how big the text is
        std::string mText;
        std::function<void()> mClick;
        bool renderedOnce;
    public:
        /**
           buttons are referenced from the top left corner... in pixels
         */
        Button(float minX,
               float minY,
               float maxX,
               float maxY,
               const std::string &text,
               const std::function<void()> &click,
               float scale=.1);
        void draw(Visualization * vis);
        bool click(Visualization * vis, const Visualization::Event&evt)const;
        void doClick(Visualization * vis, const Visualization::Event&evt)const;
    };
    class InputStateMachine {
        void drag(Visualization * vis, const Visualization::Event&evt);
        void click(Visualization * vis, const Visualization::Event&evt);
        std::vector<Button> mButtons;
    public:
        bool mActiveDrag;
        float mDragStartX;
        float mDragStartY;
        int mKeyDown[256];
        int mSpecialKeyDown[256];
        int mMouseButtons[5];
        float mMouseX;
        float mMouseY;
        void processPersistentState(const Visualization::Event&evt);
        void processEvent(Visualization*parent, const Visualization::Event&evt);
        InputStateMachine();
        
        void draw(Visualization*parent);
    }mInput;
};
BrainPlugin* makeVisualization(Brain*b);
}
