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

        // Get decay products of the candidate
        const auto decayProducts = tautool::gen::getDecayProducts(p, false);

        // Kinematic information and decay classification
        const auto tauP4 = edm4hep::utils::p4(p, edm4hep::utils::UseMass);
        ROOT::Math::PxPyPzMVector visP4{}; // starts at (0,0,0,0)
        double maxDR = 0.0;
        std::vector<edm4hep::MCParticle> comps{};

        // Const filling process
        for (const auto& prod : decayProducts) {
          // Neutrinos
          if (tautool::gen::isNeutrino(prod)) {
            cand.addToNeutrinos(prod);
            // Other constituents
          } else {
            comps.push_back(prod);
            cand.addToConstituents(prod);
            const int32_t photonOriginId = tautool::gen::photonOrigin(prod);
            cand.addToConstituentPhotonOrigin(photonOriginId);
            if (photonOriginId == tautool::gen::photonorigin::kTauFsr) {
              cand.addToFsrPhotons(prod);
            }
            const auto prodP4 = edm4hep::utils::p4(prod, edm4hep::utils::UseMass);
            visP4 += prodP4;
            const auto dR = tautool::kin::deltaRThetaPhi(tauP4, prodP4);
            if (dR > maxDR) {
              maxDR = dR;
            }
            // Extra neutrals filling
            if (tautool::gen::isExtraNeutral(prod)) {
              cand.addToExtraNeutrals(prod);
            }
          }
        }

        const int32_t decayMode = tautool::gen::visibleDecayMode(comps);
        const auto visP = edm4hep::Vector3d{visP4.Px(), visP4.Py(), visP4.Pz()};

        cand.setDecayMode(decayMode);
        cand.setVisMomentum(visP);
        cand.setVisMass(visP4.M());
        cand.setMaxConeAngle(static_cast<float>(maxDR));

        // TrueMode (direct tau daughters)
        const auto dausPdgs = tautool::gen::canonicalDaughters(p);
        cand.setTrueMode(tautool::gen::trueMode(dausPdgs));

        // TauParents
        const auto origin = tautool::gen::tauOrigin(p);
        cand.setOriginPDG(origin.originPDG);
        if (origin.motherTau)
          cand.setMotherTau(*origin.motherTau);
        if (origin.radiativePhoton)
          cand.setRadiativePhoton(*origin.radiativePhoton);

        if (msgLevel(MSG::DEBUG)) {
          debug() << "Tau " << p.getObjectID().index << " : " << cand.getConstituents().size() << " constituents, "
                  << cand.getNeutrinos().size() << " neutrinos. PDGs:";
          for (const auto& c : cand.getConstituents()) {
            debug() << " PDG: " << c.getPDG();
          }
          debug() << ".  Extra Neutrals:";
          for (const auto& extra : cand.getExtraNeutrals()) {
            debug() << " " << extra.getPDG();
          }
          debug() << ". genMass " << cand.getGenMass() << ", visMass " << cand.getVisMass() << ", maxConeAngle "
                  << cand.getMaxConeAngle() << ", decayMode " << cand.getDecayMode() << ", trueMode "
                  << cand.getTrueMode();
          debug() << ", originPDG " << cand.getOriginPDG();
          if (cand.getMotherTau().isAvailable()) {
            debug() << ", secondary: mother tau " << cand.getMotherTau().getObjectID().index;
            if (cand.getRadiativePhoton().isAvailable()) {
              debug() << ", radiative photon " << cand.getRadiativePhoton().getObjectID().index;
            } else {
              debug() << ", no radiative photon";
            }
          } else {
            debug() << ", primary";
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