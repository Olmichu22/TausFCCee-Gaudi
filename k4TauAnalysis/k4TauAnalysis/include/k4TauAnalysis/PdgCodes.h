#pragma once

/// PDG Monte Carlo particle numbering scheme codes used by the tau classification.
/// Particle codes are positive; the antiparticle has the opposite sign.
/// Only the codes that appear in the classification logic are listed here.
namespace tautool::pdgid {

// Leptons
inline constexpr int kElectron = 11;
inline constexpr int kNuE = 12;
inline constexpr int kMuon = 13;
inline constexpr int kNuMu = 14;
inline constexpr int kTau = 15;
inline constexpr int kNuTau = 16;

// Gauge bosons
inline constexpr int kPhoton = 22;
inline constexpr int kZ = 23;

// Mesons
inline constexpr int kPi0 = 111;
inline constexpr int kKLong = 130;
inline constexpr int kPiPlus = 211;
inline constexpr int kKPlus = 321;
inline constexpr int kKstarPlus = 323;

} // namespace tautool::pdgid
