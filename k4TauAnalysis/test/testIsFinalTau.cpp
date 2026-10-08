#include "edm4hep/MCParticleCollection.h"
#include "k4TauAnalysis/GenTauUtils.h"

#include <iostream>

int main() {
  int failures = 0;
  auto coll = edm4hep::MCParticleCollection();

  // Case 1: tau with gen status 2 -> true
  auto tau1 = coll.create();
  tau1.setPDG(15);
  tau1.setGeneratorStatus(2);
  if (!tautool::gen::isFinalTau(tau1)) {
    ++failures;
    std::cerr << "Case 1 failed: tau with gen status 2\n";
  }

  // Case 2: tau with gen status !=2 -> false
  auto tau2 = coll.create();
  tau2.setPDG(15);
  tau2.setGeneratorStatus(1);
  if (tautool::gen::isFinalTau(tau2)) {
    ++failures;
    std::cerr << "Case 2 failed:  tau with gen status != 2\n";
  }
  // Case 3: non-tau particle -> false
  auto pion1 = coll.create();
  pion1.setPDG(211);
  pion1.setGeneratorStatus(1);
  if (tautool::gen::isFinalTau(pion1)) {
    ++failures;
    std::cerr << "Case 3 failed: non-tau particle\n";
  }

  auto pion2 = coll.create();
  pion2.setPDG(211);
  pion2.setGeneratorStatus(2);
  if (tautool::gen::isFinalTau(pion2)) {
    ++failures;
    std::cerr << "Case 4 failed: non-tau particle with gen status 2\n";
  }
  // Case 5: tau+ with gen status 2 -> true
  auto tau3 = coll.create();
  tau3.setPDG(-15);
  tau3.setGeneratorStatus(2);
  if (!tautool::gen::isFinalTau(tau3)) {
    ++failures;
    std::cerr << "Case 5 failed: tau+ with gen status 2\n";
  }
  return failures;
}