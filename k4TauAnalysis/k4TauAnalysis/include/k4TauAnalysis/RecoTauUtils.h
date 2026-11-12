#pragma once
#include "Math/Vector4D.h"
#include "edm4hep/ReconstructedParticle.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>
namespace tautool::reco::photonmode {

inline constexpr int32_t kOneProngNeutron = -20;   // 1 prong + neutron (Pandora pi -> n misID)
inline constexpr int32_t kThreeProngNeutron = -21; // 3 prongs + neutron (Pandora pi -> n misID)
inline constexpr int32_t kInvalid = -1;            // no valid assignment
inline constexpr int32_t kThreeProngOffset = 10;   // 3 prongs + N gammas -> 10 + N
inline constexpr int32_t kMaxOneProngPhotons = 9;  // Max photons counting in one prong mode
} // namespace tautool::reco::photonmode
namespace tautool::reco {

/// Momentum interval [min, max] used by the reco tau cuts.
/// Both ends are inclusive; max defaults to +infinity (no upper limit).
struct MomentumRange {
  double min = 0.0;
  double max = std::numeric_limits<double>::infinity();
  bool contains(double p) const { return p >= min && p <= max; }
};

/// Selection applied when building a reco tau around a leading pion.
/// Port of the arguments of tauReco.buildTauFromPion, with an upper momentum
/// limit added per particle type.
struct RecoTauCuts {
  double dRMax = 0.4;           ///< Max (theta, phi) distance to the leading pion [rad] (DRCone)
  MomentumRange anyP{0.5};      ///< Momentum window for any PFO in the cone [GeV]
  MomentumRange photonP{};      ///< Momentum window for photons [GeV]
  MomentumRange pionP{};        ///< Momentum window for charged pions [GeV]
  MomentumRange neutronP{10.0}; ///< Momentum window for neutrons [GeV]
  bool chargeCondition = true;  ///< Require |charge| == 1 for a valid decay mode
};
/// Result of building a reco tau candidate around a leading pion
/// (see buildTauFromPion). Plain values, filled before writing a RecoTauCandidate.
struct RecoTauBuild {
  std::vector<edm4hep::ReconstructedParticle> constituents; ///< Counted PFOs, leading pion first
  int nPions = 0;                                           ///< Charged pions, leading pion included
  int nPhotons = 0;                                         ///< Photons
  int nNeutrons = 0;                                        ///< Neutrons
  int32_t charge = 0;                                       ///< Sum of the constituent charges [e]
  ROOT::Math::PxPyPzMVector p4{};                           ///< Sum of the constituent four-vectors [GeV]
  double maxDR = 0.;                                        ///< Max (theta, phi) distance to the leading pion [rad]
  int32_t photonDecayMode = photonmode::kInvalid;           ///< See photonDecayMode
};
/// Converts the photon-counting decay mode of a reco tau into the pi0-counting
/// encoding used at generator level, so both can be compared directly.
/// Same folding as in TausFCCee (MatchRecoGenTau / plotTausLongResults):
///   - 0..9 (1 prong + N photons)   -> ceil(N / 2)
///   - >= 10 (3 prongs + N photons) -> 10 + ceil((N - 10) / 2)
///   - negative codes (-1 invalid, -20/-21 neutron misID) are returned unchanged
///
/// @param photonDecayMode Decay mode counting photons (RecoTauCandidate::photonDecayMode)
/// @return Decay mode counting pi0, in the gen-level encoding
int32_t toPi0DecayMode(int32_t photonDecayMode);

/// Decay mode of a reco tau candidate counting photons, from the number of each
/// kind of counted constituent. Port of the ID logic in tauReco.buildTauFromPion.
///
/// Rules, in order:
///   - |charge| != 1 and chargeCondition              -> photonmode::kInvalid (-1)
///   - 1 pion, no neutrons                            -> min(nPhotons, 9)
///   - 3 pions, no neutrons                           -> 10 + nPhotons (not capped)
///   - 3 pions with neutrons (Pandora pi -> n misID)  -> photonmode::kThreeProngNeutron (-21)
///   - 1 pion with neutrons (Pandora pi -> n misID)   -> photonmode::kOneProngNeutron (-20)
///   - any other combination                          -> photonmode::kInvalid (-1)
///
/// @param nPions          Number of charged pions, leading pion included
/// @param nPhotons        Number of photons
/// @param nNeutrons       Number of neutrons
/// @param charge          Sum of the constituent charges
/// @param chargeCondition Require |charge| == 1 (as charge_condition in Python)
/// @return Photon-counting decay mode (RecoTauCandidate::photonDecayMode)
int32_t photonDecayMode(int nPions, int nPhotons, int nNeutrons, int charge, bool chargeCondition);

/// Builds a reco tau candidate from the PFOs in a cone around a leading pion.
/// Port of tauReco.buildTauFromPion.
///
/// The leading pion is the first constituent. Every other PFO is considered if
/// its (theta, phi) distance to the leading pion is <= cuts.dRMax and its
/// momentum is inside cuts.anyP. Among those:
///   - electrons and muons are ignored
///   - neutrons inside cuts.neutronP      -> counted as neutrons (Pandora pi -> n misID)
///   - charged pions inside cuts.pionP    -> counted as pions
///   - photons inside cuts.photonP        -> counted as photons
///   - anything else is ignored
/// Counted PFOs are added to the constituents, to the charge and to the
/// four-vector, and update maxDR.
///
/// Difference with Python: if the charge check fails, the kinematics are kept;
/// only photonDecayMode is set to photonmode::kInvalid.
///
/// @param seed Leading charged pion
/// @param pfos All PFOs of the event (the seed itself is skipped)
/// @param cuts Cone size and momentum windows
/// @return Constituents, counters, charge, four-vector, maxDR and photon decay mode
RecoTauBuild buildTauFromPion(const edm4hep::ReconstructedParticle& seed,
                              const edm4hep::ReconstructedParticleCollection& pfos, const RecoTauCuts& cuts);
} // namespace tautool::reco