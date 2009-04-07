import ButtonControl;
import Drawable;

class ModeToggleButton extends ButtonControl{
    
    function ModeToggleButton(mc:MovieClip,ul:Point,lr:Point){
    super(mc,ul,lr,"Drag Mode");
    status=Drawable.DRAGMODE;
    }
    var status;
      function onPress():Void {
        if(status == Drawable.DRAGMODE){
            setdraw();
        }
        else if(status == Drawable.DRAWMODE){
            seterase();
        }
        else if(status == Drawable.ERASEMODE){
            setdrag();

        }
      }
      function setdrag(){
            setText("Drag Mode");
            Drawable.mMode=Drawable.DRAGMODE;
            status = Drawable.DRAGMODE;
      }
      function setdraw(){
        Drawable.mMode= Drawable.DRAWMODE;
        setText("Draw Mode");
        status=Drawable.DRAWMODE;
      }
      function seterase(){
            setText("Erase Mode");
            Drawable.mMode=Drawable.ERASEMODE;
            status = Drawable.ERASEMODE;        
      }
}