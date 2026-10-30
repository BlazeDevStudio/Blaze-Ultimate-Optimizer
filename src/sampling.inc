static void metric_cpu(void){
    FILETIME i0,k0,u0,i1,k1,u1,cr,ex,pk,pu;
    DWORD count=0;
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snap!=INVALID_HANDLE_VALUE){
        PROCESSENTRY32W p; p.dwSize=sizeof(p);
        if(Process32FirstW(snap,&p))do{
            if(count>=320)break;
            HANDLE h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,p.th32ProcessID);
            if(!h)continue;
            if(GetProcessTimes(h,&cr,&ex,&pk,&pu)){
                g_cpu_samples[count].h=h;
                g_cpu_samples[count].pid=p.th32ProcessID;
                wcopy(g_cpu_samples[count].name,p.szExeFile,MAX_PATH);
                g_cpu_samples[count].ticks=ft64(pk)+ft64(pu);
                count++;
            }else CloseHandle(h);
        }while(Process32NextW(snap,&p));
        CloseHandle(snap);
    }
    int ok=GetSystemTimes(&i0,&k0,&u0);
    Sleep(1500);
    if(ok&&GetSystemTimes(&i1,&k1,&u1)){
        ULONGLONG idle=ft64(i1)-ft64(i0),kernel=ft64(k1)-ft64(k0),usr=ft64(u1)-ft64(u0),total=kernel+usr;
        if(total){
            g_cpu=(unsigned)((total>idle)?((total-idle)*100ULL/total):0);
            g_cpu_ok=1;
            for(DWORD i=0;i<count;i++){
                if(GetProcessTimes(g_cpu_samples[i].h,&cr,&ex,&pk,&pu)){
                    ULONGLONG ticks=ft64(pk)+ft64(pu);
                    ULONGLONG used=ticks>=g_cpu_samples[i].ticks?ticks-g_cpu_samples[i].ticks:0;
                    unsigned pct=(unsigned)((used>total)?100:(used*100ULL/total));
                    if(pct>g_cpuTopPct){g_cpuTopPct=pct;wcopy(g_cpuTopName,g_cpu_samples[i].name,MAX_PATH);}
                }
            }
        }
    }
    for(DWORD i=0;i<count;i++)CloseHandle(g_cpu_samples[i].h);
}
typedef struct {ULONGLONG rOps,wOps,oOps,rBytes,wBytes,oBytes;} IO_COUNT;
/* Report process I/O deltas, not physical SSD I/O, and both soft+hard page faults. */
static void activity_sample(void){
    static HANDLE hs[240];static DWORD pf[240];static ULONGLONG rd[240],wr[240];static BOOL havepf[240],haveio[240];
    DWORD n=0;HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snap!=INVALID_HANDLE_VALUE){
      PROCESSENTRY32W p;p.dwSize=sizeof(p);
      if(Process32FirstW(snap,&p))do{
        if(n>=240)break;
        HANDLE h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ,0,p.th32ProcessID);
        if(!h)h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,0,p.th32ProcessID);
        if(!h)continue;
        hs[n]=h;PROCESS_MEMORY_COUNTERS_EX pm;pm.cb=sizeof(pm);
        havepf[n]=K32GetProcessMemoryInfo(h,&pm,sizeof(pm));pf[n]=havepf[n]?pm.PageFaultCount:0;
        IO_COUNT io;haveio[n]=GetProcessIoCounters(h,&io);
        rd[n]=haveio[n]?io.rBytes:0;wr[n]=haveio[n]?io.wBytes:0;n++;
      }while(Process32NextW(snap,&p));
      CloseHandle(snap);
    }
    FILETIME a,b,c,d,e,f;ULONGLONG elapsed=0,busy=0;
    unsigned peak=0;BOOL ok=GetSystemTimes(&a,&b,&c);
    ULONGLONG lo=0,hi=0;
    for(int k=0;k<5;k++){
      MEMORYSTATUSEX m;m.dwLength=sizeof(m);
      if(GlobalMemoryStatusEx(&m)){
        if(!lo||m.ullAvailPhys<lo)lo=m.ullAvailPhys;
        if(m.ullAvailPhys>hi)hi=m.ullAvailPhys;
      }
      Sleep(1000);
      if(ok&&GetSystemTimes(&d,&e,&f)){
        ULONGLONG idle=ft64(d)-ft64(a),total=ft64(e)-ft64(b)+ft64(f)-ft64(c);
        ULONGLONG use=total>idle?total-idle:0;
        if(total){
          unsigned pct=(unsigned)(100ULL*use/total);if(pct>peak)peak=pct;
          elapsed+=total;busy+=use;
        }
        a=d;b=e;c=f;
      }else ok=FALSE;
    }
    ULONGLONG faults=0,reads=0,writes=0;
    for(DWORD i=0;i<n;i++){
      PROCESS_MEMORY_COUNTERS_EX pm;pm.cb=sizeof(pm);IO_COUNT io;
      if(havepf[i]&&K32GetProcessMemoryInfo(hs[i],&pm,sizeof(pm))&&pm.PageFaultCount>=pf[i])
        faults+=pm.PageFaultCount-pf[i];
      if(haveio[i]&&GetProcessIoCounters(hs[i],&io)){
        if(io.rBytes>=rd[i])reads+=io.rBytes-rd[i];
        if(io.wBytes>=wr[i])writes+=io.wBytes-wr[i];
      }
      CloseHandle(hs[i]);
    }
    heading(L"FIVE-SECOND PROCESS ACTIVITY (INCOMPLETE USER-MODE SAMPLE)");
    append(L"CPU average: ");if(elapsed)num(busy*100ULL/elapsed);else append(L"unavailable");
    append(L"% | peak one-second CPU: ");num(peak);nl();
    append(L"Available physical RAM range: ");fmt_size(lo);append(L" to ");fmt_size(hi);nl();
    append(L"Monitored processes: ");num(n);nl();
    append(L"Page fault delta (SOFT + HARD, not hard faults/sec): ");num(faults);nl();
    append(L"Process transfer bytes, read: ");fmt_size(reads);append(L", write: ");fmt_size(writes);nl();
    line(L"Process I/O can include non-disk sources; results cannot establish SSD latency or physical disk throughput.");
}
