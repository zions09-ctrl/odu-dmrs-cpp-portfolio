# C++ Design Notes

## Hot-path contract

`Controller::decide` is synchronous, `noexcept`, and contains no heap allocation or logging. It always returns a value. No mutex is held on the slot read path. Writers validate an entire immutable config and publish it in one `atomic<uint32_t>::store(release)`; readers observe a consistent snapshot with `load(acquire)`.

`GrantInput` deliberately contains *only a subset* of a production NR scheduler grant. The fixed mask table mirrors the historical four 14-symbol Type-A single-frontload mappings. It must not be reused for alternative RRC combinations, `maxLength=len2` action spaces, mapping type B, other start/length parameters, or arbitrary UE capabilities.

## Test configuration

Code 1 → pos0 → `00100000000000` → 1 symbol.  
Code 2 → pos1 → `00100000000100` → 2 symbols.  
Code 3 → pos2 → `00100001000100` → 3 symbols.  
Code 4 → pos3 → `00100100100100` → 4 symbols.

`dmrs_re_per_prb = 6 × cdm_groups_no_data × popcount(mask)` is a resource-accounting invariant for the stated Type-1 test envelope, not a general-purpose 3GPP allocation engine.

## Limitations worth discussing at interview

- Real OCUDU grants have additional geometry, resource, port and UE state. A genuine adapter must check the relevant RRC parameters, PDSCH mapping, DCI constraints and scheduler timing before mutation.
- Pure compile-time mask lookup is small and bounded but only correct for the documented fixed configuration.
- Atomic writer visibility does not guarantee application in the same wall-clock slot; remote transport and update scheduling are separate.
- The supplied demo CSV never invents TBS or BLER. Those require actual PHY/codeword/MCS evaluation.
- No lock-free queue is required in this isolated controller; transporting a command is an outer-layer responsibility.
