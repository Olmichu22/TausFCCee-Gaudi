#include "k4TauAnalysis/GenTauUtils.h"

#include "edm4hep/MCParticleCollection.h"

#include <initializer_list>
#include <iostream>
#include <utility>
#include <vector>

namespace {

// Creates a particle in `coll` with the given PDG and charge
edm4hep::MutableMCParticle make(edm4hep::MCParticleCollection& coll, int pdg, float charge) {
  auto p = coll.create();
  p.setPDG(pdg);
  p.setCharge(charge);
  return p;
}

// Builds the constituents from (pdg, charge) pairs and returns their decay mode
int32_t modeOf(std::initializer_list<std::pair<int, float>> parts) {
  edm4hep::MCParticleCollection coll;
  std::vector<edm4hep::MCParticle> constituents;
  for (const auto& [pdg, charge] : parts) {
    constituents.push_back(make(coll, pdg, charge));
  }
  return tautool::gen::visibleDecayMode(constituents);
}

// Compares the decay mode with the expected value and reports a failure with context
bool check(const char* name, int32_t got, int32_t expected) {
  if (got == expected) {
    return true;
  }
  std::cerr << name << " failed: expected " << expected << ", got " << got << "\n";
  return false;
}

} // namespace

int main() {
  int failures = 0;
  namespace dm = tautool::gen::decaymode;

  // --- Hadronic, 1 prong: code = number of pi0 ---
  if (!check("pi-", modeOf({{-211, -1.f}}), 0))
    ++failures;
  if (!check("pi- pi0", modeOf({{-211, -1.f}, {111, 0.f}}), 1))
    ++failures;
  if (!check("pi+ 2pi0", modeOf({{211, 1.f}, {111, 0.f}, {111, 0.f}}), 2))
    ++failures;
  if (!check("K- (prong)", modeOf({{-321, -1.f}}), 0))
    ++failures;
  if (!check("K*- (prong)", modeOf({{-323, -1.f}}), 0))
    ++failures;

  // --- Hadronic, 3 prongs: code = 10 + number of pi0 ---
  if (!check("3pi", modeOf({{-211, -1.f}, {-211, -1.f}, {211, 1.f}}), dm::kThreeProngOffset))
    ++failures;
  if (!check("3pi pi0", modeOf({{-211, -1.f}, {-211, -1.f}, {211, 1.f}, {111, 0.f}}), dm::kThreeProngOffset + 1))
    ++failures;

  // --- Extra neutrals do not change the ID ---
  if (!check("pi- K0L", modeOf({{-211, -1.f}, {130, 0.f}}), 0))
    ++failures;
  if (!check("pi- photon", modeOf({{-211, -1.f}, {22, 0.f}}), 0))
    ++failures;

  // --- Leptonic ---
  if (!check("mu-", modeOf({{13, -1.f}}), dm::kMuonic))
    ++failures;
  if (!check("e-", modeOf({{11, -1.f}}), dm::kElectronic))
    ++failures;
  if (!check("e+ photon", modeOf({{-11, 1.f}, {22, 0.f}}), dm::kElectronic))
    ++failures;

  // --- Unexpected charged particle ---
  if (!check("proton", modeOf({{2212, 1.f}}), dm::kUnexpected))
    ++failures;

  // --- Priority between rules: mu > e > unexpected > hadronic ---
  if (!check("mu + e", modeOf({{13, -1.f}, {11, -1.f}}), dm::kMuonic))
    ++failures;
  if (!check("e + proton", modeOf({{11, -1.f}, {2212, 1.f}}), dm::kElectronic))
    ++failures;
  if (!check("pi- + proton", modeOf({{-211, -1.f}, {2212, 1.f}}), dm::kUnexpected))
    ++failures;

  // --- No valid assignment ---
  if (!check("empty", modeOf({}), dm::kInvalid))
    ++failures;
  if (!check("visible charge 0", modeOf({{-211, -1.f}, {211, 1.f}}), dm::kInvalid))
    ++failures;
  if (!check("visible charge -2", modeOf({{-211, -1.f}, {-211, -1.f}}), dm::kInvalid))
    ++failures;
  if (!check("5 prongs, charge -1", modeOf({{-211, -1.f}, {-211, -1.f}, {-211, -1.f}, {211, 1.f}, {211, 1.f}}),
             dm::kInvalid))
    ++failures;
  if (!check("only pi0", modeOf({{111, 0.f}}), dm::kInvalid))
    ++failures;

  return failures;
}