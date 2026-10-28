#pragma once
#include "edm4hep/MCParticle.h"
#include <cstdint>
#include <optional>
#include <vector>

namespace tautool::gen {
/// Provenance of a generator-level tau, as returned by tauOrigin.
struct TauOrigin {
  int32_t originPDG = 0;                              ///< PDG of the first non-tau ancestor (23 = Z, 22 = gamma*)
  std::optional<edm4hep::MCParticle> motherTau;       ///< Status-2 copy of the mother tau; empty if primary
  std::optional<edm4hep::MCParticle> radiativePhoton; ///< Photon in tau -> gamma -> tau tau; empty if primary
};

/// Traces where a generator-level tau comes from, separating primary taus from
/// radiative ones (tau -> gamma -> tau tau). Port of tauReco.classify_tau_origin.
///
/// Walks up the ancestors, following the first parent at each step:
///   1. Copies of the tau itself are skipped. The first non-tau ancestor gives
///      originPDG (23 = Z, 22 = gamma*). If it is a photon, it is kept as the
///      candidate radiative photon.
///   2. The walk continues upwards. If another tau shows up, the candidate is
///      secondary: that tau is the mother, and the photon kept on the way (the
///      first one found) is the radiative photon.
/// A tau from gamma* also hangs from a photon, so a photon parent alone does not
/// make a tau secondary: only a tau above the first non-tau ancestor does.
///
/// The walk stops at the first ancestor without parents, on a cycle or after
/// 200 steps. Particles with several parents are followed through their first one.
///
/// @param tau Generator-level tau (typically status 2)
/// @return originPDG always set; motherTau (status-2 copy of the mother) and
///         radiativePhoton set only for secondary taus
TauOrigin tauOrigin(const edm4hep::MCParticle& tau);

/// Canonical list of the tau's direct-daughter PDG codes.
/// Port of genDecayModes.canonical_daughters:
///   - only direct daughters; generatorStatus == 0 (Geant4), neutrinos and
///     photons (tau FSR) are dropped
///   - for a tau+ every code is charge-conjugated, except self-conjugate states
///     (pi0, eta, omega, K0_S, K0_L...), so tau+ and tau- share one label
///   - sorted by (|PDG|, PDG) in descending order
///
/// @param tau Generator-level tau (typically status 2)
/// @return Canonical PDG list, e.g. {211, -211, -211, 111}
std::vector<int> canonicalDaughters(const edm4hep::MCParticle& tau);

/// Classifies where a generator-level photon comes from.
/// Port of tauReco.photon_origin_info (origin code only).
///
/// Rules, in order:
///   - not a photon (|PDG| != 22)                  -> photonorigin::kNotAPhoton
///   - created by the detector simulation          -> photonorigin::kSimulation
///     (generatorStatus 0, or isCreatedInSimulation() for status other than 0/1)
///   - otherwise walk up the ancestors, skipping photon copies, and classify
///     by the first non-photon ancestor:
///       pi0                                       -> photonorigin::kPi0
///       tau                                       -> photonorigin::kTauFsr
///       e, mu, pi+-, K+-, K*+-                    -> photonorigin::kChargedRad
///       anything else (e.g. eta -> gamma gamma)   -> photonorigin::kOther
///   - no ancestor found (no parents, cycle or more than 200 steps)
///                                                 -> photonorigin::kOther
///
/// Only the first parent is followed at each step, as in Python. Particles with
/// several parents are therefore classified by their first one.
///
/// @param p Generator-level particle
/// @return One of the tautool::gen::photonorigin codes
int32_t photonOrigin(const edm4hep::MCParticle& p);

/// True decay-mode code of a canonical daughter list (see canonicalDaughters).
/// Port of genDecayModes.true_mode / MODE_TABLE.
///
/// @param daughters Output of canonicalDaughters
/// @return One of the tautool::gen::truemode codes, truemode::kUnknown if unlisted
int32_t trueMode(const std::vector<int>& daughters);

/// Returns the visible-topology decay mode of a tau from its constituents.
/// Port of the ID logic in tauReco.visTauGen.
///
/// Codes: N = 1 prong + N pi0, 10 + N = 3 prongs + N pi0,
/// -13 = muonic, -11 = electronic, -2 = unexpected charged particle,
/// -1 = no valid assignment (e.g. |visible charge| != 1).
///
/// @param constituents Visible decay products, neutrinos excluded (pi0 unexpanded)
/// @return Decay-mode code
int32_t visibleDecayMode(const std::vector<edm4hep::MCParticle>& constituents);

/// Returns if a particle p is or not an extra neutral (different from 22, 111 and neutrinos)
/// It first checks if charge(p) is 0, and then that it is not in kNotExtraNeutralPdgs set
bool isExtraNeutral(const edm4hep::MCParticle& p);

/// Returns true if `p` is a neutrino or antineutrino (|PDG| 12, 14 or 16).
bool isNeutrino(const edm4hep::MCParticle& p);

/// Returns true if `p` is a tau (|PDG| == 15) with generatorStatus == 2,
/// i.e. the last copy before it decays. Same criterion as findAllGenTaus.
bool isFinalTau(const edm4hep::MCParticle& p);

/// Returns the decay products of a generator-level particle: the leaves of
/// its decay tree. Port of tauReco.get_visible_final_state.
///
/// pi0 are returned unexpanded, as in the decay-mode counting; their photons
/// are one step away via getDaughters(). Rules: see detail::collectDecayProducts.
///
/// @param p                Root of the decay tree (typically a status-2 tau)
/// @param excludeNeutrinos Drop neutrinos (|PDG| 12, 14, 16) from the result
/// @return Leaves of the tree, in depth-first order
std::vector<edm4hep::MCParticle> getDecayProducts(const edm4hep::MCParticle& p, bool excludeNeutrinos = true);
} // namespace tautool::gen

