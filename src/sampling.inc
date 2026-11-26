#include "health_math.inc"
#include "disk_metrics.inc"
#include "disk_report.inc"
#include "adapter_metrics.inc"
#include "adapter_report.inc"
#include "history_ring.inc"
#include "history_report.inc"
static int read_cpu(CPU_COUNTERS*out){
    FILETIME i,k,u;
    if(!GetSystemTimes(&i,&k,&u))return 0;
    out->idle=ft64(i);out->kernel=ft64(k);out->user=ft64(u);return 1;
}
static int process_ticks(HANDLE h,ULONGLONG*out){
    FILETIME cr,ex,k,u;
    return GetProcessTimes(h,&cr,&ex,&k,&u)&&health_add(ft64(k),ft64(u),out);
}
static void metric_cpu(void){
    DWORD count=0,seen=0,paired=0;
    WORD groups=GetActiveProcessorGroupCount();
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    int listed=0;
    if(snap!=INVALID_HANDLE_VALUE){
        PROCESSENTRY32W p;p.dwSize=sizeof(p);
        if(Process32FirstW(snap,&p)){
            listed=1;
            do{
                seen++;
                if(groups!=1||count>=320)continue;
                HANDLE h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,p.th32ProcessID);
                if(!h)continue;
                ULONGLONG ticks;
                if(process_ticks(h,&ticks)){
                    g_cpu_samples[count].h=h;g_cpu_samples[count].pid=p.th32ProcessID;
                    wcopy(g_cpu_samples[count].name,p.szExeFile,MAX_PATH);
                    g_cpu_samples[count].ticks=ticks;count++;
                }else CloseHandle(h);
            }while(Process32NextW(snap,&p));
        }
        CloseHandle(snap);
    }
    CPU_COUNTERS a,b;CPU_DELTA d;
    int ok=read_cpu(&a);
    Sleep(1500);
    if(read_cpu(&b)&&ok&&health_cpu_delta(a,b,&d)){
        g_cpu=health_percent(d.busy,d.total);g_cpu_ok=1;
        for(DWORD i=0;i<count;i++){
            ULONGLONG ticks;
            if(process_ticks(g_cpu_samples[i].h,&ticks)&&ticks>=g_cpu_samples[i].ticks){
                ULONGLONG used=ticks-g_cpu_samples[i].ticks;
                if(used>d.total)continue; /* Endpoint skew or invalid counter. */
                paired++;
                unsigned pct=health_percent(used,d.total);
                if(pct>g_cpuTopPct){g_cpuTopPct=pct;wcopy(g_cpuTopName,g_cpu_samples[i].name,MAX_PATH);}
            }
        }
    }
    for(DWORD i=0;i<count;i++)CloseHandle(g_cpu_samples[i].h);
    if(groups!=1)line(L"CPU scope: processor group only; per-process CPU shares unavailable on multi-group/unknown topology.");
    append(L"CPU process sample pairs: ");num(paired);append(L" of ");num(count);append(L" opened; ");
    if(listed){num(seen);line(L" process entries observed (bounded to 320 handles).");}
    else line(L"process enumeration unavailable.");
}
typedef struct {ULONGLONG rOps,wOps,oOps,rBytes,wBytes,oBytes;} IO_COUNT;
/* Report process I/O deltas, not physical SSD I/O, and both soft+hard page faults. */
static void activity_sample(void){
    static HANDLE hs[240];static DWORD pf[240];static ULONGLONG rd[240],wr[240];static BOOL havepf[240],haveio[240];
    DWORD n=0,seen=0;int listed=0;
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snap!=INVALID_HANDLE_VALUE){
      PROCESSENTRY32W p;p.dwSize=sizeof(p);
      if(Process32FirstW(snap,&p)){listed=1;do{
        seen++;if(n>=240)continue;
        HANDLE h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ,0,p.th32ProcessID);
        if(!h)h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,0,p.th32ProcessID);
        if(!h)continue;
        hs[n]=h;PROCESS_MEMORY_COUNTERS_EX pm;pm.cb=sizeof(pm);
        havepf[n]=K32GetProcessMemoryInfo(h,&pm,sizeof(pm));pf[n]=havepf[n]?pm.PageFaultCount:0;
        IO_COUNT io;haveio[n]=GetProcessIoCounters(h,&io);
        rd[n]=haveio[n]?io.rBytes:0;wr[n]=haveio[n]?io.wBytes:0;n++;
      }while(Process32NextW(snap,&p));}
      CloseHandle(snap);
    }
    CPU_COUNTERS a,b;CPU_DELTA d;ULONGLONG elapsed=0,busy=0;
    unsigned peak=0;int previous=read_cpu(&a);
    HEALTH_RANGE ram={0,0,0};
    unsigned memhot=0,memsum=0;
    DISK_CAPTURE disk;disk_capture_begin(&disk);
    static ADAPTER_CAPTURE adapters;adapter_capture_begin(&adapters);
    for(int k=0;k<5;k++){
      MEMORYSTATUSEX m;m.dwLength=sizeof(m);
      if(GlobalMemoryStatusEx(&m)&&health_memory_valid(m.ullTotalPhys,m.ullAvailPhys,m.dwMemoryLoad)){
        health_range_add(&ram,m.ullAvailPhys);memsum+=m.dwMemoryLoad;if(m.dwMemoryLoad>=83)memhot++;
      }
      Sleep(1000);
      int current=read_cpu(&b);
      if(current&&previous&&health_cpu_delta(a,b,&d)){
        ULONGLONG nt,nb;
        if(health_add(elapsed,d.total,&nt)&&health_add(busy,d.busy,&nb)){
          unsigned pct=health_percent(d.busy,d.total);if(pct>peak)peak=pct;
          elapsed=nt;busy=nb;g_cpu_intervals++;if(pct>=85)g_cpu_hot_intervals++;
        }
      }
      if(current)a=b;
      previous=current; /* Rebaseline after missing readings; never bridge gaps. */
    }
    adapter_capture_end(&adapters);
    disk_capture_end(&disk);
    ULONGLONG faults=0,reads=0,writes=0;unsigned pfPairs=0,ioPairs=0;int ioOverflow=0;
    for(DWORD i=0;i<n;i++){
      PROCESS_MEMORY_COUNTERS_EX pm;pm.cb=sizeof(pm);IO_COUNT io;
      if(havepf[i]&&K32GetProcessMemoryInfo(hs[i],&pm,sizeof(pm))&&pm.PageFaultCount>=pf[i]){
        faults+=pm.PageFaultCount-pf[i];pfPairs++;
      }
      if(haveio[i]&&GetProcessIoCounters(hs[i],&io)&&io.rBytes>=rd[i]&&io.wBytes>=wr[i]){
        ULONGLONG nr,nw;
        if(health_add(reads,io.rBytes-rd[i],&nr)&&health_add(writes,io.wBytes-wr[i],&nw)){
          reads=nr;writes=nw;ioPairs++;
        }else ioOverflow=1;
      }
      CloseHandle(hs[i]);
    }
    heading(L"FIVE-SECOND PROCESS ACTIVITY (INCOMPLETE USER-MODE SAMPLE)");
    append(L"CPU average: ");if(elapsed){num(health_percent(busy,elapsed));append(L"% | peak interval CPU: ");num(peak);append(L"%");}else append(L"unavailable");
    append(L" | valid intervals: ");num(g_cpu_intervals);line(L"/5");
    append(L"CPU intervals at or above 85%: ");num(g_cpu_hot_intervals);append(L"/");num(g_cpu_intervals);nl();
    if(g_cpu_intervals==5&&g_cpu_hot_intervals==5)line(L"CPU stayed at or above 85% across this short capture; investigate before optimizing.");
    append(L"Available physical RAM range: ");if(ram.count){fmt_size(ram.low);append(L" to ");fmt_size(ram.high);}else append(L"unavailable");
    append(L" | valid snapshots: ");num(ram.count);line(L"/5");
    if(ram.count==5&&memhot==5)line(L"RAM load stayed at or above 83% across all five snapshots; longer observation is needed.");
    append(L"Opened processes: ");num(n);append(L" (240 max); ");
    if(listed){num(seen);line(L" process entries observed.");}else line(L"process enumeration unavailable.");
    append(L"Page fault delta (SOFT + HARD, not hard faults/sec): ");if(pfPairs)num(faults);else append(L"unavailable");
    append(L" | paired processes: ");num(pfPairs);append(L"/");num(n);nl();
    append(L"Process transfer bytes, read: ");if(ioPairs&&!ioOverflow){fmt_size(reads);append(L", write: ");fmt_size(writes);}else append(L"unavailable");
    append(L" | paired processes: ");num(ioPairs);append(L"/");num(n);nl();
    line(L"Process I/O can include non-disk sources; results cannot establish SSD latency or physical disk throughput.");
    disk_report(&disk);
    adapter_report(&adapters);
    history_capture(elapsed?health_percent(busy,elapsed):0,g_cpu_intervals,ram.count?memsum/ram.count:0,ram.count,&disk,&adapters);
}
