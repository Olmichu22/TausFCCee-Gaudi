#include "k4TauAnalysis/RecoTauUtils.h"
#include "k4TauAnalysis/PdgCodes.h"
#include "k4TauAnalysis/GenTauUtils.h" 
#include <algorithm>
#include <cstdlib>

namespace tautool::reco {
  int32_t toPi0DecayMode(int32_t photonDecayMode){
    if (photonDecayMode<0) return photonDecayMode;
    
    if (photonDecayMode<photonmode::kThreeProngOffset) return (photonDecayMode + 1)/2;
    
    return gen::decaymode::kThreeProngOffset + ((photonDecayMode-photonmode::kThreeProngOffset) +1)/2;
  }


  int32_t photonDecayMode(int nPions, int nPhotons, int nNeutrons, int charge, bool chargeCondition){
    
    if (std::abs(charge)!=1 && chargeCondition) return photonmode::kInvalid;

    if (nPions==1){
      if (nNeutrons!=0) return photonmode::kOneProngNeutron;
      else {
        return std::min<int>(nPhotons, photonmode::kMaxOneProngPhotons);
      }
    }
    if (nPions==3){
      if (nNeutrons!=0) return photonmode::kThreeProngNeutron;
      else {
        return photonmode::kThreeProngOffset + nPhotons;
      }
    } 

    return photonmode::kInvalid;
  }

} // namespace tautool::reco
