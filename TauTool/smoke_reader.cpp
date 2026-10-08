#include <TauTool/GenTauCandidateCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <iostream>
#include <podio/Frame.h>
#include <podio/ROOTReader.h>

int main() {
  podio::ROOTReader reader;
  reader.openFile("smoke_test.root");

  // TODO: leer la primera entrada de la categoría "events" y
  //       construir con ella un podio::Frame
  podio::Frame entry = reader.readNextEntry("events");
  // TODO: sacar la colección "GenTauCandidates" del frame
  //       usando get<tautool::GenTauCandidateCollection>
  const auto& taucollection = entry.get<tautool::GenTauCandidateCollection>("GenTauCandidates");
  const auto& particlecollection = entry.get<edm4hep::MCParticleCollection>("McParticles");
  // TODO: bucle const auto& imprimiendo decayMode, charge y genMass
  for (const auto& t : taucollection) {
    std::cout << "decaymode =" << t.getDecayMode() << std::endl;
    std::cout << "charge =" << t.getCharge() << std::endl;
    std::cout << "genmass =" << t.getGenMass() << std::endl;
    if (t.getTau().isAvailable()) {
      std::cout << "tau" << t.getTau().getPDG() << std::endl;
      for (const auto& c : t.getConstituents()) {
        std::cout << "Component" << c.getPDG() << std::endl;
      }
    } else {
      std::cout << "tau relation not available" << std::endl;
    }
  }

  return 0;
}