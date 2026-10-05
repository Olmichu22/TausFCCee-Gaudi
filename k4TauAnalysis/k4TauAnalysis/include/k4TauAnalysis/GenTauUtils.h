#pragma once

#include "edm4hep/MCParticle.h"

namespace tautool::gen {

/// Returns true if `p` is a tau (|PDG| == 15) with no tau among its daughters,
/// i.e. the last copy of the tau in the generator history.
bool isFinalTau(const edm4hep::MCParticle& p);

} // namespace tautool::gen