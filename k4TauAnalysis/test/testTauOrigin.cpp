#include "k4TauAnalysis/GenTauUtils.h"

#include "edm4hep/MCParticleCollection.h"

#include <iostream>
#include <optional>

namespace {

// Creates a particle in `coll` with the given PDG and generator status
edm4hep::MutableMCParticle make(edm4hep::MCParticleCollection& coll, int pdg, int status) {
  auto p = coll.create();
  p.setPDG(pdg);
  p.setGeneratorStatus(status);
  return p;
}

// Links mother and daughter in both directions (tauOrigin walks up the parents)
void link(edm4hep::MutableMCParticle& mother, edm4hep::MutableMCParticle& daughter) {
  mother.addToDaughters(daughter);
  daughter.addToParents(mother);
}

bool checkPdg(const char* name, int32_t got, int32_t expected) {
  if (got == expected) return true;
  std::cerr << name << " failed: originPDG expected " << expected << ", got " << got << "\n";
  return false;
}

// The optional must be empty (primary tau)
bool checkEmpty(const char* name, const std::optional<edm4hep::MCParticle>& got) {
  if (!got) return true;
  std::cerr << name << " failed: expected empty, got PDG " << got->getPDG() << " (index "
            << got->getObjectID().index << ")\n";
  return false;
}

// The optional must hold exactly `expected` (same object, not just same PDG)
bool checkIs(const char* name, const std::optional<edm4hep::MCParticle>& got, const edm4hep::MCParticle& expected) {
  if (got && *got == expected) return true;
  std::cerr << name << " failed: expected index " << expected.getObjectID().index << ", got ";
  if (got) {
    std::cerr << "index " << got->getObjectID().index << "\n";
  } else {
    std::cerr << "empty\n";
  }
  return false;
}

} // namespace

int main() {
  int failures = 0;
  using tautool::gen::tauOrigin;

  // Primary tau from a Z: e+e- -> Z -> tau tau, one copy of each tau before the final one
  {
    edm4hep::MCParticleCollection coll;
    auto beam = make(coll, 11, 4);
    auto z = make(coll, 23, 3);
    link(beam, z);
    for (int pdg : {15, -15}) {
      auto copy = make(coll, pdg, 3);
      auto final = make(coll, pdg, 2);
      link(z, copy);
      link(copy, final);
      const auto origin = tauOrigin(final);
      if (!checkPdg("Z: originPDG", origin.originPDG, 23)) ++failures;
      if (!checkEmpty("Z: motherTau", origin.motherTau)) ++failures;
      if (!checkEmpty("Z: radiativePhoton", origin.radiativePhoton)) ++failures;
    }
  }

  // Primary tau from gamma*: also hangs from a photon, but must NOT be secondary
  {
    edm4hep::MCParticleCollection coll;
    auto beam = make(coll, 11, 4);
    auto gamma = make(coll, 22, 3);
    link(beam, gamma);
    for (int pdg : {15, -15}) {
      auto final = make(coll, pdg, 2);
      link(gamma, final);
      const auto origin = tauOrigin(final);
      if (!checkPdg("gamma*: originPDG", origin.originPDG, 22)) ++failures;
      if (!checkEmpty("gamma*: motherTau", origin.motherTau)) ++failures;
      if (!checkEmpty("gamma*: radiativePhoton", origin.radiativePhoton)) ++failures;
    }
  }

  // Secondary tau, as in test_gen_provenance.py: the final (status 2) primary tau
  // radiates the photon -> mother is that same tau
  {
    edm4hep::MCParticleCollection coll;
    auto z = make(coll, 23, 3);
    auto mother = make(coll, 15, 2);
    auto gamma = make(coll, 22, 3);
    link(z, mother);
    link(mother, gamma);
    for (int pdg : {15, -15}) {
      auto secondary = make(coll, pdg, 2);
      link(gamma, secondary);
      const auto origin = tauOrigin(secondary);
      if (!checkPdg("secondary: originPDG", origin.originPDG, 22)) ++failures;
      if (!checkIs("secondary: motherTau", origin.motherTau, mother)) ++failures;
      if (!checkIs("secondary: radiativePhoton", origin.radiativePhoton, gamma)) ++failures;
    }
    // ...and the mother itself is still primary
    const auto origin = tauOrigin(mother);
    if (!checkPdg("mother of secondary: originPDG", origin.originPDG, 23)) ++failures;
    if (!checkEmpty("mother of secondary: motherTau", origin.motherTau)) ++failures;
  }

  // Secondary tau whose photon hangs from an intermediate copy of the mother:
  //   Z -> tau(3) -> tau(2)   and   tau(3) -> gamma -> tau tau
  // motherTau must be the status-2 copy (finalTauCopy), not the status-3 one
  {
    edm4hep::MCParticleCollection coll;
    auto z = make(coll, 23, 3);
    auto motherCopy = make(coll, 15, 3);
    auto gamma = make(coll, 22, 3);
    auto motherFinal = make(coll, 15, 2);
    auto secondary = make(coll, -15, 2);
    link(z, motherCopy);
    link(motherCopy, gamma); // photon linked first: finalTauCopy must skip it
    link(motherCopy, motherFinal);
    link(gamma, secondary);
    const auto origin = tauOrigin(secondary);
    if (!checkIs("via copy: motherTau is the status-2 copy", origin.motherTau, motherFinal)) ++failures;
    if (!checkIs("via copy: radiativePhoton", origin.radiativePhoton, gamma)) ++failures;
  }

  // The radiative photon has a copy above it: the first photon found going up is kept
  //   mother(2) -> gamma(3) -> gamma(2) -> secondary tau
  {
    edm4hep::MCParticleCollection coll;
    auto z = make(coll, 23, 3);
    auto mother = make(coll, 15, 2);
    auto gammaTop = make(coll, 22, 3);
    auto gammaLow = make(coll, 22, 2);
    auto secondary = make(coll, 15, 2);
    link(z, mother);
    link(mother, gammaTop);
    link(gammaTop, gammaLow);
    link(gammaLow, secondary);
    const auto origin = tauOrigin(secondary);
    if (!checkIs("photon copies: motherTau", origin.motherTau, mother)) ++failures;
    if (!checkIs("photon copies: radiativePhoton is the lowest copy", origin.radiativePhoton, gammaLow)) ++failures;
  }

  // A tau without parents does not crash -> originPDG 0, primary
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    const auto origin = tauOrigin(tau);
    if (!checkPdg("no parents: originPDG", origin.originPDG, 0)) ++failures;
    if (!checkEmpty("no parents: motherTau", origin.motherTau)) ++failures;
  }

  // A cycle of tau copies terminates instead of looping forever
  {
    edm4hep::MCParticleCollection coll;
    auto tau1 = make(coll, 15, 2);
    auto tau2 = make(coll, 15, 3);
    link(tau1, tau2);
    link(tau2, tau1);
    const auto origin = tauOrigin(tau1);
    if (!checkPdg("cycle: originPDG", origin.originPDG, 0)) ++failures;
    if (!checkEmpty("cycle: motherTau", origin.motherTau)) ++failures;
  }

  return failures;
}