from Gaudi.Configuration import INFO, DEBUG
from Configurables import GenTauClassifier
from k4FWCore import ApplicationMgr, IOSvc
import os
io = IOSvc("IOSvc")
# io.Input = "/pnfs/ciemat.es/data/calice/local/arqolmo/CLDFCCFullSim/Ztt_SM_2M_SigmaZ/out_reco_edm4hep_1.root"
io.Input = os.environ.get(
    "TAU_INPUT",
    "/pnfs/ciemat.es/data/calice/local/arqolmo/CLDFCCFullSim/Ztt_SM_2M_SigmaZ/out_reco_edm4hep_1.root",
)
io.Output = "gentaus.root"

alg = GenTauClassifier("GenTauClassifier",
                       InputCollection="MCParticles",
                       OutputCollection="GenTauCandidates",
                       OutputLevel=DEBUG)

ApplicationMgr(TopAlg=[alg], EvtSel="NONE", EvtMax=1000,
               ExtSvc=[io], OutputLevel=INFO)