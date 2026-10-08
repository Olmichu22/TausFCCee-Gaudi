#pragma once
#include <vector>
#include "edm4hep/MCParticle.h"

namespace tautool::gen {

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
std::vector<edm4hep::MCParticle> getDecayProducts(const edm4hep::MCParticle& p,
                                                  bool excludeNeutrinos = true);
} // namespace tautool::gen

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
void collectDecayProducts(const edm4hep::MCParticle& p,
                          std::vector<edm4hep::MCParticle>& out,
                          bool excludeNeutrinos);

} // namespace tautool::gen::detail