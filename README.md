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

- `TauClassification`: shared library without Gaudi dependencies, with the helpers used by the
  algorithms:
  - `GenTauUtils.h` (namespace `tautool::gen`): generator-level logic — `isFinalTau`,
    `isNeutrino`, `getDecayProducts`, `visibleDecayMode` and the decay-mode codes in
    `tautool::gen::decaymode`.
  - `KinematicUtils.h` (namespace `tautool::kin`): `deltaRThetaPhi`, the (theta, phi) distance
    used for cone sizes and matching.
- `k4TauAnalysisPlugins`: Gaudi plugin with `GenTauClassifier`, a `k4FWCore::Transformer` that
  turns `MCParticles` into a `GenTauCandidateCollection`.
- `options/runGenTauClassifier.py`: steering file.
- `test/`: unit tests for the helpers and an integration test running `GenTauClassifier` with `k4run`.

## Status

`GenTauClassifier` fills, for every final generator-level tau: the tau itself, its charge, its
visible constituents (pi0 kept unexpanded) and neutrinos, its full and visible kinematics
(`genMomentum`, `genMass`, `visMomentum`, `visMass`, `maxConeAngle`) and its visible decay mode.
Still to be ported: true decay mode, extra neutrals, photon origins and tau provenance.

## Building and running

```bash
./build.sh          # configure (first time), build and install both projects, run the tests
source env.sh       # runtime environment, once per shell
cd k4TauAnalysis
k4run k4TauAnalysis/options/runGenTauClassifier.py
```

`build.sh` uses the pinned key4hep release from `setupKey4Hep.sh` and only configures a project
if its `build/` directory does not exist yet. `env.sh` must be sourced, not executed: it runs
`k4_local_repo` and adds the `TauTool` dictionary to `LD_LIBRARY_PATH` (needed by `podio-dump`
and PyROOT; the plugin itself finds `libTauTool.so` through its RPATH).

The input file defaults to the CIEMAT storage. Override it with:

```bash
export TAU_INPUT=/path/to/events_edm4hep.root
```

Inspect the output:

```bash
podio-dump -e 0 -d gentaus.root
```

## Tests

```bash
ctest --test-dir k4TauAnalysis/build --output-on-failure
```

`build.sh` already runs them. The `GenTauClassifier` integration test needs an accessible input
file (see `TAU_INPUT` above).

## Code style

C++ follows the `.clang-format` at the repository root:

```bash
git ls-files '*.cpp' '*.h' | xargs clang-format -i
```

## License

Apache 2.0, see `LICENSE`.