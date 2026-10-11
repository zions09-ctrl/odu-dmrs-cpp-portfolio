# Reproducibility / QA report

Scope: **newly authored standalone C++20 reference controller**, not recovered OCUDU source.

## Executed on this sandbox

- Compiler: GCC 14.2.0, CMake and Linux host.
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` — PASS.
- `cmake --build build --parallel` — PASS.
- `ctest --test-dir build --output-on-failure` — PASS (2/2 CTest targets).
- `./build/dmrs_tests` — PASS (70 assertions, 0 failures).
- `./build/dmrs_demo 40 | python3 scripts/check_demo.py` — PASS (40 deterministic synthetic rows, 32 proposals causing a changed mask).
- `python3 scripts/verify_historical_aggregates.py` — PASS (9-row archive summary, six remote events, median 1,420.2 μs).
- `cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON` — PASS.
- `ctest --test-dir build-sanitize --output-on-failure` — PASS (AddressSanitizer+UBSan build, 2/2 CTest targets).
- `./build/dmrs_benchmark 100000` — measured; results in `artifacts/benchmark.json`.

## Boundary clarification

- **Current microbenchmark** numbers are for `Controller::decide()` plus wall-clock measurement overhead, on an uncontrolled shared host. Results are not stable across runs, not worst-case execution times, and not evidence of meeting an O-DU hard deadline.
- **Historical aggregate check** recomputes metadata arithmetic only. It cannot independently replay or validate raw traces.
- **No source reconstruction equivalence test against OCUDU** was possible because the earlier original source is not present in the supplied ZIP.

## Follow-ups for a production-quality portfolio

- Recover and audit actual upstream integration patches/commit diffs and authorship.
- Add native stack integration tests that confirm DCI/PDSCH PDU matches the requested mask.
- Add measured source→feature→inference→scheduler commit timing before claiming a live AI-RAN inference loop.
