#include "inventory.inc"
static void stats(void){MEMORYSTATUSEX mem;mem.dwLength=sizeof(mem);if(GlobalMemoryStatusEx(&mem)&&health_memory_valid(mem.ullTotalPhys,mem.ullAvailPhys,mem.dwMemoryLoad)){g_mem_ok=1;g_ram=mem.ullTotalPhys;g_memload=mem.dwMemoryLoad;g_highmem=mem.dwMemoryLoad>=83;append(L"Physical RAM: ");fmt_size(mem.ullTotalPhys);append(L" | Available: ");fmt_size(mem.ullAvailPhys);append(L" | Memory load: ");num(mem.dwMemoryLoad);line(L"%");append(L"Virtual/commit capacity: ");fmt_size(mem.ullTotalPageFile);append(L" | Available commit: ");fmt_size(mem.ullAvailPageFile);nl();if(g_ram<=2147483648ULL)line(L"DEVICE PROFILE: EXTREME 1-2 GB RAM. Prioritize one active application; pagefile must remain enabled.");
else if(g_ram<=4294967296ULL)line(L"DEVICE PROFILE: LIGHT 4 GB RAM. High-impact work is grouping/closing unused app sessions.");
if(mem.ullTotalPageFile<mem.ullTotalPhys+268435456ULL){line(L"NOTICE: Commit capacity is unusually small. Verify the pagefile is SYSTEM-MANAGED.");}}else line(L"Physical RAM/commit snapshot unavailable.");
WCHAR drive[32];DWORD dn=GetEnvironmentVariableW(L"SystemDrive",drive,32);
ULONGLONG avail,total,freeb;
if(dn==2&&((drive[0]>=L'A'&&drive[0]<=L'Z')||(drive[0]>=L'a'&&drive[0]<=L'z'))&&drive[1]==L':'){
wcat(drive,L"\\",32);
if(GetDiskFreeSpaceExW(drive,&avail,&total,&freeb)&&total&&avail<=total){
g_disk_ok=1;g_disk=total;g_free=avail;g_extremedisk=(avail<1073741824ULL);g_lowdisk=(avail<3ULL*1073741824ULL||health_percent(avail,total)<12);
append(L"System drive ");append(drive);append(L": ");fmt_size(total);append(L" total available to caller, ");fmt_size(avail);line(L" free to caller (quotas may apply).");
}}
if(!g_disk_ok)line(L"System drive capacity check unavailable.");
else if(g_disk<=20ULL*1073741824ULL)line(L"STORAGE PROFILE: TINY 16-20 GB accessible capacity. Avoid extra applications and large repair caches.");
else if(g_disk<=40ULL*1073741824ULL)line(L"STORAGE PROFILE: COMPACT 32-40 GB accessible capacity. Monitor free space during Windows updates.");
g_uptime=GetTickCount64()/3600000ULL;append(L"Uptime: ");num(g_uptime);line(L" hours");metric_cpu();if(g_cpu_ok){append(L"Sample CPU utilization (1500ms): ");num(g_cpu);line(L"% (processor-group scope; a momentary sample, not a long-term average)");if(g_cpuTopPct){append(L"Top sampled process CPU share: ");append(g_cpuTopName);append(L" - ");num(g_cpuTopPct);line(L"% of sampled CPU capacity");}}else line(L"Sample CPU utilization unavailable.");}
static void top_procs(void){process_snapshot();
line(L"Grouped process private committed bytes (NOT physical RAM saved by closure):");
for(int i=0;i<g_group_count;i++)for(int j=i+1;j<g_group_count;j++)if(g_group[j].priv>g_group[i].priv){GROUP tmp=g_group[i];g_group[i]=g_group[j];g_group[j]=tmp;}
for(int i=0;i<g_group_count&&i<8;i++){append(L"  ");append(g_group[i].name);append(L" (processes ");num(g_group[i].processes);append(L"): private commit ");fmt_size(g_group[i].priv);append(L", working-set sum ");fmt_size(g_group[i].working);nl();}
line(L"Working sets below can double-count shared pages across processes:");line(L"Largest visible process working sets (shared pages included):");for(int i=0;i<8;i++){if(!g_top[i].usage)break;if(i==0)wcopy(g_top1,g_top[i].name,MAX_PATH);append(L"  ");append(g_top[i].name);append(L" [PID ");num(g_top[i].pid);append(L"]: ");fmt_size(g_top[i].usage);nl();}}
#include "optimization_rules.inc"
#include "optimization_report.inc"
static void report_title(void){SYSTEMTIME t;GetLocalTime(&t);line(L"BLAZE ULTIMATE OPTIMIZER - Windows 10/11 ultra-low-spec diagnostic v1.3");line(L"Native Windows x64/ARM64 / user-mode / offline / zero telemetry");append(L"Scan date: ");num(t.wYear);append(L"-");num(t.wMonth);append(L"-");num(t.wDay);append(L" ");num(t.wHour);append(L":");if(t.wMinute<10)append(L"0");num(t.wMinute);nl();}
static void report_log_coverage(LPCWSTR name,LOG_SCAN scan){
    append(name);append(L": ");num(scan.count);append(L" records; ");
    switch(scan.status){
    case LOG_COMPLETE: line(L"reached age boundary or end of accessible retained log");break;
    case LOG_OPEN_FAILED: append(L"open failed, Win32 error ");num(scan.error);nl();break;
    case LOG_READ_FAILED: append(L"read incomplete, Win32 error ");num(scan.error);nl();break;
    case LOG_LIMIT: line(L"partial - scan budget reached");break;
    default: line(L"partial - malformed or empty read buffer");break;
    }
}
static void collect(void){reset();report_title();heading(L"LIVE SYSTEM METRICS");stats();activity_sample();heading(L"TOP MEMORY USERS");top_procs();heading(L"STARTUP ENTRIES (REGISTRY RUN KEYS)");reg_count(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",L"User");reg_count(HKEY_LOCAL_MACHINE,L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",L"Machine");reg_count(HKEY_LOCAL_MACHINE,L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",L"Machine x86");append(L"Visible registry Run entries: ");num(g_run_count);line(L". Other startup sources (tasks/services/startup folder) are not counted.");heading(L"WINDOWS EVENT LOGS (PAST 7 DAYS)");LOG_SCAN sys=scan_events(L"System"),app=scan_events(L"Application");g_events_ok=(sys.status==LOG_COMPLETE&&app.status==LOG_COMPLETE);report_log_coverage(L"System",sys);report_log_coverage(L"Application",app);append(L"Errors: System ");num(g_sys_err);append(L", Application ");num(g_app_err);nl();append(L"Storage-related IDs: ");num(g_storage_faults);append(L" | WHEA-like IDs: ");num(g_whea);append(L" | unexpected restart (41): ");num(g_unexpected_power);append(L" | bugcheck-related (1001): ");num(g_bsod);nl();append(L"App crash (1000): ");num(g_crashes);append(L" | app hang (1002): ");num(g_hangs);nl();append(L"Provider-matched .NET Runtime Event 1026 records: ");num(g_clr_events);nl();line(L"Counts are clues, not confirmed diagnoses. Read source/provider and message in Event Viewer. Scan stops at age boundary, end of retained records, or 3500 records/60 reads per log. Cleared logs or clock changes may limit history.");
heading(L"APPLICATION CRASH CLUSTERS - EVENT 1000 DETAILS");
append(L"Grouped records: ");num(g_crash_grouped);append(L" | unsupported/malformed fields: ");num(g_crash_rejected);append(L" | outside 12-group capacity: ");num(g_crash_ungrouped);nl();
line(L"Full executable paths are not retained or reported. App/module names and other local identifiers may still be sensitive; review before sharing.");
if(!g_app_names_count)line(L"No parseable Application Error 1000 insertion strings in sampled records.");
for(int i=0;i<g_app_names_count;i++)for(int j=i+1;j<g_app_names_count;j++)if(g_faults[j].count>g_faults[i].count){FAULT tmp=g_faults[i];g_faults[i]=g_faults[j];g_faults[j]=tmp;}
for(int i=0;i<g_app_names_count;i++){
    append(L"  ");append(g_faults[i].name);append(L" - ");num(g_faults[i].count);append(L" crash record(s)");
    if(g_faults[i].module[0]){append(L" | module: ");append(g_faults[i].module);}
    if(g_faults[i].exception[0]){append(L" | exception: ");append(g_faults[i].exception);}
    if(g_faults[i].newest&&g_current_event_epoch>=g_faults[i].newest){append(L" | latest approximately ");num((g_current_event_epoch-g_faults[i].newest)/3600ULL);append(L"h ago");}
    nl();
}
line(L"Event 1000 inserted fields are evidence, not proof that the named module caused the fault. Records may repeat or be truncated.");
heading(L"TARGETED FAULT TRIAGE - NO AUTOMATIC DRIVER CHANGES");
int matched=0,audioReported=0,epsonReported=0;
for(int i=0;i<g_app_names_count;i++){
  if(same_ci(g_faults[i].name,L"AUDIODG.EXE")&&contains_ci(g_faults[i].module,L"CX64")){
    if(!audioReported){line(L"AUDIO FAMILY: Multiple CX64 module crashes can be related. Disable audio enhancements, check NEW Event 1000 timestamps, and only then consider the matching OEM driver. Do not terminate audiodg.exe or disable audio services.");audioReported=1;}matched=1;
  }else if(contains_ci(g_faults[i].name,L"E_YAR")||contains_ci(g_faults[i].module,L"E_YAP")){
    if(!epsonReported){line(L"EPSON-RELATED: Possible Epson printer status-monitor component. Repair the matching utility if needed; do not disable the Windows Print Spooler.");epsonReported=1;}matched=1;
  }else if(same_ci(g_faults[i].module,L"KERNELBASE.dll")){
    line(L"GENERIC WINDOWS MODULE: KERNELBASE.dll identifies where an exception surfaced, not necessarily the cause. Use the app name and exception code, and inspect the full path locally in Event Viewer if needed.");matched=1;
  }
}
if(!matched)line(L"No safe vendor-specific repair pattern matched. Inspect full Event Viewer messages before changing drivers or services.");
make_recommendations();history_report();inventory_report();}
