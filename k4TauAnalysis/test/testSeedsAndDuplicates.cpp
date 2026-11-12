#include "k4TauAnalysis/RecoTauUtils.h"

#include "edm4hep/ReconstructedParticleCollection.h"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

constexpr float kMPi = 0.13957f;

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

// A RecoTauBuild with only its four-vector set: all isDuplicate looks at for now
tautool::reco::RecoTauBuild buildAt(double p, double theta, double phi) {
  tautool::reco::RecoTauBuild b;
  b.p4 = ROOT::Math::PxPyPzMVector(p * std::sin(theta) * std::cos(phi), p * std::sin(theta) * std::sin(phi),
                                   p * std::cos(theta), 0.8);
  return b;
}

bool check(const char* name, bool ok) {
  if (!ok) std::cerr << name << " failed\n";
  return ok;
}

} // namespace

int main() {
  int failures = 0;
  using tautool::reco::isDuplicate;
  using tautool::reco::RecoTauBuild;
  using tautool::reco::RecoTauCuts;
  using tautool::reco::selectSeeds;

  // ---------------- selectSeeds ----------------

  // Mixed event: only charged pions inside the momentum windows, sorted by |p| descending
  {
    edm4hep::ReconstructedParticleCollection coll;
    auto pi5 = make(coll, -211, -1.f, 5.0, 1.0, 0.0, kMPi);
    auto pi20 = make(coll, 211, 1.f, 20.0, 1.5, 1.0, kMPi);
    make(coll, 22, 0.f, 30.0, 1.2, 2.0, 0.f);          // photon: never a seed
    make(coll, -211, -1.f, 0.3, 1.1, 0.5, kMPi);        // below anyP (0.5)
    auto pi10 = make(coll, 211, 1.f, 10.0, 2.0, -1.0, kMPi);
    make(coll, 2112, 0.f, 50.0, 1.3, 0.3, 0.93957f);    // neutron: never a seed

    const auto seeds = selectSeeds(coll, RecoTauCuts{});
    if (!check("default: 3 seeds", seeds.size() == 3)) ++failures;
    if (seeds.size() == 3) {
      if (!check("default: 1st is 20 GeV", seeds[0] == pi20)) ++failures;
      if (!check("default: 2nd is 10 GeV", seeds[1] == pi10)) ++failures;
      if (!check("default: 3rd is 5 GeV", seeds[2] == pi5)) ++failures;
    }

    // Narrower pion window [8, 15] GeV: only the 10 GeV pion
    RecoTauCuts custom;
    custom.pionP.min = 8.0;
    custom.pionP.max = 15.0;
    const auto narrow = selectSeeds(coll, custom);
    if (!check("pion window: 1 seed", narrow.size() == 1)) ++failures;
    if (narrow.size() == 1 && !check("pion window: it is the 10 GeV pion", narrow[0] == pi10)) ++failures;
  }

  // No PFOs -> no seeds
  {
    edm4hep::ReconstructedParticleCollection coll;
    if (!check("empty event: no seeds", selectSeeds(coll, RecoTauCuts{}).empty())) ++failures;
  }

  // ---------------- isDuplicate ----------------

  const double minDR = 0.05;
  const std::vector<RecoTauBuild> none;
  const std::vector<RecoTauBuild> one{buildAt(10.0, 1.0, 0.5)};
  const std::vector<RecoTauBuild> two{buildAt(10.0, 2.0, -1.0), buildAt(10.0, 1.0, 0.5)};

  if (!check("nothing accepted yet: not a duplicate", !isDuplicate(buildAt(10.0, 1.0, 0.5), none, minDR)))
    ++failures;
  if (!check("same direction: duplicate", isDuplicate(buildAt(12.0, 1.01, 0.5), one, minDR))) ++failures;
  if (!check("just outside minDR: not a duplicate", !isDuplicate(buildAt(10.0, 1.06, 0.5), one, minDR)))
    ++failures;
  if (!check("far away: not a duplicate", !isDuplicate(buildAt(10.0, 2.5, 2.0), one, minDR))) ++failures;
  if (!check("close to the 2nd accepted: duplicate", isDuplicate(buildAt(10.0, 1.0, 0.52), two, minDR)))
    ++failures;

  return failures;
}