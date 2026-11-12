#include "k4TauAnalysis/RecoTauUtils.h"
#include "edm4hep/utils/kinematics.h"
#include "edm4hep/utils/vector_utils.h"
#include "k4TauAnalysis/GenTauUtils.h"
#include "k4TauAnalysis/KinematicUtils.h"
#include "k4TauAnalysis/PdgCodes.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
namespace tautool::reco {
int32_t toPi0DecayMode(int32_t photonDecayMode) {
  if (photonDecayMode < 0)
    return photonDecayMode;

  if (photonDecayMode < photonmode::kThreeProngOffset)
    return (photonDecayMode + 1) / 2;

  return gen::decaymode::kThreeProngOffset + ((photonDecayMode - photonmode::kThreeProngOffset) + 1) / 2;
}

int32_t photonDecayMode(int nPions, int nPhotons, int nNeutrons, int charge, bool chargeCondition) {

  if (std::abs(charge) != 1 && chargeCondition)
    return photonmode::kInvalid;

  if (nPions == 1) {
    if (nNeutrons != 0)
      return photonmode::kOneProngNeutron;
    else {
      return std::min<int>(nPhotons, photonmode::kMaxOneProngPhotons);
    }
  }
  if (nPions == 3) {
    if (nNeutrons != 0)
      return photonmode::kThreeProngNeutron;
    else {
      return photonmode::kThreeProngOffset + nPhotons;
    }
  }

  return photonmode::kInvalid;
}

RecoTauBuild buildTauFromPion(const edm4hep::ReconstructedParticle& seed,
                              const edm4hep::ReconstructedParticleCollection& pfos, const RecoTauCuts& cuts) {
  auto result = RecoTauBuild{};

  auto seedP4 = edm4hep::utils::p4(seed, edm4hep::utils::UseMass);

  result.constituents = {seed};
  result.nPions = 1;
  result.p4 = seedP4;
  float cumCharge = seed.getCharge();

  for (const auto& cand : pfos) {
    if (cand == seed)
      continue;
    const auto candP4 = edm4hep::utils::p4(cand, edm4hep::utils::UseMass);
    const double dR = kin::deltaRThetaPhi(candP4, result.p4);

    if (dR > cuts.dRMax)
      continue;

    const double p = candP4.P();
    if (!cuts.anyP.contains(p))
      continue;

    auto absPdg = std::abs(cand.getPDG());

    if (absPdg == pdgid::kElectron || absPdg == pdgid::kMuon)
      continue;
    else if (absPdg == pdgid::kNeutron && cuts.neutronP.contains(p))
      ++result.nNeutrons;
    else if (absPdg == pdgid::kPiPlus && cuts.pionP.contains(p))
      ++result.nPions;
    else if (absPdg == pdgid::kPhoton && cuts.photonP.contains(p))
      ++result.nPhotons;
    else
      continue;
    if (dR > result.maxDR)
      result.maxDR = dR;

    cumCharge += cand.getCharge();
    result.p4 += candP4;
    result.constituents.push_back(cand);
  }
  result.charge = static_cast<int32_t>(std::lround(cumCharge));
  int32_t photonMode =
      photonDecayMode(result.nPions, result.nPhotons, result.nNeutrons, result.charge, cuts.chargeCondition);
  result.photonDecayMode = photonMode;
  return result;
}
bool isDuplicate(const RecoTauBuild& candidate, const std::vector<RecoTauBuild>& accepted, double minDR) {
  for (const auto& tauCand : accepted) {
    const auto dR = kin::deltaRThetaPhi(candidate.p4, tauCand.p4);
    if (dR < minDR)
      return true;
  }
  return false;
}
std::vector<edm4hep::ReconstructedParticle> selectSeeds(const edm4hep::ReconstructedParticleCollection& pfos,
                                                        const RecoTauCuts& cuts) {
  std::vector<edm4hep::ReconstructedParticle> seeds;
  for (const auto& pfo : pfos) {
    const auto candP4 = edm4hep::utils::p4(pfo, edm4hep::utils::UseMass);
    const double p = candP4.P();
    const auto absPDG = std::abs(pfo.getPDG());
    if (absPDG == pdgid::kPiPlus && cuts.anyP.contains(p) && cuts.pionP.contains(p)) {
      seeds.push_back(pfo);
    }
  }
  std::ranges::sort(seeds, [](const auto& a, const auto& b) {
    return edm4hep::utils::magnitude(a.getMomentum()) > edm4hep::utils::magnitude(b.getMomentum());
  });
  return seeds;
}
} // namespace tautool::reco
