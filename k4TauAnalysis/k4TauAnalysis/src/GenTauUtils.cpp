#include "k4TauAnalysis/GenTauUtils.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <map>
#include <unordered_set>
namespace {

/// From a tau, iterates over its tau daughters until finds the generatorStatus 2 copy
edm4hep::MCParticle finalTauCopy(const edm4hep::MCParticle& t) {
  constexpr int kMaxDepth = 200;
  auto curr = t;
  for (int depth = 0; depth < kMaxDepth; ++depth) {
    if (tautool::gen::isFinalTau(curr))
      return curr;
    bool foundTauDau = false;
    for (const auto& dau : curr.getDaughters()) {
      if (std::abs(dau.getPDG()) == 15) {
        foundTauDau = true;
        curr = dau;
        break;
      }
    }
    if (!foundTauDau)
      break;
  }
  return curr;
}

// |PDG| codes of charged particles a photon can be radiated from (e, mu, pi+-, K+-, K*+-).
constexpr std::array kChargedRadPdgs{11, 13, 211, 321, 323};
bool isChargedRadPdg(int pdg) { return std::ranges::find(kChargedRadPdgs, std::abs(pdg)) != kChargedRadPdgs.end(); }
// |PDG| codes counted as prongs by the decay-mode ID (pi+-, K+-, K*+-).
// Edit this list to change what counts as a prong.
constexpr std::array kProngPdgs{211, 321, 323};

bool isProng(int pdg) { return std::ranges::find(kProngPdgs, std::abs(pdg)) != kProngPdgs.end(); }

// |PDG| codes not counted as extra neutral.
// Edit this list to change what counts as an extra neutral.
constexpr std::array kNotExtraNeutralPdgs{22, 111, 12, 14, 16};

bool isExtraNeutralPdg(int pdg) {
  return std::ranges::find(kNotExtraNeutralPdgs, std::abs(pdg)) == kNotExtraNeutralPdgs.end();
}

// Self-conjugate states: charge conjugation leaves them unchanged
// (-111 or -223 are not valid PDG codes). Same set as genDecayModes.SELF_CONJUGATE_PDGS.
constexpr std::array kSelfConjugatePdgs{21,  22,  23,  25,       // gluon, gamma, Z, H
                                        111, 113, 130, 221, 223, // pi0, rho0, K0_L, eta, omega
                                        310, 331, 333, 335,      // K0_S, eta', phi, f2'
                                        441, 443, 445};          // eta_c, J/psi, chi_c2

int conjugatePdg(int pdg) {
  const bool selfConjugate = std::ranges::find(kSelfConjugatePdgs, pdg) != kSelfConjugatePdgs.end();
  return selfConjugate ? pdg : -pdg;
}

bool isSimulationSecondary(const edm4hep::MCParticle& p) {
  const int status = p.getGeneratorStatus();
  if (status == 0)
    return true;
  if (status == 1)
    return false;
  return p.isCreatedInSimulation();
}

// Canonical daughter labels -> true mode. Same content as genDecayModes.MODE_TABLE.
// Every key must already be in canonical order, otherwise it can never match.
namespace tm = tautool::gen::truemode;
const std::map<std::vector<int>, int32_t> kModeTable{
    // Leptonic
    {{11}, tm::kE},
    {{13}, tm::kMu},

    // Hadronic, no strangeness
    {{-211}, tm::kPi},
    {{-211, 111}, tm::kPiPi0},
    {{-211, 111, 111}, tm::kPi2Pi0},
    {{-211, 111, 111, 111}, tm::kPi3Pi0},
    {{-211, 111, 111, 111, 111}, tm::kPi3Pi0},
    {{211, -211, -211}, tm::k3Pi},
    {{211, -211, -211, 111}, tm::k3PiPi0},
    {{211, -211, -211, 111, 111}, tm::k3Pi2Pi0},
    {{211, 211, -211, -211, -211}, tm::k5Pi},

    // With kaons
    {{-321}, tm::kK},
    {{-321, 111}, tm::kKPi0},
    {{-321, 111, 111}, tm::kK2Pi0},
    {{-321, 111, 111, 111}, tm::kK2Pi0},
    {{-311, -211}, tm::kK0Pi},
    {{-311, -211, 111}, tm::kK0PiPi0},
    {{-311, -211, 111, 111}, tm::kK0PiPi0},
    {{-321, 211, -211}, tm::kKPiPi},
    {{-321, 211, -211, 111}, tm::kKPiPi},
    {{321, -321, -211}, tm::kKKPi},
    {{321, -321, -211, 111}, tm::kKKPi},
    {{-321, 311}, tm::kKK0},
    {{-321, 311, 111}, tm::kKK0},
    {{310, -211, 130}, tm::kK0K0Pi},
    {{-211, 130, 130}, tm::kK0K0Pi},
    {{311, -311, -211, 111}, tm::kK0K0Pi},
    {{310, 310, -211}, tm::kK0K0Pi},
    {{-311, 211, -211, -211}, tm::kK03Pi},

    // Explicit resonance in the generator record
    {{-321, 223}, tm::kOmega},
    {{223, -211, 111, 111}, tm::kOmega},
    {{223, 211, -211, -211}, tm::kOmega},
    {{221, -211, 111}, tm::kEta},
    {{221, 211, -211, -211}, tm::kEta},
    {{-321, 221}, tm::kEta},
    {{221, -211, 111, 111}, tm::kEta},
    {{-323, 221}, tm::kKstar},
};

} // namespace
namespace tautool::gen {

TauOrigin tauOrigin(const edm4hep::MCParticle& t) {
  TauOrigin origin{};
  std::optional<edm4hep::MCParticle> photon;
  bool passedNonTau = false;
  constexpr int kMaxDepth = 200;
  auto cur = t;
  std::unordered_set<int> seen;

  for (int depth = 0; depth < kMaxDepth; ++depth) {
    auto parents = cur.getParents();
    if (parents.empty())
      break;
    cur = parents[0];
    const auto pdg = cur.getPDG();
    const int idx = cur.getObjectID().index;
    if (!seen.insert(idx).second)
      break;
    if (!passedNonTau) {
      if (std::abs(pdg) == 15) {
        continue;
      } else {
        passedNonTau = true;
        origin.originPDG = pdg;
        if (origin.originPDG == 22)
          photon = cur;
      }
    } else {

      if (pdg == 22 && !photon)
        photon = cur;
      if (std::abs(pdg) == 15) {
        origin.motherTau = finalTauCopy(cur);
        origin.radiativePhoton = photon;
        break;
      }
    }
  }

  return origin;
}

int32_t photonOrigin(const edm4hep::MCParticle& p) {
  if (std::abs(p.getPDG()) != 22)
    return photonorigin::kNotAPhoton;
  if (isSimulationSecondary(p))
    return photonorigin::kSimulation;
  auto cur = p;
  std::unordered_set<int> seen;
  constexpr int kMaxDepth = 200;
  for (int depth = 0; depth < kMaxDepth; ++depth) {
    const auto parents = cur.getParents();
    if (parents.empty())
      break;

    cur = parents[0];
    const int idx = cur.getObjectID().index;
    const int absCurPdg = std::abs(cur.getPDG());
    if (!seen.insert(idx).second)
      break;

    if (absCurPdg == 22)
      continue;
    if (absCurPdg == 111)
      return photonorigin::kPi0;
    if (absCurPdg == 15)
      return photonorigin::kTauFsr;
    if (isChargedRadPdg(absCurPdg))
      return photonorigin::kChargedRad;
    return photonorigin::kOther;
  }
  return photonorigin::kOther;
}

std::vector<int> canonicalDaughters(const edm4hep::MCParticle& tau) {
  std::vector<int> pdgs;
  const bool fold = tau.getCharge() > 0; // tau+ -> tau- convention

  for (const auto& dau : tau.getDaughters()) {
    if (dau.getGeneratorStatus() == 0)
      continue; // Geant4 secondary
    if (isNeutrino(dau))
      continue;
    const int pdg = dau.getPDG();
    if (std::abs(pdg) == 22)
      continue; // photon off the tau: FSR
    pdgs.push_back(fold ? conjugatePdg(pdg) : pdg);
  }

  // Python: sort(key=lambda p: (abs(p), p), reverse=True)
  std::ranges::sort(pdgs, [](int a, int b) {
    if (std::abs(a) != std::abs(b))
      return std::abs(a) > std::abs(b);
    return a > b;
  });
  return pdgs;
}

int32_t trueMode(const std::vector<int>& daughters) {
  const auto it = kModeTable.find(daughters);
  return it != kModeTable.end() ? it->second : truemode::kUnknown;
}

bool isExtraNeutral(const edm4hep::MCParticle& p) { return p.getCharge() == 0 && isExtraNeutralPdg(p.getPDG()); }

std::vector<edm4hep::MCParticle> getDecayProducts(const edm4hep::MCParticle& p, bool excludeNeutrinos) {
  std::vector<edm4hep::MCParticle> decayProducts;
  detail::collectDecayProducts(p, decayProducts, excludeNeutrinos);
  return decayProducts;
}

bool isNeutrino(const edm4hep::MCParticle& p) {
  const int absPdg = std::abs(p.getPDG());
  return (absPdg == 12 || absPdg == 14 || absPdg == 16);
}

int32_t visibleDecayMode(const std::vector<edm4hep::MCParticle>& constituents) {
  int countMuon = 0;
  int countElectron = 0;
  int countPions = 0;
  int countPi0 = 0;
  int countOther = 0;
  double totalCharge = 0.;

  for (const auto& dau : constituents) {
    const auto absPdg = std::abs(dau.getPDG());
    const auto charge = dau.getCharge();
    if (absPdg == 13)
      ++countMuon;
    else if (absPdg == 11)
      ++countElectron;
    else if (isProng(absPdg))
      ++countPions;
    else if (absPdg == 111)
      ++countPi0;
    else if (charge != 0)
      ++countOther;
    totalCharge += charge;
  }

  const long visCharge = std::lround(totalCharge);
  if (countMuon > 0)
    return decaymode::kMuonic;
  if (countElectron > 0)
    return decaymode::kElectronic;
  if (countOther > 0)
    return decaymode::kUnexpected;
  if (std::abs(visCharge) == 1 && countPions == 1)
    return countPi0;
  if (std::abs(visCharge) == 1 && countPions == 3)
    return decaymode::kThreeProngOffset + countPi0;

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