namespace tautool::gen::decaymode {

inline constexpr int32_t kMuonic = -13;          // tau -> mu nu nu
inline constexpr int32_t kElectronic = -11;      // tau -> e nu nu
inline constexpr int32_t kUnexpected = -2;       // unexpected charged particle
inline constexpr int32_t kInvalid = -1;          // no valid assignment
inline constexpr int32_t kThreeProngOffset = 10; // 3 prongs + N pi0 -> 10 + N

} // namespace tautool::gen::decaymode
namespace tautool::gen::detail {

/// Recursive helper behind getDecayProducts: appends the decay products
/// of `p` to `out` instead of returning a new vector.
///
/// Not meant to be called directly; exposed only so it can be unit-tested.
/// Rules, in order:
///   - pi0 (|PDG| == 111) is a leaf, even if it decays in the generator
///   - generatorStatus == 1 or no daughters -> leaf
///     (neutrinos 12/14/16 dropped if excludeNeutrinos)
///   - otherwise recurse into daughters, skipping generatorStatus == 0
///     (Geant4 secondaries)
///
/// Only daughters are status-filtered: `p` itself is never checked.
///
/// @param p                Particle to expand
/// @param out              Accumulator; results are appended, never cleared
/// @param excludeNeutrinos Drop neutrinos from the result
void collectDecayProducts(const edm4hep::MCParticle& p, std::vector<edm4hep::MCParticle>& out, bool excludeNeutrinos);

} // namespace tautool::gen::detail

namespace tautool::gen::truemode {

inline constexpr int32_t kUnknown = -1;

inline constexpr int32_t kE = 0;
inline constexpr int32_t kMu = 1;

inline constexpr int32_t kPi = 10;
inline constexpr int32_t kPiPi0 = 11;
inline constexpr int32_t kPi2Pi0 = 12;
inline constexpr int32_t kPi3Pi0 = 13; // 3 or more pi0
inline constexpr int32_t k3Pi = 14;
inline constexpr int32_t k3PiPi0 = 15;
inline constexpr int32_t k3Pi2Pi0 = 16;
inline constexpr int32_t k5Pi = 17;

inline constexpr int32_t kK = 20;
inline constexpr int32_t kKPi0 = 21;
inline constexpr int32_t kK2Pi0 = 22; // 2 or more pi0
inline constexpr int32_t kK0Pi = 23;
inline constexpr int32_t kK0PiPi0 = 24;
inline constexpr int32_t kKPiPi = 25;
inline constexpr int32_t kKKPi = 26;
inline constexpr int32_t kKK0 = 27;
inline constexpr int32_t kK0K0Pi = 28;
inline constexpr int32_t kK03Pi = 29;

inline constexpr int32_t kOmega = 40;
inline constexpr int32_t kEta = 41;
inline constexpr int32_t kKstar = 42;

} // namespace tautool::gen::truemode

namespace tautool::gen::photonorigin {

inline constexpr int32_t kNotAPhoton = -1;
inline constexpr int32_t kPi0 = 0;
inline constexpr int32_t kTauFsr = 1;
inline constexpr int32_t kChargedRad = 2;
inline constexpr int32_t kOther = 3;
inline constexpr int32_t kSimulation = 4;

} // namespace tautool::gen::photonorigin