static void disk_decimal(ULONGLONG milli){
    num(milli/1000);append(L".");num((milli%1000)/100);num((milli%100)/10);num(milli%10);
}
static void disk_report_value(const DISK_CAPTURE*c,unsigned i,LPCWSTR label,LPCWSTR unit,int decimal){
    append(label);
    if(c->m[i].status==DISK_VALID){
        if(decimal)disk_decimal(c->m[i].value);else num(c->m[i].value);
        line(unit);
    }else line(L"unavailable");
}
static void disk_report_latency(const DISK_CAPTURE*c,unsigned i,unsigned ops,LPCWSTR label){
    append(label);
    if(disk_latency_available(c,i,ops)){num(c->m[i].value);line(L" ms (whole-ms resolution)");}
    else if(c->m[i].status==DISK_VALID&&c->m[ops].status==DISK_VALID)line(L"unavailable (no resolved completed operations)");
    else line(L"unavailable");
}
static void disk_report(const DISK_CAPTURE*c){
    heading(L"PHYSICAL DISK PERFORMANCE - WINDOWS PDH AGGREGATE");
    unsigned valid=0;
    for(unsigned i=0;i<DISK_METRIC_COUNT;i++)if(c->m[i].status==DISK_VALID)valid++;
    g_disk_perf_valid=valid;g_disk_latency_flag=disk_high_latency(c);
    append(L"Valid counters: ");num(valid);append(L"/");num(DISK_METRIC_COUNT);nl();
    if(c->error){append(L"PDH query/collection status (unsigned decimal): ");num((DWORD)c->error);nl();}
    disk_report_value(c,DISK_READ_BPS,L"Read throughput: ",L" bytes/sec",0);
    disk_report_value(c,DISK_WRITE_BPS,L"Write throughput: ",L" bytes/sec",0);
    disk_report_value(c,DISK_READ_MIOPS,L"Read operations: ",L"/sec",1);
    disk_report_value(c,DISK_WRITE_MIOPS,L"Write operations: ",L"/sec",1);
    disk_report_latency(c,DISK_READ_MS,DISK_READ_MIOPS,L"Average read response: ");
    disk_report_latency(c,DISK_WRITE_MS,DISK_WRITE_MIOPS,L"Average write response: ");
    disk_report_value(c,DISK_QUEUE_MILLI,L"Average disk queue length: ",L" outstanding requests",1);
    line(L"Aggregate of Windows PhysicalDisk instances over the short activity window; not a per-drive or system-drive-only reading.");
    line(L"Latency is a Windows counter, not a direct SSD benchmark. No SMART, temperature, media type, or individual-disk active percentage measured.");
}
