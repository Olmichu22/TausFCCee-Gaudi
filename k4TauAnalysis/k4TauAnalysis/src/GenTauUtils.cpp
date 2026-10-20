#include "k4TauAnalysis/GenTauUtils.h"
#include <cstdlib>
#include <algorithm>
#include <array>
#include <cmath>
namespace {
// |PDG| codes counted as prongs by the decay-mode ID (pi+-, K+-, K*+-).
// Edit this list to change what counts as a prong.
constexpr std::array kProngPdgs{211, 321, 323};

bool isProng(int pdg) {
  return std::ranges::find(kProngPdgs, std::abs(pdg)) != kProngPdgs.end();
}

} // namespace
namespace tautool::gen {

int32_t visibleDecayMode(const std::vector<edm4hep::MCParticle>& constituents){
  int countMuon = 0;
  int countElectron = 0;
  int countPions = 0;
  int countPi0 = 0;
  int countOther = 0;
  double totalCharge = 0.;

  for (const auto& dau : constituents){
    const auto absPdg = std::abs(dau.getPDG());
    const auto charge = dau.getCharge();
    if (absPdg==13) ++countMuon;
    else if (absPdg==11) ++countElectron;
    else if (isProng(absPdg)) ++countPions;
    else if (absPdg==111) ++countPi0;
    else if (charge!=0) ++countOther;
    totalCharge+=charge;
  }

  const long visCharge = std::lround(totalCharge);
  if (countMuon > 0) return decaymode::kMuonic;
  if (countElectron > 0) return decaymode::kElectronic;
  if (countOther > 0) return decaymode::kUnexpected;
  if (std::abs(visCharge) == 1 && countPions == 1) return countPi0;
  if (std::abs(visCharge) == 1 && countPions == 3) return decaymode::kThreeProngOffset + countPi0;

  return decaymode::kInvalid;
}

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
    if (!(isNeutrino(p) && excludeNeutrinos)) {
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

bool isNeutrino(const edm4hep::MCParticle& p) {
  const int absPdg = std::abs(p.getPDG());
  return (absPdg == 12 || absPdg == 14 || absPdg == 16);
}

} // namespace tautool::gen