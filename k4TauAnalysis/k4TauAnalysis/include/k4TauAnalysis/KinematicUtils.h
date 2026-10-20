#pragma once
#include "Math/Vector4D.h"

namespace tautool::kin{
/// Distance between two directions in the (theta, phi) plane:
/// sqrt(dtheta^2 + dphi^2), with dphi wrapped to [-pi, pi].
/// Port of myutils.dRAngle. Note: theta, not pseudorapidity.
///
/// @param p4a, p4b Four-vectors; only their directions are used
/// @return Distance [rad]
double deltaRThetaPhi(const ROOT::Math::PxPyPzMVector& p4a, const ROOT::Math::PxPyPzMVector& p4b);
} // namespace tautool::kin
