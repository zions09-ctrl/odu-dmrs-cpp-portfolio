<div align="center">

# Real-Time O-DU DM-RS Control

**A C++20 portfolio project in 5G NR scheduling, deterministic control, and runtime validation.**

`5G NR`　·　`O-DU`　·　`C++20`　·　`Real-Time Systems`

</div>

---

## Executive summary

This project presents a compact, testable control path for selecting a PDSCH DM-RS configuration from CQI and validating the resulting scheduler proposal. The implementation emphasizes predictable slot-path behavior, explicit input limits, safe no-op semantics, and auditable test coverage.

> **Scope note:** This repository is a newly authored, standalone reconstruction inspired by a prior software-RU campaign. It is **not** the original OCUDU gNB source, an OCUDU patch, or a production deployment. The adapter produces a proposal; it does not modify a live scheduler.

## Engineering focus

| Area | Design choice |
|---|---|
| Decision path | Fixed DM-RS mask lookup for a clearly bounded 14-symbol Type-A case |
| Runtime state | Packed atomic configuration snapshot for concurrent writer/slot-reader access |
| Safe behavior | Validate supported inputs; fail closed for invalid or unsupported configurations |
| Idempotency | Distinguish a correct no-op from a failed actuation |
| Verification | Unit, boundary, invariant, adapter, and concurrency tests |
| Evidence discipline | Keep reconstructed-code results separate from historical campaign aggregates |

## What is implemented

- Four explicit fixed-test masks (`pos0`–`pos3`) for a 14-symbol, Type-A, single-front-loaded mapping case.
- Passthrough, forced-action, and CQI-threshold policies (`CQI <= 6 → code 4`, `CQI >= 13 → code 2`, otherwise `code 3`).
- Allocation-free configuration snapshots between remote writers and the scheduler reader.
- Safe no-op behavior when the requested mask is already present. `applied_additional_pos = 255` means **no mutation occurred**, not failure.
- Rejection of invalid or unsupported input rather than silent extrapolation beyond the tested configuration.
- CMake build, tests, synthetic-CQI demo, and an isolated controller microbenchmark.

## Evidence, with boundaries

The repository includes **historical aggregate evidence from an earlier software-RU campaign**: 9 retained runs, 352,054 grant-trace rows, per-run local `decide-entry → apply-exit` p50 range of 0.273–0.524 μs, p99 up to 1.349 μs, and an observed maximum of 3.096 ms. Six remote command events had a median of 1,420.2 μs (2.8404 nominal 500-μs slots).

These figures are **not measurements of this reconstruction** and do not establish a hard real-time guarantee. The included demo uses synthetic CQI. The microbenchmark measures `Controller::decide()` on the current host, includes measurement overhead, and is not a live slot/PHY deadline demonstration. Historical CSVs are aggregates, not the original raw experiment trace.

### Explicitly out of scope

Original OCUDU hook files or patches, WebSocket transport, UL channel estimator, PHY decoder, DCI encoder, live neural policy, the raw 352,054-row trace, OTA verification, and production deployment are not included.

## Build and run

**Requirements:** C++20 compiler, CMake 3.16 or newer, and a POSIX-compatible host (or another platform with threads).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/dmrs_demo 40 > artifacts/demo_trace.csv
./build/dmrs_benchmark 100000 > artifacts/benchmark.json
python3 scripts/verify_historical_aggregates.py
```

## Architecture

```text
control-plane writer
        │ validated settings
        ▼
 atomic<uint32_t> snapshot
        │
slot grant → validation → policy decision → fixed-mask lookup
                                      │
                              no-op / proposal / status
                                      │
                     live OCUDU integration not included
```

A production adapter would need to read the actual OCUDU allocation and UE/RRC context, confirm that the slot permits a layout change, apply through the native scheduler API, and verify consistency with downstream DCI/PDSCH structures. This repository makes no broader 3GPP conformance claim.

## Verification coverage

`dmrs_tests` covers mask/code lookups, popcount/resource-element invariants, forced and CQI-threshold boundaries, no-op semantics, invalid configuration handling, unsupported layouts, adapter mapping, and concurrent reads/writes of the packed atomic configuration. This is standalone library testing; it does not validate OCUDU or radio behavior.

## Selected design decisions

1. **Why an atomic snapshot?** The slot thread should not observe a partially updated control policy. A compact immutable snapshot makes the runtime boundary explicit.
2. **Why distinguish no-op from failure?** If the allocation already matches the target, avoiding a redundant mutation is correct and should be observable as such.
3. **Why fail closed on unsupported layouts?** The precomputed masks are valid only within a bounded mapping/RRC envelope.
4. **Why separate microbenchmark and campaign timings?** Controller timing, instrumentation timing, remote command latency, and downstream commit timing are different measurement boundaries.
5. **What remains before production?** Source/patch audit, gNB build and replay, downstream PDU cross-check, broader trace coverage, real-time host controls, and repeated stress testing.

## Repository guide

- [Korean portfolio overview and interview notes](docs/PORTFOLIO_KO.md)
- [Architecture and trade-offs](docs/ARCHITECTURE.md)
- [Integration acceptance plan](docs/INTEGRATION_PLAN.md)
- [Provenance, evidence boundaries, and limitations](docs/PROVENANCE_AND_LIMITS.md)
- [QA report](docs/QA_REPORT.md)
- [Portfolio overview PDF](docs/Portfolio_Overview.pdf)

## License

MIT applies to this newly authored standalone example. Upstream OCUDU source and patches are not distributed under this license. Any future upstream integration must separately respect its license and ownership conditions.
