# matrixpp
A C++20 and later matrix library

![Build Status](https://img.shields.io/azure-devops/build/samestimable2016/matrixpp/2?label=%F0%9F%94%A8%20Build%20Status) ![Test Results](https://img.shields.io/azure-devops/tests/samestimable2016/matrixpp/2?label=%F0%9F%A7%AA%20Test%20Results) ![Code Coverage](https://img.shields.io/azure-devops/coverage/samestimable2016/matrixpp/2?label=%F0%9F%93%B6%20Code%20Coverage) ![GitHub Pages](https://img.shields.io/github/deployments/sam20908/matrixpp/github-pages?label=%F0%9F%9A%80%20GitHub%20Pages)

## Including the Project:
- A target `matrixpp` will be created (or `matrixpp::matrixpp` if you prefer this form). This means you can simply `add_subdirectory` the project and link the target.
- You can also just include the headers into your project manually if that's your decision.

## Test Suite Dependencies:
- Python package `lit` for compile tests (has to be found in **PATH**).
- `gtest` in `vcpkg` for unit tests.
- `benchmark` in `vcpkg` for benchmarks.
- `gcov` and `gcovr` for code coverage (has to be found in **PATH**).

## Using Built-in Tests:
- Specify `MATRIXPP_BUILD_TESTS` to a boolean value to enable or disable building the test.
    - This enables all of the following implicitly:
        - `MATRIXPP_BUILD_COMPILE_TESTS` to `TRUE`.
        - `MATRIXPP_BUILD_UNIT_TESTS` to `TRUE`.
            - **Note: Multi configuration generators will put same executables of different configurations in unit tests binary directory!**
    - You can manually specify one of those above options to `FALSE` should you disable that specific test.
- Specify `MATRIXPP_BUILD_BENCHMARKS` to a boolean value to enable or disable building the benchmarks.
    - Recommended to build in **Release** configuration.

## Using the Lit Test Suite:
- Running `ctest` in `<build directory>` directory invokes all tests (use `ctest -V` for verbose output).
- Running `lit` either in `<build directory>/bin/tests/unit_tests` or `<build directory>/bin/tests/compile_tests` runs all the test in that directory (CMake will detect if `lit` is found in PATH before setting up the test suite).
- You can manually run specific unit tests by invoking the executable (don't double click on them, instead run them from command line).
- You can manually run specific compile tests by using Lit's parameter `--filter` and pass a regex string to match the tests.

## Using the Performance Test Suite:
- Each benchmarks will be comppiled to an executable, and all the benchmarks will be located in `<build directory>/bin/benchmarks`.
- The benchmarks are written using `Google Benchmark`, which means you may use `Google Benchmark` command line options when invoking the executable.
- There will be an executable called `all_benchmarks`, which invokes all the benchmarks.

## Using Code Coverage:
- ***Requires unit tests to be enabled!***
- NOTE: **Ninja** won't work for code coverage due to a bug, so a workaround is to use **Ninja Multi-Config**.
- Specify `MATRIXPP_CODE_COVERAGE` to a boolean value to enable or disable code coverage.
- **NOTE**: All the requirements are found in https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake.
    - Recommended to build in **Debug** configuration.
    - Coverage file will be in `<build directory>` named `matrixpp_code_coverage.xml`.