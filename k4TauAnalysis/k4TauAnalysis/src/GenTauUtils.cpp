#include "k4TauAnalysis/GenTauUtils.h"
#include <cstdlib>

namespace tautool::gen {

bool isFinalTau(const edm4hep::MCParticle& p) { return std::abs(p.getPDG()) == 15 && p.getGeneratorStatus() == 2; }
} // namespace tautool::gen

namespace tautool::gen::detail {

void collectDecayProducts(const edm4hep::MCParticle& p, std::vector<edm4hep::MCParticle>& out, bool excludeNeutrinos) {
  const int absPdg = std::abs(p.getPDG());

  // Rule 1: pi0 -> push_back and return
  if (absPdg == 111) {
    out.push_back(p);
    return;
  }
  // Rule 2: status 1 or no daughters -> leaf
  const int status = p.getGeneratorStatus();
  if (status == 1 || p.getDaughters().empty()) {
    const bool isNeutrino = absPdg == 12 || absPdg == 14 || absPdg == 16;
    if (!(isNeutrino && excludeNeutrinos)) {
      out.push_back(p);
    }
    return;
  }

  // Rule 3: range-for over p.getDaughters(),
  //   skip status 0, recurse passing the same `out`
  for (const auto& dau : p.getDaughters()) {
    if (dau.getGeneratorStatus() != 0) {
      collectDecayProducts(dau, out, excludeNeutrinos);
    }
  }
}

} // namespace tautool::gen::detail

namespace tautool::gen {

std::vector<edm4hep::MCParticle> getDecayProducts(const edm4hep::MCParticle& p, bool excludeNeutrinos) {
  std::vector<edm4hep::MCParticle> decayProducts;
  detail::collectDecayProducts(p, decayProducts, excludeNeutrinos);
  return decayProducts;
}

} // namespace tautool::gen