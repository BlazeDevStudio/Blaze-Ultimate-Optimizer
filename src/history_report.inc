static HISTORY_RING g_history;
static unsigned g_history_phase=HIST_DIAGNOSE;
static void history_capture(unsigned cpu,unsigned cpuCount,unsigned ram,unsigned ramCount,const DISK_CAPTURE*d,const ADAPTER_CAPTURE*a){
    HISTORY_POINT p={0};
    p.tick=GetTickCount64();p.phase=g_history_phase;
    if(cpuCount==5&&GetActiveProcessorGroupCount()==1){p.valid|=HIST_CPU;p.cpu=cpu;}
    if(ramCount==5){p.valid|=HIST_RAM;p.ram=ram;}
    if(d->m[DISK_READ_BPS].status==DISK_VALID){p.valid|=HIST_DISK_READ;p.readBps=d->m[DISK_READ_BPS].value;}
    if(d->m[DISK_WRITE_BPS].status==DISK_VALID){p.valid|=HIST_DISK_WRITE;p.writeBps=d->m[DISK_WRITE_BPS].value;}
    if(a->m[NET_RX].available&&!a->m[NET_RX].overflow&&!a->m[NET_RX].invalid){p.valid|=HIST_NET_RX;p.rxBps=a->m[NET_RX].total;}
    if(a->m[NET_TX].available&&!a->m[NET_TX].overflow&&!a->m[NET_TX].invalid){p.valid|=HIST_NET_TX;p.txBps=a->m[NET_TX].total;}
    if(a->m[GPU_ENGINE].available&&!a->m[GPU_ENGINE].invalid){p.valid|=HIST_GPU;p.gpu=(unsigned)a->m[GPU_ENGINE].peak;}
    history_push(&g_history,p);
}
static void history_report_value(const HISTORY_POINT*p,unsigned field,ULONGLONG v){
    if(p->valid&field)num(v);else append(L"-");
}
static void history_report_stats(unsigned field,unsigned threshold,LPCWSTR label){
    HISTORY_STATS s=history_stats(&g_history,field,threshold);append(label);
    if(s.count){num(s.average);append(L"% mean, ");num(s.peak);append(L"% peak; ");num(s.hot);append(L" high observations");}
    else append(L"unavailable");
    append(L" | valid windows ");num(s.count);append(L"/");num(g_history.count);nl();
}
static void history_report(void){
    heading(L"SESSION PERFORMANCE HISTORY - ON-DEMAND SCANS");
    append(L"Retained scan windows: ");num(g_history.count);append(L"/");num(HISTORY_CAP);nl();
    line(L"Not continuous monitoring. Memory-only ring; oldest overwritten, cleared when program exits. Latest text report includes up to eight rows.");
    history_report_stats(HIST_CPU,85,L"CPU five-interval averages: ");
    history_report_stats(HIST_RAM,83,L"RAM five-snapshot averages: ");
    history_report_stats(HIST_GPU,90,L"GPU reported engine-instance peaks: ");
    line(L"Age(s) | phase | CPU% | RAM% | disk read/write B/s | net RX/TX B/s | GPU peak%");
    unsigned first=g_history.count>8?g_history.count-8:0;
    const HISTORY_POINT*last=history_at(&g_history,g_history.count?g_history.count-1:0);
    for(unsigned i=first;i<g_history.count;i++){
        const HISTORY_POINT*p=history_at(&g_history,i);
        num((last->tick-p->tick)/1000);append(L" | ");
        append(p->phase==HIST_DIAGNOSE?L"scan":p->phase==HIST_POST_FIX?L"post-fix":L"pre-action");append(L" | ");
        history_report_value(p,HIST_CPU,p->cpu);append(L" | ");history_report_value(p,HIST_RAM,p->ram);append(L" | ");
        history_report_value(p,HIST_DISK_READ,p->readBps);append(L"/");history_report_value(p,HIST_DISK_WRITE,p->writeBps);append(L" | ");
        history_report_value(p,HIST_NET_RX,p->rxBps);append(L"/");history_report_value(p,HIST_NET_TX,p->txBps);append(L" | ");
        history_report_value(p,HIST_GPU,p->gpu);nl();
    }
    line(L"- = unavailable. CPU history requires single processor group and five valid intervals; RAM requires five valid snapshots. Network sums may overlap virtual interfaces.");
    line(L"Means are equal-weight scan-window summaries, not time-weighted continuous utilization; no inference about gaps.");
}
