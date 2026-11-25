static void adapter_report_summary(const ADAPTER_SUMMARY*s,LPCWSTR label,LPCWSTR unit,int peakOnly){
    append(label);
    if(s->available&&(peakOnly||!s->overflow)){num(peakOnly?s->peak:s->total);append(unit);}
    else append(L"unavailable");
    append(L" | valid instances: ");num(s->valid);append(L", invalid: ");num(s->invalid);
    append(L", aggregate rows excluded: ");num(s->skipped);nl();
}
static void adapter_report(const ADAPTER_CAPTURE*c){
    unsigned covered=0;
    for(unsigned i=0;i<ADAPTER_METRIC_COUNT;i++)if(c->m[i].available&&!c->m[i].overflow&&!c->m[i].invalid)covered++;
    g_adapter_perf_valid=covered;
    heading(L"NETWORK ADAPTER ACTIVITY - WINDOWS PDH");
    adapter_report_summary(&c->m[NET_RX],L"Sum of reported receive rates: ",L" bytes/sec",0);
    adapter_report_summary(&c->m[NET_TX],L"Sum of reported transmit rates: ",L" bytes/sec",0);
    line(L"Physical and virtual interface counters may overlap. Not internet speed, unique traffic, or link capacity.");
    heading(L"GPU ENGINE AND ADAPTER MEMORY - WINDOWS PDH");
    adapter_report_summary(&c->m[GPU_ENGINE],L"Highest reported process/engine instance: ",L"%",1);
    adapter_report_summary(&c->m[GPU_DEDICATED],L"Sum of reported dedicated memory usage: ",L" bytes",0);
    adapter_report_summary(&c->m[GPU_SHARED],L"Sum of reported shared memory usage: ",L" bytes",0);
    line(L"Engine instance peak is not overall GPU utilization. Memory usage is not total installed VRAM or budget.");
    line(L"Provider instance names omitted; multi-adapter identities, connection details and full GPU engine aggregation are not measured.");
    append(L"Complete counter summaries (among returned instances): ");num(covered);append(L"/");num(ADAPTER_METRIC_COUNT);nl();
    if(c->error){append(L"PDH query/collection status (unsigned decimal): ");num((DWORD)c->error);nl();}
    line(L"Unavailable or partial counters do not establish zero activity; provider support and bounded buffers limit coverage.");
}
