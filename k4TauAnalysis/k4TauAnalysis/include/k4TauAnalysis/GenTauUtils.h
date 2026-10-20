#pragma once
#include "edm4hep/MCParticle.h"
#include <vector>

namespace tautool::gen {
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

inline constexpr int32_t kMuonic = -13;        // tau -> mu nu nu
inline constexpr int32_t kElectronic = -11;    // tau -> e nu nu
inline constexpr int32_t kUnexpected = -2;     // unexpected charged particle
inline constexpr int32_t kInvalid = -1;        // no valid assignment
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