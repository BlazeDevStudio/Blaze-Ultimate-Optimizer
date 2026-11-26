/* Local PDH PhysicalDisk(_Total) counters; no disk handles, writes or repair actions.
 * Explicit scale factors make all displayed values integers without a CRT.
 * Two successful collections are required for rate/average counters. */
enum { DISK_READ_BPS,DISK_WRITE_BPS,DISK_READ_MIOPS,DISK_WRITE_MIOPS,DISK_READ_MS,DISK_WRITE_MS,DISK_QUEUE_MILLI,DISK_METRIC_COUNT };
enum { DISK_NOT_ADDED,DISK_PENDING,DISK_VALID,DISK_FAILED };
typedef struct {LPCWSTR path;LONG scale;} DISK_SPEC;
static const DISK_SPEC disk_specs[DISK_METRIC_COUNT]={
    {L"\\PhysicalDisk(_Total)\\Disk Read Bytes/sec",0},
    {L"\\PhysicalDisk(_Total)\\Disk Write Bytes/sec",0},
    {L"\\PhysicalDisk(_Total)\\Disk Reads/sec",3},
    {L"\\PhysicalDisk(_Total)\\Disk Writes/sec",3},
    {L"\\PhysicalDisk(_Total)\\Avg. Disk sec/Read",3},
    {L"\\PhysicalDisk(_Total)\\Avg. Disk sec/Write",3},
    {L"\\PhysicalDisk(_Total)\\Avg. Disk Queue Length",3}
};
typedef struct {HANDLE handle;unsigned status;ULONGLONG value;} DISK_METRIC;
typedef struct {HANDLE query;DISK_METRIC m[DISK_METRIC_COUNT];int baseline;LONG error;} DISK_CAPTURE;
static void disk_capture_begin(DISK_CAPTURE*c){
    c->query=0;c->baseline=0;c->error=0;
    for(unsigned i=0;i<DISK_METRIC_COUNT;i++){c->m[i].handle=0;c->m[i].status=DISK_NOT_ADDED;c->m[i].value=0;}
    c->error=PdhOpenQueryW(0,0,&c->query);
    if(c->error||!c->query){c->query=0;return;}
    unsigned added=0;
    for(unsigned i=0;i<DISK_METRIC_COUNT;i++){
        HANDLE h=0;
        if(!PdhAddEnglishCounterW(c->query,disk_specs[i].path,0,&h)&&h){
            if(!PdhSetCounterScaleFactor(h,disk_specs[i].scale)){
                c->m[i].handle=h;c->m[i].status=DISK_PENDING;added++;
            }
        }
    }
    if(added){c->error=PdhCollectQueryData(c->query);c->baseline=!c->error;}
}
static int disk_read_metric(HANDLE h,ULONGLONG*out){
    BLAZE_PDH_VALUE v;
    /* PDH_FMT_LARGE | PDH_FMT_NOCAP100. Explicit scale already set. */
    if(PdhGetFormattedCounterValue(h,0x8400,0,&v)||(v.CStatus!=0&&v.CStatus!=1)||v.value.largeValue<0)return 0;
    *out=(ULONGLONG)v.value.largeValue;return 1;
}
static void disk_capture_end(DISK_CAPTURE*c){
    if(!c->query)return;
    int ok=0;
    if(c->baseline){c->error=PdhCollectQueryData(c->query);ok=!c->error;}
    for(unsigned i=0;i<DISK_METRIC_COUNT;i++){
        if(c->m[i].status==DISK_PENDING)
            c->m[i].status=ok&&disk_read_metric(c->m[i].handle,&c->m[i].value)?DISK_VALID:DISK_FAILED;
        c->m[i].handle=0;
    }
    PdhCloseQuery(c->query);c->query=0;
}
static int disk_latency_available(const DISK_CAPTURE*c,unsigned latency,unsigned ops){
    return c->m[latency].status==DISK_VALID&&c->m[ops].status==DISK_VALID&&c->m[ops].value>0;
}
static int disk_high_latency(const DISK_CAPTURE*c){
    return (disk_latency_available(c,DISK_READ_MS,DISK_READ_MIOPS)&&c->m[DISK_READ_MS].value>=50)
        || (disk_latency_available(c,DISK_WRITE_MS,DISK_WRITE_MIOPS)&&c->m[DISK_WRITE_MS].value>=50);
}
