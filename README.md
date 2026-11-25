# Blaze Ultimate Optimizer
A native, lightweight offline Windows diagnostics utility by **Blaze Systems**, with executable publisher metadata **Loumer Cabaluna Manreal**.

## v1.3 preview (Windows x64 / experimental Windows ARM64)
Built for low-spec PCs, converted Intel tablets and compatible Windows-based Chromebooks. Windows ARM64 is **not** a Linux Orange Pi build.

v1.3 adds a five-second CPU activity window, a sampled available-RAM range, process page fault deltas, process I/O transfer measurements, and .NET Runtime Event 1026 counting. It groups Conexant audio faults into a single recommendation and avoids duplicate Epson advice.

**Measurement cautions:** page fault deltas include soft and hard faults; process I/O includes non-disk activity. Neither equals hard faults/sec, SSD I/O nor disk latency. It does not diagnose thermals, SMART, malware, or verify every file.

Use **1-CLICK DIAGNOSE** before any changes. Safe Fix remains conservative and correctly reports when it does not measurably improve RAM pressure. Advanced Risk Mode is opt-in, requires two confirmations and a three-second warning, and affects only the WSearch and SysMain startup types, with a rollback option.

Logo V2 is the owner-approved *cropped raster* asset, retrieved from the pinned historical source during builds and checksum-validated; the icon is not a newly generated or redrawn logo. The Windows PE metadata contains your name, but the binaries are unsigned, so Windows may display **Unknown publisher**. No telemetry.

**Status:** cross-compiled and static checks run; no real-Windows runtime, tablet or ARM64 hardware verification yet. Never treat an untested preview as a repair for a boot-critical failure.

### Source provenance
The historical v1.0–v1.2 previews remain intact in [BlazingSystems-Labs](https://github.com/BlazeDevStudio/BlazingSystems-Labs/tree/main/BlazeUltimateOptimizer). This dedicated repository contains v1.3 source and reproducible Windows build scripts.

### Diagnostics continuation (unreleased)
Reference: [Pulse Diagnostics](https://github.com/Regncreative/PulseDiagnostics), MIT license (feature/architecture inspiration only; no upstream source or branding imported). This native continuation keeps diagnostics read-only, reports coverage independently for each Windows event log, and separates unexpected restart from bugcheck-related records. A complete scan means the age cutoff or retained-log EOF was reached, not that seven days of history exist. Scan limits, failed reads and malformed records remain visible. No changes to optimizer actions, service allowlists, or runtime privileges. Mocked scanner tests and native builds do not establish actual Windows runtime or rollback readiness.
