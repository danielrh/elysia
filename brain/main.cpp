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
#include "SimpleSpatialSearch.hpp"
#include "SharedLibrary.hpp"
#include "test.hpp"
#include "MainThread.hpp"
#include "BrainPlugins.hpp"
bool loadFile(const char* fileName, Elysia::Genome::Genome &retval) {
    FILE * fp=fopen(fileName,"rb");
    if (!fp) return false;
    fseek(fp,0,SEEK_END);
    size_t fileSize=ftell(fp);
    std::vector<unsigned char> data(fileSize);
    fseek(fp,0,SEEK_SET);
    if (!data.empty()) {
        fread(&*data.begin(),1,fileSize,fp);
    }
    fclose(fp);
    if (data.empty()) return false;
    std::vector<unsigned char> buffer;
    if (fromBase64(buffer,Base64::MemoryReference(&*data.begin(),fileSize))) {
        if (buffer.size()&&retval.ParseFromArray(&*buffer.begin(),buffer.size())) {
            return true;
        }else {
            return retval.ParseFromArray(&*data.begin(),fileSize);
        }
    }else {
        return retval.ParseFromArray(&*data.begin(),fileSize);
    }
}
void nilDestroy() {}
Elysia::SharedLibrary gVis(Elysia::SharedLibrary::prefix()+"vis"+Elysia::SharedLibrary::postfix()+Elysia::SharedLibrary::extension());

Elysia::SharedLibrary gCanvas(Elysia::SharedLibrary::prefix()+"canvas"+Elysia::SharedLibrary::postfix()+Elysia::SharedLibrary::extension());

typedef std::map<std::string,std::shared_ptr<Elysia::SharedLibrary> > DevelopmentPluginMap;
DevelopmentPluginMap gPlugins;
bool loadedVis=false;
bool loadedCanvas=false;

void destroyDevelopmentPlugins() {
    void (*destroy)()=&nilDestroy;
    for (DevelopmentPluginMap::iterator i=gPlugins.begin(),ie=gPlugins.end();i!=ie;++i) {
        destroy=(void(*)())i->second->symbol("destroy");
        if (destroy)
            (*destroy)();            
    }

}

int asyncMain(int argc, char**argv, bool loadvis) {
    void (*destroy)()=&nilDestroy;
    void (*canvasDestroy)()=&nilDestroy;
    if (loadvis) {
        
        if (loadedVis == false || loadedCanvas == false) {
            std::cerr<<"Failed to load vis or canvas library\n";
        }else {
            void (*canvasInit)();
            canvasInit=(void(*)())gCanvas.symbol("init");
            canvasDestroy=(void(*)())gCanvas.symbol("destroy");
            (*canvasInit)();
            void (*init)();
            init=(void(*)())gVis.symbol("init");
            destroy=(void(*)())gVis.symbol("destroy");
            (*init)();
        }
    }
    {
        void (*init)();
        for (DevelopmentPluginMap::iterator i=gPlugins.begin(),ie=gPlugins.end();i!=ie;++i) {
            init=(void(*)())i->second->symbol("init");
            if (init)
                (*init)();            
        }      
    }
    Elysia::BrainPlugins::constructCanvasOrGetLast("", "sdl", 1024, 768);
    Elysia::Vector3f test(0,1,2);
    std::unordered_map<Elysia::String,Elysia::Vector3f> bleh;
    bleh["ahoy"]=test;
    Elysia::Genome::Genome genes;
    if (argc>1) {
		if(0 == strcmp(argv[1],"-test")){
			int retval= runtest();
			//int retval= Elysia::NNTest();
			printf("%d",retval);
            if (destroy)
                (*destroy)();
            if (canvasDestroy)
                (*canvasDestroy)();
            destroyDevelopmentPlugins();
            return retval;
		}
        bool retval=loadFile(argv[1],genes);
        if (!retval) {
            printf("Error loading saved file\n");            
        }else {
            printf("Success loading saved file\n");
        }
    }
    {
        Elysia::Brain brain(&(new Elysia::SimpleProteinEnvironment)->initialize(genes), new Elysia::SimpleSpatialSearch);
        //std::vector<Branch *>BranchestoWipe;
        for (size_t i=0;i<10000;++i) {
            bool should_continue = brain.tick();
            if (!should_continue) {
                break;
            }
        }
    }

	
    (*destroy)();
    destroyDevelopmentPlugins();
    (*canvasDestroy)();
	

	return 0;
}
void asyncMainWrapper(int argc, char**argv, bool loadvis) {
    asyncMain(argc,argv,loadvis);
}
void loadDevelLib(const char*name){
    std::shared_ptr<Elysia::SharedLibrary> item(new Elysia::SharedLibrary(
                                                         Elysia::SharedLibrary::prefix()+name
                                                         +Elysia::SharedLibrary::postfix()+Elysia::SharedLibrary::extension()));
    gPlugins[name]=item;
}
int main(int argc, char **argv) {
    bool loadvis=true;
    loadDevelLib("naive");
    for (int i=0;i<argc;++i) {
        bool foundArg=true;
      if (strcmp(argv[i],"-nogfx")==0) {
        loadvis=false;
      }else if (strncmp(argv[i],"-plugin=",9)==0) {
          loadDevelLib(argv[i]+9);
      }else{
          foundArg=false;
      }
      if (foundArg) {
          for (int j=i;j+1<argc;++j) {
              argv[j]=argv[j+1];              
          }
          --argc;
          --i;
      }
    }
    if (loadvis) {
        loadedCanvas =gCanvas.load();
        loadedVis =gVis.load();
        loadDevelLib("platformer");
    }
    std::vector<std::string> failedPlugins;
    for (DevelopmentPluginMap::iterator i=gPlugins.begin(),ie=gPlugins.end();i!=ie;++i) {
        if (!i->second->load()) {
            failedPlugins.push_back(i->first);
        }
    }
    while(failedPlugins.size()) {
        gPlugins.erase(failedPlugins.back());
        failedPlugins.pop_back();
    }
    std::shared_ptr<std::thread> formerMain;
    if (
#ifdef __APPLE__
	loadvis
#else
    false
#endif
        )

    {
        formerMain=Elysia::MainThread::giveUpMain(std::bind(asyncMainWrapper,argc,argv,true));
        
    }else return asyncMain(argc,argv,loadvis);
    return 0;
}
