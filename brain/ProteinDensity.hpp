
class ProteinDensity {
    typedef std::map<Elysia::Genome::Protein::Effect,float> ProteinDensityMap;
    
    
    ProteinDensityMap mIntrinsicProteins;
    ProteinEnvironment *mExtrinsicProteins;
public:
  float getProteinDensity(const Elysia::Genome::ProteinEffect&);
  std::vector<ProteinDensityMap::value_type> getCompleteProteinDensity();
};
