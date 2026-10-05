from Gaudi.Configuration import INFO
from Configurables import GenTauClassifier
from k4FWCore import ApplicationMgr, IOSvc

io = IOSvc("IOSvc")
io.Input = "/pnfs/ciemat.es/data/calice/local/arqolmo/CLDFCCFullSim/Ztt_SM_2M_SigmaZ/out_reco_edm4hep_1.root"
io.Output = "gentaus.root"

alg = GenTauClassifier("GenTauClassifier",
                       InputCollection="MCParticles",
                       OutputCollection="GenTauCandidates")

ApplicationMgr(TopAlg=[alg], EvtSel="NONE", EvtMax=10,
               ExtSvc=[io], OutputLevel=INFO)