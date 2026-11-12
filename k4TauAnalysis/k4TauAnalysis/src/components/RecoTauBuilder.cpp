#include "TauTool/RecoTauCandidateCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/utils/kinematics.h"
#include "k4FWCore/Transformer.h"
#include "k4TauAnalysis/RecoTauUtils.h"
#include "k4TauAnalysis/KinematicUtils.h"
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include "Gaudi/Property.h"


struct RecoTauBuilder final
    : k4FWCore::Transformer<tautool::RecoTauCandidateCollection(const edm4hep::ReconstructedParticleCollection&)> {

  RecoTauBuilder(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValue("InputCollection", "PandoraPFOs")},
                    {KeyValue("OutputCollection", "RecoTauCandidates")}) {}

  Gaudi::Property<double> m_dRMax{this, "DRMax", 0.4, "Max (theta, phi) distance to the leading pion [rad]"};
  Gaudi::Property<double> m_minAnyP{this, "MinAnyP", 0.5, "Min general momentum [GeV]"};
  Gaudi::Property<double> m_maxAnyP{this, "MaxAnyP", 1e9, "Max general momentum [GeV]"};
  Gaudi::Property<double> m_minPhotonP{this, "MinPhotonP", 0.0, "Min photon momentum [GeV]"};
  Gaudi::Property<double> m_maxPhotonP{this, "MaxPhotonP", 1e9, "Max photon momentum [GeV]"};
  Gaudi::Property<double> m_minPionP{this, "MinPionP", 0, "Min pion momentum [GeV]"};
  Gaudi::Property<double> m_maxPionP{this, "MaxPionP", 1e9, "Max pion momentum [GeV]"};
  Gaudi::Property<double> m_minNeutronP{this, "MinNeutronP", 10, "Min neutron momentum [GeV]"};
  Gaudi::Property<double> m_maxNeutronP{this, "MaxNeutronP", 1e9, "Max neutron momentum [GeV]"};
  Gaudi::Property<bool> m_chargeCondition{this, "ChargeCondition", true, "Require |charge| == 1"};
  Gaudi::Property<double> m_duplicateDR{this, "DuplicateDR", 0.05, "Max distance to eliminate duplicates in reco candidates"};
  
  tautool::RecoTauCandidateCollection operator()(const edm4hep::ReconstructedParticleCollection& pfos) const override {
  
    tautool::reco::RecoTauCuts cuts;
    cuts.dRMax = m_dRMax;
    cuts.anyP = {m_minAnyP.value(), m_maxAnyP.value()};
    cuts.photonP = {m_minPhotonP.value(), m_maxPhotonP.value()};
    cuts.pionP = {m_minPionP.value(), m_maxPionP.value()};
    cuts.neutronP = {m_minNeutronP.value(), m_maxNeutronP.value()};
    cuts.chargeCondition = m_chargeCondition;
  
  auto out = tautool::RecoTauCandidateCollection();
  const auto seeds = tautool::reco::selectSeeds(pfos, cuts);
  std::vector<tautool::reco::RecoTauBuild> candidates;
  for (const auto& seed : seeds){
    const auto build = tautool::reco::buildTauFromPion(seed, pfos, cuts);
    if (!tautool::reco::isDuplicate(build, candidates, m_duplicateDR)){
      candidates.push_back(build);
    }
    }
  
  for (const auto& tauCand : candidates){
    auto cand = out.create();
    cand.setPhotonDecayMode(tauCand.photonDecayMode);
    cand.setDecayMode(tautool::reco::toPi0DecayMode(tauCand.photonDecayMode));
    cand.setCharge(tauCand.charge);
    const auto p3 = edm4hep::Vector3d{tauCand.p4.Px(), tauCand.p4.Py(), tauCand.p4.Pz()};
    cand.setMomentum(p3);
    cand.setMass(tauCand.p4.M());
    cand.setMaxConeAngle(static_cast<float>(tauCand.maxDR));
    cand.setLeadingPion(tauCand.constituents[0]);
    for (const auto& constituent : tauCand.constituents){
      cand.addToConstituents(constituent);
    }
    if (msgLevel(MSG::DEBUG)) {
    debug() << "Reco tau seeded by PFO " << tauCand.constituents[0].getObjectID().index << ": "
            << tauCand.constituents.size() << " constituents (" << tauCand.nPions << " pions, "
            << tauCand.nPhotons << " photons, " << tauCand.nNeutrons << " neutrons). PDGs:";
    for (const auto& c : tauCand.constituents) {
      debug() << " " << c.getPDG();
    }
    debug() << ". charge " << cand.getCharge() << ", photonDecayMode " << cand.getPhotonDecayMode()
            << ", decayMode " << cand.getDecayMode() << ", P " << tauCand.p4.P() << ", mass "
            << cand.getMass() << ", maxConeAngle " << cand.getMaxConeAngle() << endmsg;
    }
  }
  info() << seeds.size() << " seeds, " << out.size() << " reco tau candidates after duplicate removal" << endmsg;
  return out;
  }
};
DECLARE_COMPONENT(RecoTauBuilder)
