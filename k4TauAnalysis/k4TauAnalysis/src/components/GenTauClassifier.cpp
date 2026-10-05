#include "k4FWCore/Transformer.h"
#include "edm4hep/MCParticleCollection.h"
#include "TauTool/GenTauCandidateCollection.h"
#include "k4TauAnalysis/GenTauUtils.h"
#include <cstdint>
#include <string>

struct GenTauClassifier final
    : k4FWCore::Transformer<tautool::GenTauCandidateCollection(const edm4hep::MCParticleCollection&)> {

  GenTauClassifier(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc,
                    {KeyValue("InputCollection", "MCParticles")},
                    {KeyValue("OutputCollection", "GenTauCandidates")}) {}

  tautool::GenTauCandidateCollection operator()(const edm4hep::MCParticleCollection& mcParticles) const override {
    info() << "Input size " << mcParticles.size() << endmsg;
    auto out = tautool::GenTauCandidateCollection();
    for (const auto& p : mcParticles){
        if (tautool::gen::isFinalTau(p)){
          auto cand = out.create();
          cand.setTau(p);
          cand.setCharge(static_cast<int32_t>(p.getCharge()));
        }
      
    }
    info() << "Found " << out.size() << " gen level Taus" << endmsg;

    return out;
  }
};

DECLARE_COMPONENT(GenTauClassifier)