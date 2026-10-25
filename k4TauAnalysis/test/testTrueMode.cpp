#include "k4TauAnalysis/GenTauUtils.h"

#include "edm4hep/MCParticleCollection.h"

#include <initializer_list>
#include <iostream>
#include <utility>
#include <vector>

namespace {

struct Daughter {
  int pdg;
  int status;
};

// Builds tau -> daughters and returns canonicalDaughters(tau).
// tauPdg = 15 for tau-, -15 for tau+ (charge set accordingly).
std::vector<int> canonicalOf(int tauPdg, std::initializer_list<Daughter> daughters) {
  edm4hep::MCParticleCollection coll;
  auto tau = coll.create();
  tau.setPDG(tauPdg);
  tau.setCharge(tauPdg > 0 ? -1.f : 1.f);
  tau.setGeneratorStatus(2);
  for (const auto& [pdg, status] : daughters) {
    auto d = coll.create();
    d.setPDG(pdg);
    d.setGeneratorStatus(status);
    tau.addToDaughters(d);
    d.addToParents(tau);
  }
  return tautool::gen::canonicalDaughters(tau);
}

void print(const std::vector<int>& v) {
  std::cerr << "{";
  for (std::size_t i = 0; i < v.size(); ++i) {
    std::cerr << (i ? ", " : "") << v[i];
  }
  std::cerr << "}";
}

bool checkList(const char* name, const std::vector<int>& got, const std::vector<int>& expected) {
  if (got == expected)
    return true;
  std::cerr << name << " failed: expected ";
  print(expected);
  std::cerr << ", got ";
  print(got);
  std::cerr << "\n";
  return false;
}

bool checkMode(const char* name, int32_t got, int32_t expected) {
  if (got == expected)
    return true;
  std::cerr << name << " failed: expected " << expected << ", got " << got << "\n";
  return false;
}

} // namespace

int main() {
  int failures = 0;
  namespace tm = tautool::gen::truemode;
  using tautool::gen::trueMode;

  // --- canonicalDaughters ---

  // Order of the daughters in the record does not matter
  {
    const auto a = canonicalOf(15, {{16, 1}, {111, 2}, {-211, 1}, {111, 2}});
    const auto b = canonicalOf(15, {{111, 2}, {16, 1}, {111, 2}, {-211, 1}});
    if (!checkList("ordering (a)", a, {-211, 111, 111}))
      ++failures;
    if (!checkList("ordering (b)", b, a))
      ++failures;
  }
  // Same |PDG|: positive code first (211 before -211)
  if (!checkList("tie on |PDG|", canonicalOf(15, {{-211, 1}, {211, 1}, {-211, 1}, {16, 1}}), {211, -211, -211}))
    ++failures;
  // Geant4 secondaries (status 0) are dropped
  if (!checkList("status 0 dropped", canonicalOf(15, {{16, 1}, {-211, 1}, {2112, 0}}), {-211}))
    ++failures;
  // Photons off the tau (FSR) are dropped
  if (!checkList("FSR photon dropped", canonicalOf(15, {{16, 1}, {-211, 1}, {22, 1}}), {-211}))
    ++failures;
  // Both neutrinos of a leptonic decay are dropped
  if (!checkList("leptonic tau-", canonicalOf(15, {{16, 1}, {-12, 1}, {11, 1}}), {11}))
    ++failures;
  // tau+: conjugated onto the tau- convention...
  if (!checkList("tau+ folded", canonicalOf(-15, {{-16, 1}, {211, 1}, {111, 2}}), {-211, 111}))
    ++failures;
  if (!checkList("leptonic tau+", canonicalOf(-15, {{-16, 1}, {12, 1}, {-11, 1}}), {11}))
    ++failures;
  // ...except self-conjugate states (K0_L stays 130, K0_S stays 310)
  if (!checkList("tau+ self-conjugate", canonicalOf(-15, {{-16, 1}, {-310, 1}, {211, 1}, {130, 1}}), {310, -211, 130}))
    ++failures;
  // No daughters -> empty list
  if (!checkList("no daughters", canonicalOf(15, {}), {}))
    ++failures;

  // --- trueMode ---

  if (!checkMode("e", trueMode({11}), tm::kE))
    ++failures;
  if (!checkMode("pi pi0", trueMode({-211, 111}), tm::kPiPi0))
    ++failures;
  if (!checkMode("3pi", trueMode({211, -211, -211}), tm::k3Pi))
    ++failures;
  if (!checkMode("K0 pi", trueMode({-311, -211}), tm::kK0Pi))
    ++failures;
  if (!checkMode("4 pi0 merged", trueMode({-211, 111, 111, 111, 111}), tm::kPi3Pi0))
    ++failures;
  if (!checkMode("unknown label", trueMode({-4122}), tm::kUnknown))
    ++failures;
  if (!checkMode("non-canonical order", trueMode({111, -211}), tm::kUnknown))
    ++failures;
  if (!checkMode("empty", trueMode({}), tm::kUnknown))
    ++failures;

  // --- Both together, as the plugin uses them ---

  if (!checkMode("tau+ -> pi+ pi0 nu", trueMode(canonicalOf(-15, {{-16, 1}, {211, 1}, {111, 2}})), tm::kPiPi0))
    ++failures;
  if (!checkMode("tau- -> K0 pi nu", trueMode(canonicalOf(15, {{16, 1}, {-211, 1}, {-311, 2}})), tm::kK0Pi))
    ++failures;

  return failures;
}