#include "TauTool/GenTauCandidateCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "k4FWCore/Transformer.h"
#include "k4TauAnalysis/GenTauUtils.h"
#include <cstdint>
#include <cstdlib>
#include <string>

struct GenTauClassifier final
    : k4FWCore::Transformer<tautool::GenTauCandidateCollection(const edm4hep::MCParticleCollection&)> {

  GenTauClassifier(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValue("InputCollection", "MCParticles")},
                    {KeyValue("OutputCollection", "GenTauCandidates")}) {}

  tautool::GenTauCandidateCollection operator()(const edm4hep::MCParticleCollection& mcParticles) const override {
    info() << "Input size " << mcParticles.size() << endmsg;
    auto out = tautool::GenTauCandidateCollection();
    for (const auto& p : mcParticles) {
      if (tautool::gen::isFinalTau(p)) {
        auto cand = out.create();
        cand.setTau(p);
        cand.setCharge(static_cast<int32_t>(p.getCharge()));
        auto decayProducts = tautool::gen::getDecayProducts(p, false);
        for (const auto& prod : decayProducts) {
          if (tautool::gen::isNeutrino(prod)) {
            cand.addToNeutrinos(prod);
          } else {
            cand.addToConstituents(prod);
          }
        }
        if (msgLevel(MSG::DEBUG)) {
          debug() << "Tau " << p.getObjectID().index << ": " << cand.getConstituents().size() << " constituents, "
                  << cand.getNeutrinos().size() << " neutrinos. PDGs:";
          for (const auto& c : cand.getConstituents()) {
            debug() << " " << c.getPDG();
          }
          debug() << endmsg;
        }
      }
    }
    info() << "Found " << out.size() << " gen level Taus" << endmsg;

    return out;
  }
};

DECLARE_COMPONENT(GenTauClassifier)