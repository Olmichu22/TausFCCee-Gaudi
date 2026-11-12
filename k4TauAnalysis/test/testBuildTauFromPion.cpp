#include "k4TauAnalysis/RecoTauUtils.h"

#include "edm4hep/ReconstructedParticleCollection.h"

#include <cmath>
#include <cstdint>
#include <iostream>

namespace {

constexpr double kTol = 1e-6;
constexpr double kTheta = 1.2; // seed direction
constexpr double kPhi = 0.5;

// Creates a PFO with the given PDG, charge, |p|, direction and mass
edm4hep::MutableReconstructedParticle make(edm4hep::ReconstructedParticleCollection& coll, int pdg, float charge,
                                           double p, double theta, double phi, float mass) {
  auto pfo = coll.create();
  pfo.setPDG(pdg);
  pfo.setCharge(charge);
  pfo.setMomentum(edm4hep::Vector3f{static_cast<float>(p * std::sin(theta) * std::cos(phi)),
                                    static_cast<float>(p * std::sin(theta) * std::sin(phi)),
                                    static_cast<float>(p * std::cos(theta))});
  pfo.setMass(mass);
  return pfo;
}

constexpr float kMPi = 0.13957f;
constexpr float kMN = 0.93957f;

bool checkInt(const char* name, long got, long expected) {
  if (got == expected)
    return true;
  std::cerr << name << " failed: expected " << expected << ", got " << got << "\n";
  return false;
}

bool checkNear(const char* name, double got, double expected, double tol = kTol) {
  if (std::abs(got - expected) < tol)
    return true;
  std::cerr << name << " failed: expected " << expected << ", got " << got << "\n";
  return false;
}

} // namespace

