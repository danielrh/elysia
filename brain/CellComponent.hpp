
class CellComponent{
    float activity;                   
    float threshold;                  //How much activity is required to fire
    int stage;                        //0 = development, 1=mature
    void strengthen(float reward); //for punishment, use negative reward
 
};