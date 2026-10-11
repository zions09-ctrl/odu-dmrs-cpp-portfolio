# Provenance, validation boundaries, and source recovery

## Three layers that must not be confused

- **A. Historical OCUDU experiment (reported):** October 2026 OCUDU `v26.10.0`, commit `5512e6519f9dd7382af36810904b1bf7f7be41ad`; 9 retained software-RU runs, 352,054 trace rows, reported 0 V1/V2 violations, bounded hook p99 but substantial observed tail stalls. The aggregate files in `historical/` were preserved from the WCL manuscript archive. This project does not independently authenticate the original experiment.
- **B. This C++ source (newly created):** Clean-room decision/controller reconstruction built from previously supplied descriptions and masks; compilable standalone. Tests and benchmark are newly executed in the current sandbox. **No original OCUDU source code is being passed off as recovered.**
- **C. Offline two-action risk-aware ML experiment (distinct):** Separate 32-feature, two-action policy described by the WCL manuscript. **Not implemented/inferred** by this four-density controller. Its results may not be reported as live ML-in-OCUDU performance.

## Original files listed in supplied patch manifest but not in the available ZIP

Tracked modifications include `lib/mac/mac_dl/mac_cell_processor.cpp`, `lib/phy/lower/modulation/ofdm_demodulator_impl.cpp`, `lib/phy/upper/channel_processors/pusch/pusch_demodulator_impl.cpp`, `lib/phy/upper/equalization/channel_equalizer_generic_impl.cpp`, `lib/phy/upper/signal_processors/channel_estimator/port_channel_estimator_average_impl.cpp`, `lib/phy/upper/signal_processors/pusch/dmrs_pusch_estimator_impl.cpp`, `lib/phy/upper/upper_phy_factories.cpp`, and `lib/radio/realtime_loopback/radio_session_realtime_loopback_impl.{cpp,h}`. Out-of-tree additions were listed under `include/ocudu/out_of_tree_{dl_commit,tl1}_probe.h`, `.../dapp_ch_est_capture.*`, `.../v9_frontend*.hpp`, and `.../lb_channel_model.h`. Another live controller source was reported at `include/ocudu/out_of_tree_dmrs_dapp.h`. Full C++ source and original patch scripts were not inside the consulted documentation ZIP.

**Historical workspace path (not available in this sandbox):** `/root/oran_workspace/ocudu`.

## Revalidation when real source is recovered

1. Preserve the original tree or commit and produce a `git diff --binary` before modifying anything.
2. Confirm source SHA-256 against the historical patch manifest; do not assume every listed hash represents a single consistent run across archived revisions.
3. Review all owner-modified files (no blind copy from incompatible version), third-party licensing, and secrets/identifiers before public release.
4. Rebuild OCUDU and run unit/integration tests in the original environment.
5. Reproduce Campaign 4, retain raw trace/commit/PDU logs and run V1–V6 cross-layer checks.
6. Separately benchmark the frozen neural decision integrated in the scheduler if making a full real-time AI claim.

## Precise measurement boundaries

- Historical `ctrl_ns`: `decide()` entry → `apply()` exit, **not** trace flush, UL channel estimation, 32-D feature construction or neural inference.
- Historical remote timing: command arrival → first applied slot; median from six reported events is 1,420.2 μs / 500 μs = 2.8404 slots.
- Our microbenchmark: standalone host microseconds/nanoseconds per call, affected by timer, scheduling and compiler optimizations; neither a hard deadline nor a real stack result.
- Literature / CPRS: not evaluated by this repository.

Source package: `oran_v9_dapp_complete_docs_2026-10-11(1).zip`, and `WCL_Manuscript_Draft_Package.zip` previously available in the user's Library.
