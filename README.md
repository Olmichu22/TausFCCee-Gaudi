# TausFCCee-Gaudi

Native Gaudi/key4hep port of the tau reconstruction and classification analysis in
[TausFCCee](https://github.com/Olmichu22/TausFCCee) (Python), for the CLD detector at FCC-ee.

## Layout

The repository holds two independent CMake projects:

| Directory | Contents |
| --- | --- |
| `TauTool/` | podio datamodel (`tautool.yaml`) defining `tautool::GenTauCandidate`, a generator-level tau with its decay topology, kinematics and relations to `edm4hep::MCParticle`. Builds `libTauTool` and its ROOT dictionary `TauToolDict`. |
| `k4TauAnalysis/` | Gaudi algorithms, created from `k4-project-template`. Consumes `TauTool` via `find_package`. |

Inside `k4TauAnalysis/`:

- `TauClassification`: shared library without Gaudi dependencies (`GenTauUtils.h`, namespace
  `tautool::gen`) with the generator-level helpers: `isFinalTau`, `isNeutrino`, `getDecayProducts`.
- `k4TauAnalysisPlugins`: Gaudi plugin with `GenTauClassifier`, a `k4FWCore::Transformer` that
  turns `MCParticles` into a `GenTauCandidateCollection`.
- `options/runGenTauClassifier.py`: steering file.
- `test/`: unit tests for the helpers and an integration test running `GenTauClassifier` with `k4run`.

## Status

`GenTauClassifier` currently fills the tau, its charge, its visible constituents (pi0 kept
unexpanded) and its neutrinos. Validated against the Python implementation on 10 events.
Kinematics, decay-mode ID and tau provenance are still to be ported.

## Building

Set up the environment (pinned key4hep release):

```bash
source setupKey4Hep.sh
```

Build and install the datamodel first:

```bash
cd TauTool
cmake -B build -G Ninja -DCMAKE_INSTALL_PREFIX=install
ninja -C build install
```

Then the algorithms, pointing CMake to the installed datamodel:

```bash
cd ../k4TauAnalysis
cmake -B build -G Ninja -DCMAKE_INSTALL_PREFIX=install \
      -DCMAKE_PREFIX_PATH=$PWD/../TauTool/install
ninja -C build install
```

## Running

```bash
cd k4TauAnalysis
k4_local_repo
export LD_LIBRARY_PATH=$PWD/../TauTool/install/lib64:$LD_LIBRARY_PATH
k4run k4TauAnalysis/options/runGenTauClassifier.py
```

`k4_local_repo` puts the local install first in the environment, so remember to run
`ninja install` after every change to the plugin. The `LD_LIBRARY_PATH` export is only needed
for `podio-dump` and PyROOT to find the `TauTool` dictionary; the plugin finds `libTauTool.so`
through its RPATH.

Inspect the output:

```bash
podio-dump -e 0 -d gentaus.root
```

## Tests

```bash
cd k4TauAnalysis/build
ctest --output-on-failure
```

The `GenTauClassifier` test reads an input file on the CIEMAT storage, so it only runs where
that path is accessible.

## Code style

C++ follows the `.clang-format` at the repository root:

```bash
git ls-files '*.cpp' '*.h' | xargs clang-format -i
```

## License

Apache 2.0, see `LICENSE`.