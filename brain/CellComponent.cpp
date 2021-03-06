#include "Platform.hpp"
#include "CellComponent.hpp"
#include "Branch.hpp"
#include "ActivityStats.hpp"
namespace Elysia {

/**
 *	Description:	Instantiates a new CellComponent object with some default values
**/
CellComponent::CellComponent() {
    // These are default values for a generic neuron
	//	
    mActivity = _M_ACTIVITY_DEFAULT_VALUE_;
    mThreshold = _M_THRESHOLD_DEFAULT_VALUE_;
    mDevelopmentStage = _M_DEVELOPMENT_STAGE_DEFAULT_VALUE_;
}

/**
 *	@param float parentRandomBranchDeterminer
 *  @param float parentRandomDepthDeterminer
 *	@param float baseBranchiness
 *	@param float tipBranchiness
 *	@param float treeDepth
 *	@param int depth
 *	
 *	Description:	This takes the genetically determined parameters or the dendritic tree at the neuron's birth
 *					     it creates the initial dendritic structures and makes the terminal synapses
 *						 when triggered through changes in protein level it will modify the dendritic structure
 *						 based on the new parameters (either creating new branches or killing existing ones
**/

/**
*	Description:	Sets the neuron's activity to a value. Presently useful for exciting input neurons
*					Which lack synapses to get excited in the normal way.
**/
void CellComponent::setActivity(float activity){
	mActivity = activity;
}

void CellComponent::syncBranchDensity(float parentRandomBranchDeterminer, float parentRandomDepthDeterminer, 
									  float baseBranchiness, float tipBranchiness, float treeDepth, float baseThreshold, float tipThreshold, int depth) {
    float branchiness;
    if(depth>treeDepth/2+parentRandomDepthDeterminer){
        branchiness = tipBranchiness;
		mThreshold = tipThreshold;
    }
    else{
		branchiness = baseBranchiness;
		mThreshold = baseThreshold;
	}

    assert(branchiness<(1<<23) );//using floats in a manner that will not cooperate with floats after a few millions
    size_t childSize=mChildBranches.size();
    float roundingError=branchiness-floor(branchiness);
    if (parentRandomBranchDeterminer>roundingError) {
       branchiness=ceil(branchiness);
    }else {
       branchiness=floor(branchiness);
    }
    size_t maxDepth=(size_t)floor(parentRandomDepthDeterminer);
    if (parentRandomDepthDeterminer>treeDepth-floor(treeDepth))
      maxDepth=(size_t)ceil(treeDepth);
    if (depth>=(int)maxDepth) {
        branchiness=0;
    }    
    branchiness-=childSize;
    while(branchiness>0){
       Branch *b;
       mChildBranches.push_back(b = new Branch(this));
       if(depth>=(int)maxDepth-1){
            b->growSynapse();
       }
       branchiness-=1;
    }
    while (branchiness<0&&mChildBranches.size()) {
       size_t whichDies=rand()%mChildBranches.size();
       delete mChildBranches[whichDies];
       if (whichDies+1!=mChildBranches.size()) 
           mChildBranches[whichDies]=mChildBranches.back();
       mChildBranches.pop_back();
    }    
    for (size_t i=0;i<mChildBranches.size();++i) {
        mChildBranches[i]->syncBranchDensity(parentRandomBranchDeterminer, parentRandomDepthDeterminer, baseBranchiness, tipBranchiness, treeDepth, baseThreshold, tipThreshold, depth + 1);
    }
}


std::vector<Branch*>::const_iterator CellComponent::childBegin()const{
    return mChildBranches.begin();
}
std::vector<Branch*>::const_iterator CellComponent::childEnd()const{
    return mChildBranches.end();
}

}
