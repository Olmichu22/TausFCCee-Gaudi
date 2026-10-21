#include "TauTool/GenTauCandidateCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/utils/kinematics.h"
#include "k4FWCore/Transformer.h"
#include "k4TauAnalysis/GenTauUtils.h"
#include "k4TauAnalysis/KinematicUtils.h"
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

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

        // Tau candidate gen information
        auto cand = out.create();
        cand.setTau(p);
        cand.setCharge(static_cast<int32_t>(p.getCharge()));
        cand.setGenMomentum(p.getMomentum());
        cand.setGenMass(p.getMass());

        const auto decayProducts = tautool::gen::getDecayProducts(p, false);

        const auto tauP4 = edm4hep::utils::p4(p, edm4hep::utils::UseMass);
        ROOT::Math::PxPyPzMVector visP4{}; // starts at (0,0,0,0)
        double maxDR = 0.0;
        std::vector<edm4hep::MCParticle> comps{};
        // Const filling process
        for (const auto& prod : decayProducts) {
          if (tautool::gen::isNeutrino(prod)) {
            cand.addToNeutrinos(prod);
          } else {
            comps.push_back(prod);
            cand.addToConstituents(prod);
            const auto prodP4 = edm4hep::utils::p4(prod, edm4hep::utils::UseMass);
            visP4 += prodP4;
            const auto dR = tautool::kin::deltaRThetaPhi(tauP4, prodP4);
            if (dR > maxDR) {
              maxDR = dR;
            }
          }
        }


        const int32_t decayMode = tautool::gen::visibleDecayMode(comps);
        const auto visP = edm4hep::Vector3d{visP4.Px(), visP4.Py(), visP4.Pz()};
        
        cand.setDecayMode(decayMode);
        cand.setVisMomentum(visP);
        cand.setVisMass(visP4.M());
        cand.setMaxConeAngle(static_cast<float>(maxDR));
        
        if (msgLevel(MSG::DEBUG)) {
          debug() << "Tau " << p.getObjectID().index << " : " << cand.getConstituents().size() << " constituents, "
                  << cand.getNeutrinos().size() << " neutrinos. PDGs:";
          for (const auto& c : cand.getConstituents()) {
            debug() << " " << c.getPDG();
          }
          debug() << ". genMass " << cand.getGenMass() << ", visMass " << cand.getVisMass() << ", maxConeAngle "
                  << cand.getMaxConeAngle() << ", decayMode " << cand.getDecayMode();

          debug() << endmsg;
        }
      }
    }
    info() << "Found " << out.size() << " gen level Taus" << endmsg;

    return out;
  }
};

DECLARE_COMPONENT(GenTauClassifier)