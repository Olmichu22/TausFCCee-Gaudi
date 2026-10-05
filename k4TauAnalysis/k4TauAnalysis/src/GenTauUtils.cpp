#include "k4TauAnalysis/GenTauUtils.h"
#include <cstdlib>

namespace tautool::gen {

bool isFinalTau(const edm4hep::MCParticle& p) { return std::abs(p.getPDG()) == 15 && p.getGeneratorStatus() == 2; }
} // namespace tautool::gen