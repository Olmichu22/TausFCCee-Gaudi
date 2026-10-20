#include "k4TauAnalysis/KinematicUtils.h"
#include <cmath>
#include <numbers>

namespace tautool::kin{

double deltaRThetaPhi(const ROOT::Math::PxPyPzMVector& p4a, const ROOT::Math::PxPyPzMVector& p4b){
  constexpr double pi = std::numbers::pi;
  const double dtheta = p4a.Theta() - p4b.Theta();
  double dphi = p4a.Phi() - p4b.Phi();
  if (dphi > pi) dphi -=2 * pi;
  if (dphi < -pi) dphi +=2 * pi;
  
  return std::hypot(dtheta, dphi);
}
} // namespace tautool::kin
