#include "Platform.hpp"
#include "genome.pb.h"
#include "CellComponent.hpp"
#include "ActivityStats.hpp"
#include "Neuron.hpp"
#include "Branch.hpp"
#include "Synapse.hpp"
#include "Base64.hpp"
#include "Brain.hpp"
#include "SimpleProteinEnvironment.hpp"
#include "genome.pb.h"

namespace Elysia {
void testTwoConnectedNeurons() {
    ProteinEnvironment *myProteinEnvironment= new SimpleProteinEnvironment();

	Brain *brain= new Brain(myProteinEnvironment);
	for(float i=0;i<2;i++){
        Genome::Gene gene;//FIXME set source and target regions to match the desired behavior
        Genome::TemporalBoundingBox *sourcebb=gene.add_bounds();
        Genome::TemporalBoundingBox *targetbb=gene.add_bounds();
        sourcebb->set_minx(i);
        sourcebb->set_miny(i);
        sourcebb->set_minz(i);

        sourcebb->set_maxx(i);
        sourcebb->set_maxy(i);
        sourcebb->set_maxz(i);

        targetbb->set_minx(1-i);
        targetbb->set_miny(1-i);
        targetbb->set_minz(1-i);

        targetbb->set_maxx(1-i);
        targetbb->set_maxy(1-i);
        targetbb->set_maxz(1-i);

        

		Vector3f v;
		v.x = i;
		v.y = i;
		v.z = i;
		Neuron *n;
		brain->mAllNeurons.insert(n = new Neuron(brain, 2, 3, 4, v,gene)); 
        //n->connect();
		//const Vector3f &location):  mNeuronLocation(location){));
	}

}
}


int runtest(){
    Elysia::testTwoConnectedNeurons();
	printf("puppies");
	getchar();
	return 1;
	
}