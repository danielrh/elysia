#ifndef _ELYSIA_BRAIN_HPP_
#define _ELYSIA_BRAIN_HPP_
#include "ProteinDensity.hpp"
#include "ProteinEnvironment.hpp"
#include "NeuronTypes.hpp"
namespace Elysia {
class Neuron;
class Synapse; 
class SpatialSearch;
class BrainPlugin;

class BRAIN_CORE_EXPORT Brain {
    ProteinEnvironment *mProteinMap;
    SpatialSearch *mSpatialSearch;
    ///creature maturity from 0(just born) to 1(adult)
    float mAge;
    std::vector<BrainPlugin*> mPlugins;
    void makeInitialNeurons();
    ///maps the index into the protein environment iterator uid to a currently generated neuron count
    std::unordered_map<unsigned int,int> mNumNeurons;
    unsigned int mEnvironmentOffset;
    ProteinEnvironment::iterator mEnvironmentIteration;
    void syncEnvironmentNeurons(unsigned int index, ProteinEnvironment::iterator environmentIterator);
public:
    void notifyPluginsNeuronDestruction(Neuron * mfd);
    void incrementNumNeurons(ProteinEnvironment::iterator i){
        mNumNeurons[i.uid()]++;
    }
    void decrementNumNeurons(ProteinEnvironment::iterator i){
        if (--mNumNeurons[i.uid()]==0) {
            mNumNeurons.erase(mNumNeurons.find(i.uid()));
        }
    }
    /**
     * Brain constructor
     * \param proteinMap the map of where proteins are being deployed in the brain so that neurons may grow appropriately. Brain takes ownership of pointer and destroys it
     */

	
    Brain (ProteinEnvironment *proteinMap, SpatialSearch* spatialsearch);
    SimTime mCurTime;
	int mDevelopmentCounter;
    float getAge()const{return mAge;}
    std::list<Neuron *>mActiveNeurons;
    std::list<Synapse *>mActiveSynapses;
	std::vector<Neuron *>mInputNeurons;		    //These have no dendritic tree and are directly activated by the world
	std::vector<Neuron *>mOutputNeurons;		//These have no axons and I'll decide how they should behave later
    typedef std::set<Neuron*> NeuronSet;
	NeuronSet mAllNeurons;
	NeuronSet mDevelopingNeurons;
    //typedef std::set<Synapse*> SynapseSet;
	//SynapseSet mAllSynapses;					//Removed due to lack of purpose
    BoundingBox3f3f getBounds()const;
    bool tick();
    bool drawFrame(); // returns false if the program should exit
	void processSynapse();
	void processNeuron();
	void developAllNeurons();
	void processDevelopment();
	void processNeuronDeath();		//Remove Neuron from set of AllNeurons upon death
	void processNeuronMature(Neuron* n);
	void deleteNeuron(Neuron* deletedNeuron);
	void deleteSynapse(Synapse* deletedSynapse);
    Neuron *addNeuron(const BoundingBox3f3f&generationArea, const Genome::Gene&gene, NeuronTypes::InputType neuronType);
    ~Brain();
    std::list<Synapse *>::iterator activateSynapse(Synapse *activeSynapse);
    std::list<Neuron *>::iterator activateNeuron(Neuron *activeNeuron);
    void inactivateSynapse(Synapse *inactiveSynapse);
    void inactivateNeuron(Neuron *inactiveNeuron);
    std::list<Synapse *>::iterator activeSynapseListSentinel(){return mActiveSynapses.end();}
    std::list<Neuron *>::iterator activeNeuronListSentinel(){return mActiveNeurons.end();}
    Neuron* createInputNeuron(float x, float y, float z, float spread, NeuronTypes::InputType neuronType);
	void createInputRegion(int neurons);
	void fireInputNeuron(int neuronNumber);
	ProteinEnvironment* getProteinEnvironment() {
        return mProteinMap;
    }
    SpatialSearch *getSpatialSearch(){
        return mSpatialSearch;
    }
};
}

#endif
