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

Crash cluster reports now omit full executable paths and use validated app/module basenames plus normalized exception codes. Malformed or unsupported layouts and records outside the 12-group limit are counted separately. These are not anonymized reports: app names, startup entry names, process IDs and timestamps may still identify local activity. Review before sharing.

System Health continuation: CPU counter deltas reject resets, invalid idle totals and overflow; missing intervals do not bridge gaps. Five-second CPU/RAM summaries show sample coverage and sustained short-window pressure only when all five readings qualify. Process I/O and page-fault totals show paired-process coverage. Missing RAM/disk snapshots are not zero or stale; before/after receipts require valid readings. CPU is a time-based processor-group measurement, not Task Manager frequency-aware utility; per-process shares are omitted for multi-group/unknown topology. Physical disk performance is now separately sampled; long-term history remains planned; initial GPU and network counter summaries are now present.

Physical Disk continuation: seven local PDH PhysicalDisk(_Total) counters sampled across the existing five-second activity window (throughput, operations/sec, average read/write response and queue length). English counter paths are requested through PdhAddEnglishCounterW; counter availability varies by Windows provider and permissions. No elevation is requested, no counter configuration or disks are modified. A 50 ms aggregate response screening flag is not a failure diagnosis. Missing counters and zero-operation latency are explicitly unavailable. Individual disks, SSD/HDD identification, SMART and temperature are not implemented. Microsoft API references: https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhgetformattedcountervalue and https://learn.microsoft.com/en-us/windows/win32/perfctrs/checking-pdh-interface-return-values.

GPU/network continuation: local PDH wildcard arrays collect receive/transmit rates, GPU process/engine peak and adapter dedicated/shared memory usage. English paths are localized via counter info before adding the localized wildcard; buffers are bounded to 16 KiB for metadata and 256 KiB for arrays, with unavailable results on failure. Excludes `_Total` rows, checks instance status and arithmetic overflow, and omits instance names. Network sums may double-count virtual interfaces; engine peak is not overall GPU utilization, and memory sums are not installed capacity. Per-adapter identity, link state and true GPU-engine aggregation remain future work. API references: https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhaddenglishcounterw and https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhgetformattedcounterarrayw.
