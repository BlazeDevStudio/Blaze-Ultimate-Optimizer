/* Read-only wildcard PDH counters. Bounded scratch buffers shared by serial worker.
 * Localize the English wildcard path, then add it via PdhAddCounterW for array reads.
 * Names are validated but never retained/exported. Results are provider instance summaries. */
enum { NET_RX,NET_TX,GPU_ENGINE,GPU_DEDICATED,GPU_SHARED,ADAPTER_METRIC_COUNT };
typedef struct {LPCWSTR path;LONG scale;int peakOnly;} ADAPTER_SPEC;
static const ADAPTER_SPEC adapter_specs[ADAPTER_METRIC_COUNT]={
    {L"\\Network Interface(*)\\Bytes Received/sec",0,0},
    {L"\\Network Interface(*)\\Bytes Sent/sec",0,0},
    {L"\\GPU Engine(*)\\Utilization Percentage",0,1},
    {L"\\GPU Adapter Memory(*)\\Dedicated Usage",0,0},
    {L"\\GPU Adapter Memory(*)\\Shared Usage",0,0}
};
typedef struct {ULONGLONG total,peak;unsigned valid,invalid,skipped;int available,overflow;} ADAPTER_SUMMARY;
typedef struct {HANDLE query,h[ADAPTER_METRIC_COUNT];ADAPTER_SUMMARY m[ADAPTER_METRIC_COUNT];int baseline;LONG error;} ADAPTER_CAPTURE;
static union {ULONGLONG align;BYTE bytes[16384];} adapter_info;
static union {ULONGLONG align;BYTE bytes[262144];} adapter_array;
/* Validate pointer, alignment and bounded NUL termination before accessing provider names. */
static int adapter_string(const BYTE*base,DWORD size,LPCWSTR name,DWORD minOffset){
    ULONG_PTR p=(ULONG_PTR)name,b=(ULONG_PTR)base;
    if(p<b||p-b<minOffset||p-b>=size||((p-b)&1))return 0;
    DWORD off=(DWORD)(p-b);
    for(;off<=size&&size-off>=sizeof(WCHAR);off+=sizeof(WCHAR))
        if(!*(const WCHAR*)(base+off))return 1;
    return 0;
}
static int adapter_is_total(LPCWSTR s){
    const WCHAR*t=L"_Total";
    while(*t&&*s){WCHAR a=*s++,b=*t++;if(a>=L'A'&&a<=L'Z')a+=32;if(b>=L'A'&&b<=L'Z')b+=32;if(a!=b)return 0;}
    return !*s&&!*t;
}
static HANDLE adapter_add(HANDLE q,const ADAPTER_SPEC*s){
    HANDLE english=0,localized=0;DWORD bytes=sizeof(adapter_info.bytes);
    if(PdhAddEnglishCounterW(q,s->path,0,&english)||!english)return 0;
    if(!PdhGetCounterInfoW(english,0,&bytes,adapter_info.bytes)&&bytes<=sizeof(adapter_info.bytes)&&bytes>=sizeof(BLAZE_PDH_INFO_PREFIX)){
        const BLAZE_PDH_INFO_PREFIX*p=(const BLAZE_PDH_INFO_PREFIX*)adapter_info.bytes;
        if(adapter_string(adapter_info.bytes,bytes,p->fullPath,sizeof(*p))){
            if(PdhAddCounterW(q,p->fullPath,0,&localized))localized=0;
        }
    }
    PdhRemoveCounter(english);
    if(localized&&PdhSetCounterScaleFactor(localized,s->scale)){PdhRemoveCounter(localized);localized=0;}
    return localized;
}
static void adapter_capture_begin(ADAPTER_CAPTURE*c){
    c->query=0;c->baseline=0;c->error=0;
    for(unsigned i=0;i<ADAPTER_METRIC_COUNT;i++){c->h[i]=0;ADAPTER_SUMMARY empty={0,0,0,0,0,0,0};c->m[i]=empty;}
    c->error=PdhOpenQueryW(0,0,&c->query);
    if(c->error||!c->query){c->query=0;return;}
    unsigned added=0;
    for(unsigned i=0;i<ADAPTER_METRIC_COUNT;i++){c->h[i]=adapter_add(c->query,&adapter_specs[i]);if(c->h[i])added++;}
    if(added){c->error=PdhCollectQueryData(c->query);c->baseline=!c->error;}
}
static ADAPTER_SUMMARY adapter_read(HANDLE h,int peakOnly){
    ADAPTER_SUMMARY s={0,0,0,0,0,0,0};
    DWORD bytes=sizeof(adapter_array.bytes),count=0;
    if(PdhGetFormattedCounterArrayW(h,0x8400,&bytes,&count,(BLAZE_PDH_ITEM*)adapter_array.bytes)||bytes>sizeof(adapter_array.bytes)||count>bytes/sizeof(BLAZE_PDH_ITEM))return s;
    const BLAZE_PDH_ITEM*items=(const BLAZE_PDH_ITEM*)adapter_array.bytes;
    DWORD nameStart=count*sizeof(BLAZE_PDH_ITEM);
    for(DWORD i=0;i<count;i++){
        const BLAZE_PDH_ITEM*p=&items[i];
        if(!adapter_string(adapter_array.bytes,bytes,p->name,nameStart)||!p->name[0]){s.invalid++;continue;}
        if(adapter_is_total(p->name)){s.skipped++;continue;}
        if((p->value.CStatus!=0&&p->value.CStatus!=1)||p->value.value.largeValue<0||(peakOnly&&p->value.value.largeValue>100)){s.invalid++;continue;}
        ULONGLONG v=(ULONGLONG)p->value.value.largeValue;
        s.valid++;if(v>s.peak)s.peak=v;
        if(!peakOnly&&!health_add(s.total,v,&s.total))s.overflow=1;
    }
    s.available=s.valid>0;
    return s;
}
static void adapter_capture_end(ADAPTER_CAPTURE*c){
    if(!c->query)return;
    int ok=0;if(c->baseline){c->error=PdhCollectQueryData(c->query);ok=!c->error;}
    for(unsigned i=0;i<ADAPTER_METRIC_COUNT;i++){
        if(ok&&c->h[i])c->m[i]=adapter_read(c->h[i],adapter_specs[i].peakOnly);
        c->h[i]=0;
    }
    PdhCloseQuery(c->query);c->query=0;
}
