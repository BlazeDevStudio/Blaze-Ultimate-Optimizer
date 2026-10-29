# Blaze Ultimate Optimizer source migration manifest

**Staged:** 2026-10-29. **Status:** SOURCE_STAGED (source only), NOT hardware-verified or release-ready.

## Provenance and exact hash verification

- **Original:** `BlazingSystems/BlazingSystems-Labs/BlazeUltimateOptimizer/v1.2-preview/` at commit `5d8e4f1d95b6bfc551996634df9c6f7b86af56b4`.
- **Existing destination:** `BlazingSystems/Blaze-Ultimate-Optimizer` on migration branch `migration/optimizer-v12-source-20261029`. Original `main@d89a4352b92361eb06cc8d49c13082566668beb3` untouched.
- **Exact-source staging commit:** `6887ad751308145d10a05b4f1296406df27d61c8`.
- **Source-only integrity:** 10/10 nonrelease files match original Git blob SHAs and modes (includes C source, four Python scripts, README, safety gate, historic checksum manifest, original PNG and ICO assets). Generated binaries were *deliberately excluded*.
- **Original compiled preview artifacts retained in Labs, not duplicated:** `BlazeUltimateOptimizer-x64.exe` blob `42ea7e9066f59a04b1776aa00b0f6c4df427ea4f` (195072 bytes); `BlazeUltimateOptimizer-arm64.exe` blob `ecc082243e9474ce6cfc7e8167d91e4270a745af` (187904 bytes); `BlazeUltimateOptimizer-v1.2-preview-complete.zip` blob `9a58d655458edd03b997efd1437fdeab2eee7e6b` (466782 bytes).
- The historic `SHA256SUMS.txt` is retained for attribution. It refers to original Labs release assets; CI creates a separate `CI-SHA256SUMS.txt` for newly built outputs.
- This content copy does not transplant historical commit ancestry; refer to source Lab commit for full history and attribution.

## Independent CI and release safety

The destination `.github/workflows/optimizer-ci.yml` runs Python syntax, native Windows x64 and ARM64 builds, pre-existing static PE and source safety verification and generated checksum evidence; it publishes **no executables, GitHub release or signing keys**. This validation must complete successfully at its exact PR head before marking source build VERIFIED.

The original `SAFETY-CHECKS-v1.1.md` requires physical Windows x64/ARM64 runtime smoke, privileged service backup/restore/rollback (including after reboot), actual low-memory devices, and Authenticode signing. The application modifies service settings only under user opt-in Advanced Risk Mode; those paths must remain release-blocked until tested. No claims about RAM relief or automatic driver repair are inferred from build success.

## Outstanding review

1. Validate destination CI and Windows native static gates.
2. Compare platform-specific output metadata and original release artifact ancestry.
3. Audit privileged-service rollback on real hardware; no production administration performed by migration.
4. Keep Labs sources and releases untouched until owner-approved cutover.

**No merge, release, deletion, or production rollout is authorized here.**