int main() {
  int failures = 0;
  using tautool::reco::buildTauFromPion;
  using tautool::reco::RecoTauCuts;
  namespace pm = tautool::reco::photonmode;
  const RecoTauCuts cuts; // defaults: dRMax 0.4, any p >= 0.5, neutrons p >= 10

  // Seed alone: one constituent, 1 prong with 0 photons
  {
    edm4hep::ReconstructedParticleCollection coll;
    auto seed = make(coll, -211, -1.f, 10.0, kTheta, kPhi, kMPi);
    const auto r = buildTauFromPion(seed, coll, cuts);
    if (!checkInt("seed alone: constituents", r.constituents.size(), 1))
      ++failures;
    if (!checkInt("seed alone: seed first", r.constituents[0] == seed, 1))
      ++failures;
    if (!checkInt("seed alone: nPions", r.nPions, 1))
      ++failures;
    if (!checkInt("seed alone: charge", r.charge, -1))
      ++failures;
    if (!checkInt("seed alone: mode", r.photonDecayMode, 0))
      ++failures;
    if (!checkNear("seed alone: maxDR", r.maxDR, 0.0))
      ++failures;
    if (!checkNear("seed alone: mass", r.p4.M(), kMPi, 1e-4))
      ++failures;
  }

  // Seed + 2 photons inside the cone -> 1 prong + 2 photons
  {
    edm4hep::ReconstructedParticleCollection coll;
    auto seed = make(coll, -211, -1.f, 10.0, kTheta, kPhi, kMPi);
    make(coll, 22, 0.f, 5.0, kTheta + 0.05, kPhi, 0.f);
    make(coll, 22, 0.f, 4.0, kTheta, kPhi + 0.05, 0.f);
    const auto r = buildTauFromPion(seed, coll, cuts);
    if (!checkInt("2 photons: constituents", r.constituents.size(), 3))
      ++failures;
    if (!checkInt("2 photons: nPhotons", r.nPhotons, 2))
      ++failures;
    if (!checkInt("2 photons: charge", r.charge, -1))
      ++failures;
    if (!checkInt("2 photons: mode", r.photonDecayMode, 2))
      ++failures;
  }

  // maxDR: a single photon at dtheta = 0.1 from the seed
  {
    edm4hep::ReconstructedParticleCollection coll;
    auto seed = make(coll, -211, -1.f, 10.0, kTheta, kPhi, kMPi);
    make(coll, 22, 0.f, 5.0, kTheta + 0.1, kPhi, 0.f);
    const auto r = buildTauFromPion(seed, coll, cuts);
    if (!checkNear("maxDR", r.maxDR, 0.1, 1e-5))
      ++failures;
  }

  // PFOs that must be skipped: outside the cone, below anyP, leptons, other species
  {
    edm4hep::ReconstructedParticleCollection coll;
    auto seed = make(coll, -211, -1.f, 10.0, kTheta, kPhi, kMPi);
    make(coll, 22, 0.f, 5.0, kTheta + 1.0, kPhi, 0.f);         // outside the cone
    make(coll, 22, 0.f, 0.3, kTheta + 0.05, kPhi, 0.f);        // below anyP (0.5)
    make(coll, 11, -1.f, 5.0, kTheta + 0.05, kPhi, 0.000511f); // electron
    make(coll, 13, -1.f, 5.0, kTheta + 0.05, kPhi, 0.10566f);  // muon
    make(coll, 130, 0.f, 5.0, kTheta + 0.05, kPhi, 0.49761f);  // K0_L: not counted
    const auto r = buildTauFromPion(seed, coll, cuts);
    if (!checkInt("skipped: constituents", r.constituents.size(), 1))
      ++failures;
    if (!checkInt("skipped: nPhotons", r.nPhotons, 0))
      ++failures;
    if (!checkInt("skipped: charge", r.charge, -1))
      ++failures;
    if (!checkInt("skipped: mode", r.photonDecayMode, 0))
      ++failures;
  }

  // Neutrons: below neutronP (10) ignored, above counted -> 1 prong + neutron
  {
    edm4hep::ReconstructedParticleCollection coll;
    auto seed = make(coll, -211, -1.f, 10.0, kTheta, kPhi, kMPi);
    make(coll, 2112, 0.f, 5.0, kTheta + 0.05, kPhi, kMN);   // ignored
    make(coll, -2112, 0.f, 20.0, kTheta, kPhi + 0.05, kMN); // counted (anti-neutron too)
    const auto r = buildTauFromPion(seed, coll, cuts);
    if (!checkInt("neutron: nNeutrons", r.nNeutrons, 1))
      ++failures;
    if (!checkInt("neutron: constituents", r.constituents.size(), 2))
      ++failures;
    if (!checkInt("neutron: mode", r.photonDecayMode, pm::kOneProngNeutron))
      ++failures;
  }

  // 3 prongs: pi- pi- pi+ -> mode 10, charge -1
  {
    edm4hep::ReconstructedParticleCollection coll;
    auto seed = make(coll, -211, -1.f, 10.0, kTheta, kPhi, kMPi);
    make(coll, -211, -1.f, 6.0, kTheta + 0.05, kPhi, kMPi);
    make(coll, 211, 1.f, 4.0, kTheta, kPhi + 0.05, kMPi);
    const auto r = buildTauFromPion(seed, coll, cuts);
    if (!checkInt("3 prongs: nPions", r.nPions, 3))
      ++failures;
    if (!checkInt("3 prongs: charge", r.charge, -1))
      ++failures;
    if (!checkInt("3 prongs: mode", r.photonDecayMode, pm::kThreeProngOffset))
      ++failures;
  }

  // Wrong charge: kinematics kept, only the mode is invalid
  {
    edm4hep::ReconstructedParticleCollection coll;
    auto seed = make(coll, -211, -1.f, 10.0, kTheta, kPhi, kMPi);
    make(coll, -211, -1.f, 6.0, kTheta + 0.05, kPhi, kMPi);
    const auto r = buildTauFromPion(seed, coll, cuts);
    if (!checkInt("wrong charge: charge", r.charge, -2))
      ++failures;
    if (!checkInt("wrong charge: constituents kept", r.constituents.size(), 2))
      ++failures;
    if (!checkInt("wrong charge: mode", r.photonDecayMode, pm::kInvalid))
      ++failures;
    if (!checkInt("wrong charge: p4 kept", r.p4.P() > 15.0, 1))
      ++failures;
  }

  // Custom photon window [2, 6] GeV: 1 GeV and 8 GeV photons rejected, 4 GeV accepted
  {
    edm4hep::ReconstructedParticleCollection coll;
    auto seed = make(coll, -211, -1.f, 10.0, kTheta, kPhi, kMPi);
    make(coll, 22, 0.f, 1.0, kTheta + 0.05, kPhi, 0.f);
    make(coll, 22, 0.f, 8.0, kTheta, kPhi + 0.05, 0.f);
    make(coll, 22, 0.f, 4.0, kTheta - 0.05, kPhi, 0.f);
    RecoTauCuts custom;
    custom.photonP.min = 2.0;
    custom.photonP.max = 6.0;
    const auto r = buildTauFromPion(seed, coll, custom);
    if (!checkInt("photon window: nPhotons", r.nPhotons, 1))
      ++failures;
    if (!checkInt("photon window: mode", r.photonDecayMode, 1))
      ++failures;
  }

  return failures;
}