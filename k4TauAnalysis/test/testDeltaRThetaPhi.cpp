#include "Math/Vector4D.h"
#include "k4TauAnalysis/KinematicUtils.h"

#include <cmath>
#include <iostream>
#include <numbers>

namespace {

constexpr double kTol = 1e-9;

// Builds a 4-vector from spherical coordinates (|p|, theta, phi) and mass
ROOT::Math::PxPyPzMVector fromThetaPhi(double p, double theta, double phi, double m = 0.0) {
  return {p * std::sin(theta) * std::cos(phi), p * std::sin(theta) * std::sin(phi), p * std::cos(theta), m};
}

// Compares two doubles within kTol and reports a failure with context
bool check(const char* name, double got, double expected) {
  if (std::abs(got - expected) < kTol) {
    return true;
  }
  std::cerr << name << " failed: expected " << expected << ", got " << got << "\n";
  return false;
}

} // namespace

int main() {
  int failures = 0;
  using tautool::kin::deltaRThetaPhi;
  constexpr double pi = std::numbers::pi;

  // Case 1: identical vectors -> 0
  {
    const auto a = fromThetaPhi(10.0, 1.0, 0.5);
    if (!check("Case 1 (identical)", deltaRThetaPhi(a, a), 0.0)) {
      ++failures;
    }
  }

  // Case 2: only theta differs -> |dtheta|
  {
    const auto a = fromThetaPhi(10.0, 1.0, 0.5);
    const auto b = fromThetaPhi(10.0, 1.3, 0.5);
    if (!check("Case 2 (dtheta only)", deltaRThetaPhi(a, b), 0.3)) {
      ++failures;
    }
  }

  // Case 3: only phi differs -> |dphi|
  {
    const auto a = fromThetaPhi(10.0, 1.0, 0.5);
    const auto b = fromThetaPhi(10.0, 1.0, 0.9);
    if (!check("Case 3 (dphi only)", deltaRThetaPhi(a, b), 0.4)) {
      ++failures;
    }
  }

  // Case 4: both differ (3-4-5 triangle) -> 0.5
  {
    const auto a = fromThetaPhi(10.0, 1.0, 0.5);
    const auto b = fromThetaPhi(10.0, 1.3, 0.9);
    if (!check("Case 4 (dtheta and dphi)", deltaRThetaPhi(a, b), 0.5)) {
      ++failures;
    }
  }

  // Case 5: phi wraps across +-pi -> 2pi - 6, not 6
  {
    const auto a = fromThetaPhi(10.0, 1.0, 3.0);
    const auto b = fromThetaPhi(10.0, 1.0, -3.0);
    if (!check("Case 5 (phi wrap)", deltaRThetaPhi(a, b), 2 * pi - 6.0)) {
      ++failures;
    }
  }

  // Case 6: symmetric, including the wrap -> d(a, b) == d(b, a)
  {
    const auto a = fromThetaPhi(10.0, 1.0, 3.0);
    const auto b = fromThetaPhi(10.0, 1.2, -3.0);
    if (!check("Case 6 (symmetry)", deltaRThetaPhi(b, a), deltaRThetaPhi(a, b))) {
      ++failures;
    }
  }

  // Case 7: independent of |p| and mass -> only direction matters
  {
    const auto a = fromThetaPhi(1.0, 1.0, 0.5, 0.0);
    const auto b = fromThetaPhi(50.0, 1.3, 0.5, 1.777);
    if (!check("Case 7 (|p| and mass)", deltaRThetaPhi(a, b), 0.3)) {
      ++failures;
    }
  }

  return failures;
}