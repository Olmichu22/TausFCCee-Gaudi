#include "k4TauAnalysis/RecoTauUtils.h"

#include <cstdint>
#include <iostream>

namespace {

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
  using tautool::reco::toPi0DecayMode;

  // --- 1 prong: N photons -> ceil(N / 2) pi0 ---
  if (!check("0 photons", toPi0DecayMode(0), 0)) ++failures;
  if (!check("1 photon (merged pi0)", toPi0DecayMode(1), 1)) ++failures;
  if (!check("2 photons", toPi0DecayMode(2), 1)) ++failures;
  if (!check("3 photons", toPi0DecayMode(3), 2)) ++failures;
  if (!check("4 photons", toPi0DecayMode(4), 2)) ++failures;
  if (!check("9 photons (cap)", toPi0DecayMode(9), 5)) ++failures;

  // --- 3 prongs: 10 + N photons -> 10 + ceil(N / 2) pi0 ---
  if (!check("3 prongs, 0 photons", toPi0DecayMode(10), 10)) ++failures;
  if (!check("3 prongs, 1 photon", toPi0DecayMode(11), 11)) ++failures;
  if (!check("3 prongs, 2 photons", toPi0DecayMode(12), 11)) ++failures;
  if (!check("3 prongs, 3 photons", toPi0DecayMode(13), 12)) ++failures;
  if (!check("3 prongs, 4 photons", toPi0DecayMode(14), 12)) ++failures;
  if (!check("3 prongs, 9 photons", toPi0DecayMode(19), 15)) ++failures;

  // --- Negative codes are passed through unchanged ---
  if (!check("invalid", toPi0DecayMode(-1), -1)) ++failures;
  if (!check("1 prong + neutron", toPi0DecayMode(-20), -20)) ++failures;
  if (!check("3 prongs + neutron", toPi0DecayMode(-21), -21)) ++failures;

  return failures;
}