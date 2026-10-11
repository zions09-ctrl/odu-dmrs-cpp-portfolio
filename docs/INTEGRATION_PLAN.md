# Native OCUDU integration plan (NOT yet implemented)

1. **Source restoration:** obtain original OCUDU tree / exact commit and the v9 out-of-tree controller source. Confirm license and patch integrity; review the tracked scheduler and remote-server modifications.
2. **RRC/geometry validator:** replace `supported(existing_mask)` with a guard against actual OCUDU PDSCH mapping type, l0, l_d, DM-RS type, `maxLength`, additional position, scheduling grant, and UE capabilities. Use native mapping APIs rather than fixed lookup for other RRC states.
3. **Stable hook:** call controller at the precise scheduler decision point before DCI/PDSCH commit. Benchmark hook via monotonic clock without blocking file IO.
4. **Application layer:** safely update scheduler-owned grant state via approved OCUDU allocation API. Verify actual applied additional position, symbol mask, RE/RB, and DCI/PDSCH PDU.
5. **Remote control:** validate input off-slot and publish a compact atomic config snapshot. Authenticate/control access outside demonstration setup.
6. **Trace & observability:** maintain separated `ctrl_ns` and `trace_ns`. Write asynchronously or to preallocated bounded buffers, and flush off-slot (the historical experiments used tmpfs). Do not report added file IO as controller compute.
7. **Validation gates:** V1 popcount, V2 RE formula, V3 applied position/mask, V4 independent gNB TBS histogram, V5 PHY PDU correspondence, V6 commit/trace slot correspondence. Raw traces required, not provided here.
8. **Timing gates:** run no-op and controller control paths, report p50/p99/max and misses against an *actual measured* commit deadline, test bursts, RT kernel and CPU isolation if required. No hard RT claim from a C++ microbenchmark.

This file is an engineering checklist, not a claim of already executed work.
