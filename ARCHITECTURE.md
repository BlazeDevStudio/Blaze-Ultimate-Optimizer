# Source cleanup baseline — Blaze Ultimate Optimizer

Based on v1.3 HEAD `0e2afa801ef331d90f2b54f270e4b462ed24b4c9`. This change is a **behavior-preserving structural refactor**, not an optimizer repair or a release.

The app remains one translation unit because it currently depends on ordered static globals, custom Win32 ABI declarations, and a freestanding/no-CRT build. `src/BlazeUltimateOptimizer.c` includes source fragments in dependency order. This is intentional transitional architecture, not separately linked modules yet.

| File | Responsibility |
| --- | --- |
| `platform_win32.h` | Windows API definitions, imports, constants |
| `state.inc` | Shared process state and internal structs |
| `text.inc` | UTF-16 text report primitives and reset |
| `events.inc` | Startup, process and event-log collection |
| `sampling.inc` | CPU and process activity sampling |
| `report.inc` | Report formatting and diagnostic verdict |
| `repairs.inc` | Visual preference rollback, temp cleaning, WSearch/SysMain allowlist |
| `runtime.inc` | Local report save, background worker and risk confirmations |
| `ui.inc` | Native button layout, message handling, Windows entry point |

Build.py discovers Win32 imports from the platform header. Verify.py uses the concatenated source text for its historical static assertions. The source-layout checker compares the concatenation to the immutable original commit byte for byte, preventing accidental logic edits during cleanup.

**Safety unchanged:** no new service targets; no Defender, pagefile, boot/driver modifications; no automatic app killing. Advanced Risk Mode still requires the original opt-in and 3-second warning. No production release or actual Windows runtime proof is claimed.

**Next cleanup:** replace shared mutable globals with bounded context interfaces and individual object-file modules, after explicit compile/runtime acceptance. Main remains README-only and the independent v1.2 migration PR #1 is preserved.
