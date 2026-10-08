#include "k4TauAnalysis/GenTauUtils.h"

#include "edm4hep/MCParticleCollection.h"

#include <iostream>

#include <algorithm>

#include <vector>
namespace {

// Creates a particle in `coll` with the given PDG and generator status
edm4hep::MutableMCParticle make(edm4hep::MCParticleCollection& coll, int pdg, int status) {
  auto p = coll.create();
  p.setPDG(pdg);
  p.setGeneratorStatus(status);
  return p;
}

// Links mother and daughter in both directions (podio does not sync them)
void link(edm4hep::MutableMCParticle& mother, edm4hep::MutableMCParticle& daughter) {
  mother.addToDaughters(daughter);
  daughter.addToParents(mother);
}

bool contains(const std::vector<edm4hep::MCParticle>& v, const edm4hep::MCParticle& p) {
  return std::find(v.begin(), v.end(), p) != v.end();
}
} // namespace

int main() {
  int failures = 0;

  // Case 1: tau -> pi nu, excludeNeutrinos = true -> {pi}
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    auto pi = make(coll, -211, 1);
    auto nu = make(coll, 16, 1);
    bool excludeNeutrinos = true;

    link(tau, pi);
    link(tau, nu);

    const auto result = tautool::gen::getDecayProducts(tau, excludeNeutrinos);
    if (result.size() != 1 || !(result[0] == pi)) {
      ++failures;
      std::cerr << "Case 1 failed: expected {pi}, got " << result.size() << " particles\n";
    }
  }

  // Case 2: same tree, excludeNeutrinos = false -> {pi, nu}
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    auto pi = make(coll, -211, 1);
    auto nu = make(coll, 16, 1);
    bool excludeNeutrinos = false;

    link(tau, pi);
    link(tau, nu);

    const auto result = tautool::gen::getDecayProducts(tau, excludeNeutrinos);
    if (result.size() != 2 || !(contains(result, pi) && contains(result, nu))) {
      ++failures;
      std::cerr << "Case 2 failed: expected {pi, nu}, got " << result.size() << " particles\n";
    }
  }
  // Case 3: tau -> pi pi0(->gg) nu               -> {pi, pi0}
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    auto pi0 = make(coll, 111, 2);
    auto pi = make(coll, 211, 1);
    auto gamma1 = make(coll, 22, 1);
    auto gamma2 = make(coll, 22, 1);
    auto nu = make(coll, 16, 1);
    bool excludeNeutrinos = true;

    link(tau, pi0);
    link(tau, pi);
    link(pi0, gamma1);
    link(pi0, gamma2);
    link(tau, nu);

    const auto result = tautool::gen::getDecayProducts(tau, excludeNeutrinos);
    if (result.size() != 2 || !(contains(result, pi) && contains(result, pi0))) {
      ++failures;
      std::cerr << "Case 3 failed: expected {pi, pi0}, got " << result.size() << " particles\n";
    }
  }
  // Case 4: tau -> rho(status 2) -> pi pi0       -> recurse through rho
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    auto pi0 = make(coll, 111, 2);
    auto pi = make(coll, 211, 1);
    auto gamma1 = make(coll, 22, 1);
    auto gamma2 = make(coll, 22, 1);
    auto rho = make(coll, 213, 2);
    bool excludeNeutrinos = true;

    link(tau, rho);
    link(rho, pi0);
    link(rho, pi);
    link(pi0, gamma1);
    link(pi0, gamma2);

    const auto result = tautool::gen::getDecayProducts(tau, excludeNeutrinos);
    if (result.size() != 2 || !(contains(result, pi) && contains(result, pi0))) {
      ++failures;
      std::cerr << "Case 4 failed: expected {pi, pi0}, got " << result.size() << " particles\n";
    }
  }
  // Case 5: intermediate with a status-0 daughter -> that daughter dropped
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    auto pi = make(coll, 211, 1);
    auto nu = make(coll, 16, 1);
    auto e = make(coll, 11, 0);
    bool excludeNeutrinos = false;

    link(tau, pi);
    link(tau, nu);
    link(tau, e);

    const auto result = tautool::gen::getDecayProducts(tau, excludeNeutrinos);
    if (result.size() != 2 || !(contains(result, pi) && contains(result, nu))) {
      ++failures;
      std::cerr << "Case 5 failed: expected {pi, nu}, got " << result.size() << " particles\n";
    }
  }
  // Case 6: status != 1 and no daughters          -> leaf
  {
    edm4hep::MCParticleCollection coll;
    auto tau = make(coll, 15, 2);
    auto e = make(coll, 11, 2);
    auto nue = make(coll, -12, 2);
    auto nutau = make(coll, 16, 2);
    bool excludeNeutrinos = true;

    link(tau, nue);
    link(tau, nutau);
    link(tau, e);

    const auto result = tautool::gen::getDecayProducts(tau, excludeNeutrinos);
    if (result.size() != 1 || !contains(result, e)) {
      ++failures;
      std::cerr << "Case 6 failed: expected {e}, got " << result.size() << " particles\n";
    }
  }
  return failures;
}