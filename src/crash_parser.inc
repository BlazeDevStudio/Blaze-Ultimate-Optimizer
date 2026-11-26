/* Read only the fields required for grouping; never retain application paths.
 * Caller supplies an aligned EVENTLOGRECORD validated against its read buffer.
 * Offsets below are checked before any insertion-string access. */
static int crash_span_string(const EVENTLOGRECORD*r,DWORD*off,DWORD end,DWORD*start,DWORD*units){
    *start=*off;*units=0;
    while(*off<=end&&end-*off>=2){
        WCHAR c=*(const WCHAR*)((const BYTE*)r+*off);*off+=2;
        if(!c)return 1;
        (*units)++;
    }
    return 0;
}
static int crash_label(const EVENTLOGRECORD*r,DWORD start,DWORD units,WCHAR*out,UINT cap,int basename){
    const WCHAR*s=(const WCHAR*)((const BYTE*)r+start);
    DWORD first=0;
    if(basename)for(DWORD i=0;i<units;i++)if(s[i]==L'\\'||s[i]==L'/'||s[i]==L':')first=i+1;
    if(units-first>=cap)return 0; /* Do not merge distinct truncated labels. */
    UINT n=0;
    for(DWORD i=first;i<units;i++){
        WCHAR c=s[i];
        if(c<32||(c>=127&&c<=159)||(c>=0x202A&&c<=0x202E)||(c>=0x2066&&c<=0x2069)||c==0x2028||c==0x2029)
            return 0; /* Reject misleading control characters in displayed fields. */
        out[n++]=c;
    }
    out[n]=0;return 1;
}
static int crash_exception(const WCHAR*s){
    UINT i=0,n=0;if(s[0]==L'0'&&(s[1]==L'x'||s[1]==L'X'))i=2;
    for(;s[i];i++,n++){WCHAR c=s[i];if(!((c>=L'0'&&c<=L'9')||(c>=L'a'&&c<=L'f')||(c>=L'A'&&c<=L'F')))return 0;}
    return n==8;
}
static int store_crash(LPCWSTR app,LPCWSTR mod,LPCWSTR exception,DWORD epoch){
    int k=0;
    for(;k<g_app_names_count;k++)if(same_ci(g_faults[k].name,app)&&same_ci(g_faults[k].module,mod)&&same_ci(g_faults[k].exception,exception))break;
    if(k>=12)return 0;
    if(k==g_app_names_count){
        g_app_names_count++;
        wcopy(g_faults[k].name,app,120);wcopy(g_faults[k].module,mod,120);
        wcopy(g_faults[k].exception,exception,32);
        g_faults[k].count=0;g_faults[k].newest=0;
    }
    g_faults[k].count++;
    if(epoch>g_faults[k].newest)g_faults[k].newest=epoch;
    return 1;
}
static int crash_parse(const EVENTLOGRECORD*r){
    if(r->Length<sizeof(EVENTLOGRECORD)+4||r->NumStrings<7)return 0;
    DWORD end=r->Length-4,off=r->StringOffset;
    if(off<sizeof(EVENTLOGRECORD)||off>=end||(off&1))return 0;
    if(r->UserSidLength){
        if(r->UserSidOffset<sizeof(EVENTLOGRECORD)||r->UserSidOffset>off||r->UserSidLength>off-r->UserSidOffset)return 0;
    }
    if(r->DataLength){
        if(r->DataOffset<off||r->DataOffset>end||r->DataLength>end-r->DataOffset)return 0;
        end=r->DataOffset;
    }
    /* Source name and computer name must end before SID or insertions. */
    DWORD names=sizeof(EVENTLOGRECORD),name_end=r->UserSidLength?r->UserSidOffset:off,start,units;
    if(!crash_span_string(r,&names,name_end,&start,&units)||!crash_span_string(r,&names,name_end,&start,&units))return 0;
    WCHAR app[120],mod[120],exception[32];app[0]=mod[0]=exception[0]=0;
    for(unsigned k=0;k<r->NumStrings;k++){
        if(!crash_span_string(r,&off,end,&start,&units))return 0;
        if(k==0&&!crash_label(r,start,units,app,120,1))return 0;
        if(k==3&&!crash_label(r,start,units,mod,120,1))return 0;
        if(k==6&&!crash_label(r,start,units,exception,32,0))return 0;
    }
    if(!app[0]||!mod[0]||!crash_exception(exception))return 0;
    /* Canonicalize the optional 0x prefix and letter case for grouping. */
    UINT first=(exception[0]==L'0'&&(exception[1]==L'x'||exception[1]==L'X'))?2:0;
    for(UINT i=0;i<8;i++){WCHAR c=exception[first+i];exception[i]=(c>=L'a'&&c<=L'f')?c-32:c;}exception[8]=0;
    return store_crash(app,mod,exception,r->TimeGenerated)?1:2;
}
static void crash_insertions(const EVENTLOGRECORD*r){
    int status=crash_parse(r);
    if(status==1)g_crash_grouped++;
    else if(status==2)g_crash_ungrouped++;
    else g_crash_rejected++;
}
