# v00.07.01

* 2026-08-03 Victor Schwan ([PR#33](https://github.com/key4hep/k4-project-template/pull/33))
  - Format Python files and README

* 2026-07-30 Juan Miguel Carceller ([PR#32](https://github.com/key4hep/k4-project-template/pull/32))
  - Set `GAUDI_PLUGIN_PATH` in tests; Gaudi will prefer that and it seems to fix some obscure bugs in CI when using LCG stacks sometimes.

# v00.07.00

* 2025-09-09 Thomas Madlener ([PR#26](https://github.com/key4hep/k4-project-template/pull/26))
  - Make sure to prefer the Key4hep built python over the system python when calling cmake

* 2025-08-27 Juan Miguel Carceller ([PR#31](https://github.com/key4hep/k4-project-template/pull/31))
  - Add some information about continuous integration in forks

* 2025-08-04 jmcarcell ([PR#30](https://github.com/key4hep/k4-project-template/pull/30))
  - Move the tests to its own folder
  - Use general code to set the environment for the tests, with explanation about why each path is needed. This should work in most repositories (at least those that have all the tests in one folder) without changes, avoiding the current status of copy & paste and confusion.
  - Other updates in the README on how to use this project as a template

* 2025-06-20 BrieucF ([PR#28](https://github.com/key4hep/k4-project-template/pull/28))
  - Update pre-commit to run on ubuntu-latest

# v00-01

* This file is also automatically populated by the tagging script