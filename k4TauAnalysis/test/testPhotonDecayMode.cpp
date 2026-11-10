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
  using tautool::reco::photonDecayMode;
  namespace pm = tautool::reco::photonmode;

  // Arguments: nPions, nPhotons, nNeutrons, charge, chargeCondition

  // --- 1 prong, no neutrons: N photons, capped at 9 ---
  if (!check("1 prong, 0 photons", photonDecayMode(1, 0, 0, -1, true), 0)) ++failures;
  if (!check("1 prong, 2 photons", photonDecayMode(1, 2, 0, 1, true), 2)) ++failures;
  if (!check("1 prong, 9 photons", photonDecayMode(1, 9, 0, -1, true), 9)) ++failures;
  if (!check("1 prong, 10 photons (capped)", photonDecayMode(1, 10, 0, -1, true), 9)) ++failures;
  if (!check("1 prong, 15 photons (capped)", photonDecayMode(1, 15, 0, -1, true), 9)) ++failures;

  // --- 3 prongs, no neutrons: 10 + N photons, NOT capped ---
  if (!check("3 prongs, 0 photons", photonDecayMode(3, 0, 0, -1, true), 10)) ++failures;
  if (!check("3 prongs, 2 photons", photonDecayMode(3, 2, 0, 1, true), 12)) ++failures;
  if (!check("3 prongs, 12 photons (no cap)", photonDecayMode(3, 12, 0, -1, true), 22)) ++failures;

  // --- Neutrons (Pandora pi -> n misID) ---
  if (!check("1 prong + neutron", photonDecayMode(1, 0, 1, -1, true), pm::kOneProngNeutron)) ++failures;
  if (!check("1 prong + neutron + photons", photonDecayMode(1, 2, 2, -1, true), pm::kOneProngNeutron)) ++failures;
  if (!check("3 prongs + neutron", photonDecayMode(3, 0, 1, -1, true), pm::kThreeProngNeutron)) ++failures;

  // --- Invalid prong multiplicities ---
  if (!check("2 prongs", photonDecayMode(2, 0, 0, -1, true), pm::kInvalid)) ++failures;
  if (!check("4 prongs", photonDecayMode(4, 0, 0, -1, true), pm::kInvalid)) ++failures;
  if (!check("5 prongs, charge -1", photonDecayMode(5, 0, 0, -1, true), pm::kInvalid)) ++failures;
  if (!check("0 prongs", photonDecayMode(0, 2, 0, -1, true), pm::kInvalid)) ++failures;

  // --- Charge condition ---
  if (!check("charge 0, condition on", photonDecayMode(1, 0, 0, 0, true), pm::kInvalid)) ++failures;
  if (!check("charge 3, condition on", photonDecayMode(3, 0, 0, 3, true), pm::kInvalid)) ++failures;
  if (!check("charge 3, condition off", photonDecayMode(3, 0, 0, 3, false), 10)) ++failures;
  if (!check("charge 0, condition off", photonDecayMode(1, 1, 0, 0, false), 1)) ++failures;
  // The charge check comes first: neutrons do not rescue a wrong charge
  if (!check("charge 0 + neutron, condition on", photonDecayMode(1, 0, 1, 0, true), pm::kInvalid)) ++failures;

  return failures;
}