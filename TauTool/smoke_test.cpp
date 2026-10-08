#include <TauTool/GenTauCandidateCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <iostream>
#include <podio/Frame.h>
#include <podio/ROOTWriter.h>
#include <utility> // std::move
int main() {
  // empty collection
  auto tauCollection = tautool::GenTauCandidateCollection();
  auto mcParticlesCollection = edm4hep::MCParticleCollection();
  // Create tau
  auto p1 = mcParticlesCollection.create();
  auto p2 = mcParticlesCollection.create();

  p1.setPDG(15);
  p2.setPDG(211);

  auto tau = tauCollection.create();

  tau.setDecayMode(1);
  tau.setCharge(1);
  tau.setGenMass(0.23);

  tau.setTau(p1);
  tau.addToConstituents(p2);
  for (const auto& t : tauCollection) {
    std::cout << "decaymode =" << t.getDecayMode() << std::endl;
    std::cout << "charge =" << t.getCharge() << std::endl;
    std::cout << "genmass =" << t.getGenMass() << std::endl;
    std::cout << "tau" << t.getTau().getPDG() << std::endl;
    for (const auto& c : t.getConstituents()) {
      std::cout << "Component" << c.getPDG() << std::endl;
    }
  }

  podio::Frame frame;
  frame.put(std::move(tauCollection), "GenTauCandidates");
  frame.put(std::move(mcParticlesCollection), "McParticles");
  podio::ROOTWriter writer("smoke_test.root");
  writer.writeFrame(frame, "events");
  writer.finish();

  return 0;
}