#include "k4TauAnalysis/GenTauUtils.h"

#include "edm4hep/MCParticleCollection.h"

#include <iostream>

namespace {

// Creates a particle in `coll` with the given PDG and generator status
edm4hep::MutableMCParticle make(edm4hep::MCParticleCollection& coll, int pdg, int status) {
  auto p = coll.create();
  p.setPDG(pdg);
  p.setGeneratorStatus(status);
  return p;
}

// Links mother and daughter in both directions (photonOrigin walks up the parents)
void link(edm4hep::MutableMCParticle& mother, edm4hep::MutableMCParticle& daughter) {
  mother.addToDaughters(daughter);
  daughter.addToParents(mother);
}

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
  namespace po = tautool::gen::photonorigin;
  using tautool::gen::photonOrigin;

  // Same tree as test_photon_origin in test_gen_provenance.py:
  //   tau -> pi0 -> gamma (gPi0, gPi0b)
  //   tau -> gamma (gFsr)
  //   tau -> pi- -> gamma (gPion, generator) and gamma (gSim, Geant4)
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    auto pi0 = make(coll, 111, 2);
    auto gPi0 = make(coll, 22, 1);
    auto gPi0b = make(coll, 22, 1);
    auto gFsr = make(coll, 22, 1);
    auto pion = make(coll, -211, 1);
    auto gPion = make(coll, 22, 1);
    auto gSim = make(coll, 22, 0);
    link(tau, pi0);
    link(pi0, gPi0);
    link(pi0, gPi0b);
    link(tau, gFsr);
    link(tau, pion);
    link(pion, gPion);
    link(pion, gSim);

    if (!check("photon from pi0", photonOrigin(gPi0), po::kPi0))
      ++failures;
    if (!check("second photon from pi0", photonOrigin(gPi0b), po::kPi0))
      ++failures;
    if (!check("tau FSR", photonOrigin(gFsr), po::kTauFsr))
      ++failures;
    if (!check("radiated by pi-", photonOrigin(gPion), po::kChargedRad))
      ++failures;
    if (!check("Geant4 photon (status 0)", photonOrigin(gSim), po::kSimulation))
      ++failures;
    if (!check("pion is not a photon", photonOrigin(pion), po::kNotAPhoton))
      ++failures;
    if (!check("pi0 is not a photon", photonOrigin(pi0), po::kNotAPhoton))
      ++failures;
  }

  // Photon copies are skipped: tau -> gamma (status 2) -> gamma (status 1)
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    auto gCopy = make(coll, 22, 2);
    auto gFinal = make(coll, 22, 1);
    link(tau, gCopy);
    link(gCopy, gFinal);
    if (!check("photon copy chain", photonOrigin(gFinal), po::kTauFsr))
      ++failures;
  }

  // Radiation from an electron counts as charged radiation
  {
    edm4hep::MCParticleCollection coll;
    auto e = make(coll, 11, 1);
    auto g = make(coll, 22, 1);
    link(e, g);
    if (!check("radiated by e-", photonOrigin(g), po::kChargedRad))
      ++failures;
  }

  // Any other ancestor -> other (eta -> gamma gamma)
  {
    edm4hep::MCParticleCollection coll;
    auto eta = make(coll, 221, 2);
    auto g = make(coll, 22, 1);
    link(eta, g);
    if (!check("photon from eta", photonOrigin(g), po::kOther))
      ++failures;
  }

  // Status other than 0/1: the createdInSimulation flag decides
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    auto gGen = make(coll, 22, 2);
    auto gSimFlag = make(coll, 22, 2);
    gSimFlag.setCreatedInSimulation(true);
    link(tau, gGen);
    link(tau, gSimFlag);
    if (!check("status 2, generator", photonOrigin(gGen), po::kTauFsr))
      ++failures;
    if (!check("status 2, created in simulation", photonOrigin(gSimFlag), po::kSimulation))
      ++failures;
  }

  // A photon without parents does not crash -> other
  {
    edm4hep::MCParticleCollection coll;
    auto g = make(coll, 22, 1);
    if (!check("no parents", photonOrigin(g), po::kOther))
      ++failures;
  }

  // A cycle of photon copies terminates instead of looping forever -> other
  {
    edm4hep::MCParticleCollection coll;
    auto g1 = make(coll, 22, 1);
    auto g2 = make(coll, 22, 2);
    link(g1, g2);
    link(g2, g1);
    if (!check("cycle of photons", photonOrigin(g1), po::kOther))
      ++failures;
  }

  return failures;
}