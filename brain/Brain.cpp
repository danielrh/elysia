#include "Platform.hpp"
#include "genome.pb.h"
#include "Brain.hpp"
#include "ProteinEnvironment.hpp"
#include "Neuron.hpp"
#include "Synapse.hpp"
#include "SimpleSpatialSearch.hpp"
#include "BrainPlugins.hpp"
#include "BrainPlugin.hpp"

#define INPUT_NEURONS			100
#define INPUT_REGION_MINX		0.0f
#define INPUT_REGION_MAXX		5.0f
#define INPUT_REGION_MINY		0.0f
#define INPUT_REGION_MAXY		5.0f
#define INPUT_REGION_SPACING	0.01f	//This also implies max input neurons of ((max-min)/spacing)^2
#define INPUT_AXON_SPREAD		0.0f	//The range of axon locations from input neurons

#include <boost/math/distributions/uniform.hpp>
#include <boost/random.hpp>

namespace Elysia {

/**
 *  @param ProteinEnvironment *proteinMap - a base protein map for the brain
 *
 *	Description:	Creates a new Brain instance using a defined protein map as a base.
 *					Also adds a new spatial search instance, sets the brain's age to 0 and
 *					initializes all requisite brain plugins
**/
Brain::Brain (ProteinEnvironment *proteinMap){
    mProteinMap=proteinMap;
    mSpatialSearch=new SimpleSpatialSearch;
    mAge=0;
	createInputRegion(INPUT_NEURONS);
    makeInitialNeurons();
    BrainPlugins::constructAll(this).swap(mPlugins);
}

void Brain::syncEnvironmentNeurons(unsigned int index, ProteinEnvironment::iterator environmentIterator){
    Elysia::Genome::Effect growEffect=Elysia::Genome::GROW_NEURON;
    float neuronDensity =environmentIterator.getProteinDensity(growEffect);
    BoundingBox3f3f bounds = environmentIterator.getBoundingBox();
    float area = bounds.across().x*bounds.across().y;
    float expectedNumNeurons=neuronDensity*area;
    boost::minstd_rand randGen(1+index);//needs to be >0
    boost::uniform_real<> uni_dist(0.0f,1.0f);
    boost::variate_generator<boost::minstd_rand, boost::uniform_real<> > uniform(randGen,uni_dist);
    if (expectedNumNeurons-floor(expectedNumNeurons)<uniform()) {
        expectedNumNeurons=ceil(expectedNumNeurons);
    }else {
        expectedNumNeurons=floor(expectedNumNeurons);
    }
    int numNeurons=(unsigned int)expectedNumNeurons;
    if (numNeurons<mNumNeurons[index]) {
        //FIXME: do we want to delete excess neurons
    }
    for (int n=mNumNeurons[index];n<numNeurons;++n) {
        this->addNeuron(bounds, environmentIterator.retrieveGene(growEffect));
    }
    if (mNumNeurons[index]!=numNeurons) {
        fprintf(stderr,"The index count isn't equal to the sync count:\nthis could happen if a neuron lies at the exact edge of a lobe,\nso don't panic, but verify that this is indeed the case\nand the wrong count got updated for that neuron");
    }
}



void Brain::makeInitialNeurons() {
    unsigned int index=0;
    for (ProteinEnvironment::iterator i=mProteinMap->begin();
         i!=mProteinMap->end();
         ++i,++index) {
        mNumNeurons[i.uid()]=0;
        syncEnvironmentNeurons(index,i);
        //FIXME add numNeurons to the count in this protein zone so we can add/remove more later
    }
}

Neuron * Brain::addNeuron(const BoundingBox3f3f&generationArea, const Genome::Gene&gene) {
    Vector3f v = generationArea.min();
    v.x+=generationArea.across().x*(rand()/(double)RAND_MAX);
    v.y+=generationArea.across().y*(rand()/(double)RAND_MAX);
	v.z=0;	 //Fixing z to 0 for now.
    //v.z+=generationArea.across().z*(rand()/(double)RAND_MAX); 
	Neuron *n = new Neuron(this, 2, 3, 4, 1, 1, v, gene);
    mAllNeurons.insert(n);
    return n;
}

/**
 *	Description:	One "tick" of the brain simulation code, where the following occurs:
 *					 - processNeuron() called
 *					 - processSynapes() called
 *					 - increments the current time in the simulation
 *					 - ages the brain
 *					 - updates all the plugins for this brain
**/
void Brain::tick(){
	processNeuron();
	processSynapse();
    ++mCurTime;
    mAge+=1.0e-6;//fixme this is probably not correct: we probably need genes to do this
    if (mAge>1.0) mAge=1.0;
    for(std::vector<BrainPlugin*>::iterator i=mPlugins.begin(),ie=mPlugins.end();i!=ie;++i) {
        (*i)->update();
    }
}


/**
 *	Description:	Updates all neurons in the neuron list for this brain, then clears the list of
 *					active neurons
**/
void Brain::processNeuron(){
	for (std::list<Neuron*>::iterator i=mActiveNeurons.begin(),ie=mActiveNeurons.end();
         i!=ie;
		 ++i) {
        (*i)->tick();
	}
	mActiveNeurons.clear();
}

/**
 *	Description:	Iterates through brain's associates synapses and updates all of them using
 *					the Synapse class residualFire() method
**/
void Brain::processSynapse(){
    std::list<Synapse*>::iterator j,i=mActiveSynapses.begin(),ie=mActiveSynapses.end();
	while (i!=ie) {
        std::list<Synapse*>::iterator j=i++;
        (*j)->residualFire();
	}
}

/**
 *	@param Synapse *inactiveSynapse - synapse to remove
 *
 *	Description:	Removes a synapse from the synapse list and updates it.
**/
void Brain::inactivateSynapse(Synapse *inactiveSynapse){
	mActiveSynapses.erase(inactiveSynapse->mWhere);
	inactiveSynapse->mWhere=activeSynapseListSentinel();
}

/**
 *	@param Neuron *inactiveNeuron - an inactive neuron to remove
 *
 *	Description:	Removes an unused neuron from the neuron list and updates
**/
void Brain::inactivateNeuron(Neuron *inactiveNeuron){
	mActiveNeurons.erase(inactiveNeuron->mWhere);
	inactiveNeuron->mWhere=activeNeuronListSentinel();
}

/**
 *	@param Synapse *activeSynapse - a synapse to activate
 *	@return	the beginning element of the active synapses list for this brain
 *
 *	Description:	Adds a new synapse to the brain (at front of active synapse list)
 *					and returns the front element of the list
**/
std::list<Synapse *>::iterator Brain::activateSynapse(Synapse *activeSynapse){
    mActiveSynapses.push_front(activeSynapse);
    return mActiveSynapses.begin();
}
/**
 *	@param Neuron *activeNeuron - a neuron to activate
 *	@return	the beginning element of the active neuron list for this brain
 *
 *	Description:	Adds a new neuron to the brain (at front of active neuron list)
 *					and returns the front element of the list
**/
std::list<Neuron *>::iterator Brain::activateNeuron(Neuron *activeNeuron){
  activeNeuron->mActivity=5.0f;
  mActiveNeurons.push_front(activeNeuron);
  return mActiveNeurons.begin();
}

/**
 *	Brain destructor
**/
Brain::~Brain() {
	for(std::set<Neuron*>::iterator i=mAllNeurons.begin(),ie=mAllNeurons.end();i!=ie;++i){
		delete *i;
	}
    delete mSpatialSearch;
    delete mProteinMap;
    for (std::vector<BrainPlugin*>::iterator i=mPlugins.begin(),ie=mPlugins.end();i!=ie;++i) {
        delete *i;
    }
}

Neuron* Brain::createInputNeuron(float x, float y, float z, float spread){
	Genome::Gene gene;
    Genome::TemporalBoundingBox *sourcebb=gene.add_bounds();
    Genome::TemporalBoundingBox *targetbb=gene.add_bounds();
    sourcebb->set_minx(0);
    sourcebb->set_miny(0);
    sourcebb->set_minz(0);
    sourcebb->set_maxx(0);
    sourcebb->set_maxy(0);
    sourcebb->set_maxz(0);
    targetbb->set_minx(x-spread);
    targetbb->set_miny(y-spread);
    targetbb->set_minz(0);
    targetbb->set_maxx(x+spread);
    targetbb->set_maxy(y+spread);
    targetbb->set_maxz(0);
	Vector3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	Neuron *n;
	this->mAllNeurons.insert(n = new Neuron(this, 0, 0, 0, 1, 1, v,gene)); 
	return n;
}

void Brain::createInputRegion(int neurons){
	//At present, reserved location of input lobe is <0,0,0> to <5,5,0> neurons are added at 0.01 increments
	float minx = INPUT_REGION_MINX;
	float maxx = INPUT_REGION_MAXX;
	float miny = INPUT_REGION_MINY;
	float maxy = INPUT_REGION_MAXY;
	float spacing = INPUT_REGION_SPACING;
	float spread = INPUT_AXON_SPREAD;

	if(neurons > (maxx-minx)*(maxy-miny)/spacing){
		assert(this);
	}
	Neuron* n;
	float x=minx;
	float y=miny;

	for(int i=0;i<neurons;i++){
		n = createInputNeuron(x,y,0,spread);
		mInputNeurons.push_back(n);
		x += spacing;
		if(x >= maxx){
			x = minx;
			y += spacing;
		}
	}
}

void Brain::fireInputNeuron(int neuronNumber){
	Neuron* n= mInputNeurons[neuronNumber];
	n->setActivity(5);
	this -> activateNeuron(n);
}



/**
 * @return the bounds of all genes in the brain, i.e. where the farthest neurons may spout
 */
BoundingBox3f3f Brain::getBounds()const{
    
    
//    return BoundingBox3f3f(Vector3f(0,00,00),Vector3f(1000,1000,1000));
    return mProteinMap->getBounds();
}

}

