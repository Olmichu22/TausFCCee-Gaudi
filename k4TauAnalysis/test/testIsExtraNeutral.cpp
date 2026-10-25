#include "k4TauAnalysis/GenTauUtils.h"

#include "edm4hep/MCParticleCollection.h"

#include <iostream>

namespace {

// Builds a single particle with the given PDG and charge and classifies it
bool extraNeutral(int pdg, float charge) {
  edm4hep::MCParticleCollection coll;
  auto p = coll.create();
  p.setPDG(pdg);
  p.setCharge(charge);
  return tautool::gen::isExtraNeutral(p);
}

// Compares the result with the expected value and reports a failure with context
bool check(const char* name, bool got, bool expected) {
  if (got == expected) {
    return true;
  }
  std::cerr << name << " failed: expected " << std::boolalpha << expected << ", got " << got << "\n";
  return false;
}

} // namespace

int main() {
  int failures = 0;

  // --- Neutral and not in the excluded list -> extra neutral ---
  if (!check("K0_L", extraNeutral(130, 0.f), true))
    ++failures;
  if (!check("K0_S", extraNeutral(310, 0.f), true))
    ++failures;
  if (!check("neutron", extraNeutral(2112, 0.f), true))
    ++failures;
  if (!check("anti-neutron", extraNeutral(-2112, 0.f), true))
    ++failures;
  if (!check("Lambda", extraNeutral(3122, 0.f), true))
    ++failures;

  // --- Neutral but excluded (own tagging or counted by the ID) ---
  if (!check("photon", extraNeutral(22, 0.f), false))
    ++failures;
  if (!check("pi0", extraNeutral(111, 0.f), false))
    ++failures;
  if (!check("nu_e", extraNeutral(12, 0.f), false))
    ++failures;
  if (!check("anti-nu_mu", extraNeutral(-14, 0.f), false))
    ++failures;
  if (!check("nu_tau", extraNeutral(16, 0.f), false))
    ++failures;

  // --- Charged -> never an extra neutral ---
  if (!check("pi-", extraNeutral(-211, -1.f), false))
    ++failures;
  if (!check("K+", extraNeutral(321, 1.f), false))
    ++failures;
  if (!check("proton", extraNeutral(2212, 1.f), false))
    ++failures;
  if (!check("electron", extraNeutral(11, -1.f), false))
    ++failures;

  // --- Charge check is independent of the PDG list ---
  // Wrong charge on a PDG that would otherwise qualify: the charge decides
  if (!check("K0_L with charge", extraNeutral(130, 1.f), false))
    ++failures;

  return failures;
}