static void opt_report_item(const OPT_ITEM*a,const OPT_INPUT*i){
    append(a->priority==OPT_URGENT?L"URGENT":a->priority==OPT_HIGH?L"HIGH":a->priority==OPT_REVIEW?L"REVIEW":L"INFO");
    append(L" | ");append(a->evidence==OPT_REPEATED?L"repeated windows":a->evidence==OPT_PARTIAL?L"partial coverage":L"observed");
    append(L" | ");line(a->action==OPT_PROTECT?L"protect data":a->action==OPT_MANUAL?L"manual review":a->action==OPT_CONFIRM_CLEANUP?L"cleanup only with confirmation":L"remeasure");
    switch(a->id){
    case OPT_BACKUP: append(L"Storage-related event records: ");num(i->storage);nl();
        line(L"Back up important files first. Inspect the event source and disk health before cleanup or repairs; avoid unnecessary writes.");break;
    case OPT_SPACE: line(i->extremeSpace?L"System drive: less than 1 GB available to caller.":L"System drive: low available capacity.");
        if(a->action==OPT_PROTECT)line(L"Storage warnings also exist; protect data before cleanup.");
        else line(L"Review Storage Settings and personal downloads. Optional user-temp cleanup deletes files permanently and requires confirmation.");
        line(L"Keep pagefile enabled; capacity may reflect user quotas.");break;
    case OPT_HARDWARE: line(L"WHEA-like event records: inspect provider, power, temperatures and driver stability. No hardware failure established.");break;
    case OPT_BUGCHECK: line(L"Bugcheck-related event records: inspect event details and crash dumps for stop code and cause.");break;
    case OPT_RESTART: line(L"Unexpected restart records: examine nearby events, power loss and forced shutdown. Not proof of a bugcheck.");break;
    case OPT_DISK_RESPONSE: line(L"Aggregate disk response reached 50 ms in this capture. Repeat under typical load, then inspect individual disks.");
        if(i->storage)line(L"Storage events also present; device and timing correlation not established.");break;
    case OPT_MEMORY:
        line(a->evidence==OPT_REPEATED?L"RAM pressure repeated across recent diagnostic windows; gaps not monitored.":L"High RAM load in current snapshot.");
        line(L"Save work and close unused app sessions normally. Keep the pagefile system-managed.");
        if(g_group_count){append(L"Largest sampled private-commit group: ");append(g_group[0].name);line(L" (not predicted RAM savings).");}break;
    case OPT_CPU:
        line(a->evidence==OPT_REPEATED?L"CPU pressure repeated across recent diagnostic windows; gaps not monitored.":L"High CPU in current short sample.");
        if(g_cpuTopPct>=15){append(L"Top sampled process: ");append(g_cpuTopName);append(L" (");num(g_cpuTopPct);line(L"% sampled CPU capacity).");}
        line(L"Review workload and remeasure at idle; do not stop core or security processes.");break;
    case OPT_APP: append(L"Crash/hang records: ");num(i->crashes);append(L"/");num(i->hangs);nl();
        line(L"See targeted fault triage. Generic SAFE FIX does not repair driver or application crashes.");break;
    case OPT_STARTUP: line(L"Multiple visible legacy Run entries. Review Startup Apps manually; count does not establish boot impact.");break;
    case OPT_ERRORS: line(L"Multiple logged errors. Investigate provider and messages; do not run broad repairs solely on counts.");break;
    case OPT_COVERAGE: line(L"Missing or partial evidence; no clean bill of health can be inferred.");
        if(!i->logsComplete)line(L"  Event log coverage incomplete.");
        if(!i->metricsComplete)line(L"  CPU/RAM/capacity sampling incomplete.");
        if(!i->diskComplete)line(L"  Disk counter coverage incomplete.");
        if(!i->adaptersComplete)line(L"  Network/GPU counter coverage incomplete.");break;
    case OPT_OBSERVE: line(L"No major flags in the available short samples. Intermittent problems may require longer observation.");break;
    }
    nl();
}
static void make_recommendations(void){
    OPT_INPUT i={0};
    i.storage=g_storage_faults;i.whea=g_whea;i.restart=g_unexpected_power;i.bugcheck=g_bsod;
    i.crashes=g_crashes;i.hangs=g_hangs;i.sysErrors=g_sys_err;i.appErrors=g_app_err;i.startup=g_run_count;
    i.diskKnown=g_disk_ok;i.extremeSpace=g_extremedisk;i.lowSpace=g_lowdisk;i.slowDisk=g_disk_latency_flag;
    i.ramKnown=g_mem_ok;i.highRam=g_highmem;i.cpuKnown=g_cpu_ok;i.highCpu=g_cpu>=85;
    i.repeatRam=history_repeat_high(&g_history,HIST_RAM,83);i.repeatCpu=history_repeat_high(&g_history,HIST_CPU,85);
    i.logsComplete=g_events_ok;i.metricsComplete=g_mem_ok&&g_disk_ok&&g_cpu_ok&&g_cpu_intervals==5;
    i.diskComplete=g_disk_perf_valid==DISK_METRIC_COUNT;i.adaptersComplete=g_adapter_perf_valid==ADAPTER_METRIC_COUNT;
    OPT_PLAN p=opt_evaluate(&i);
    g_last_priority=p.items[0].priority;g_last_findings=p.count;
    heading(L"FINAL VERDICT - EVIDENCE-BASED PRIORITIES");
    line(L"Local rules rank observed signals, not diagnoses or a numerical health score. No action is executed by this engine.");
    for(unsigned j=0;j<p.count;j++)opt_report_item(&p.items[j],&i);
    heading(L"OPTIONAL ACTIONS AND LIMITS");
    line(L"SAFE FIX remains a separately confirmed option: reversible visual preferences and irreversible old personal temp-file deletion. Neither guarantees a speed increase.");
    line(L"Advanced Risk Mode remains separate and is never recommended automatically. Service changes require explicit confirmation and administrator rights.");
    line(L"For unexplained activity consider Windows Security Quick scan. For suspected corruption, verify first with sfc /verifyonly; repairs require separate review.");
    line(L"No random service disabling, forced RAM flushing, registry cleaning or security changes recommended.");
    line(L"Event records are not unique incidents or proof of causes. Process I/O is not physical disk throughput. Short captures cannot exclude intermittent issues.");
}
