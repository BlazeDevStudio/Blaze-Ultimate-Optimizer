static void reg_count(HKEY root,LPCWSTR key,const WCHAR*title){HKEY h=0;if(RegOpenKeyExW(root,key,0,KEY_READ,&h)!=0)return;DWORD idx=0;WCHAR name[256];WCHAR data[1024];DWORD namelen,type,datalen;int c=0;for(;idx<120;idx++){namelen=255;datalen=sizeof(data);type=0;if(RegEnumValueW(h,idx,name,&namelen,0,&type,(BYTE*)data,&datalen)!=0)break;name[namelen]=0;if(c<20){append(L"  ");append(title);append(L": ");append(name);nl();}c++;}g_run_count+=c;RegCloseKey(h);}
static void group_process(LPCWSTR name,ULONGLONG priv,ULONGLONG ws){
    int k=0;for(;k<g_group_count;k++)if(same_ci(g_group[k].name,name))break;
    if(k==g_group_count){if(k>=64)return;wcopy(g_group[k].name,name,MAX_PATH);g_group[k].priv=0;g_group[k].working=0;g_group[k].processes=0;g_group_count++;}
    g_group[k].priv+=priv;g_group[k].working+=ws;g_group[k].processes++;
}
static void store_crash(LPCWSTR app,LPCWSTR mod,LPCWSTR exception,LPCWSTR path,DWORD eventEpoch){
    if(!app||!app[0])app=L"(name not available)";
    if(!mod)mod=L"";if(!exception)exception=L"";
    int k=0;
    for(;k<g_app_names_count;k++)if(same_ci(g_faults[k].name,app)&&same_ci(g_faults[k].module,mod)&&same_ci(g_faults[k].exception,exception))break;
    if(k>=12)return;
    if(k==g_app_names_count){
        g_app_names_count++;
        wcopy(g_faults[k].name,app,120);wcopy(g_faults[k].module,mod,120);
        wcopy(g_faults[k].exception,exception,32);wcopy(g_faults[k].appPath,path?path:L"",260);
        g_faults[k].count=0;g_faults[k].newest=0;
    }
    g_faults[k].count++;
    if(eventEpoch>g_faults[k].newest)g_faults[k].newest=eventEpoch;
}
static void crash_insertions(const EVENTLOGRECORD*r){
    /* Application Error event 1000 insertion order is commonly:
       1 app; 4 module; 7 exception code; 11 application path.
       Read only NUL-terminated UTF-16 strings within the original record. */
    if(r->NumStrings<1||r->StringOffset>=r->Length||r->StringOffset<sizeof(EVENTLOGRECORD)||(r->StringOffset&1))return;
    DWORD byteOff=r->StringOffset;
    WCHAR app[120],mod[120],exception[32],path[260];
    app[0]=mod[0]=exception[0]=path[0]=0;
    for(unsigned k=0;k<r->NumStrings&&k<11;k++){
        WCHAR tmp[260];UINT j=0;int terminated=0;
        while(byteOff+2<=r->Length){
            WCHAR wc=*(const WCHAR*)((const BYTE*)r+byteOff);byteOff+=2;
            if(!wc){terminated=1;break;}
            if(j+1<260)tmp[j++]=wc;
        }
        if(!terminated)return;
        tmp[j]=0;
        if(k==0)wcopy(app,tmp,120);
        if(k==3)wcopy(mod,tmp,120);
        if(k==6)wcopy(exception,tmp,32);
        if(k==10)wcopy(path,tmp,260);
    }
    store_crash(app,mod,exception,path,r->TimeGenerated);
}
static void process_snapshot(void){HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snap==INVALID_HANDLE_VALUE)return;PROCESSENTRY32W p;p.dwSize=sizeof(p);if(Process32FirstW(snap,&p)){do{HANDLE h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ,0,p.th32ProcessID);if(!h)h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,0,p.th32ProcessID);if(h){PROCESS_MEMORY_COUNTERS_EX pm;pm.cb=sizeof(pm);if(K32GetProcessMemoryInfo(h,&pm,sizeof(pm))){ULONGLONG m=(ULONGLONG)pm.WorkingSetSize;group_process(p.szExeFile,(ULONGLONG)pm.PrivateUsage,m);for(int k=0;k<8;k++){if(m>g_top[k].usage){for(int q=7;q>k;q--)g_top[q]=g_top[q-1];g_top[k].pid=p.th32ProcessID;g_top[k].usage=m;wcopy(g_top[k].name,p.szExeFile,MAX_PATH);break;}}}CloseHandle(h);}}while(Process32NextW(snap,&p));}CloseHandle(snap);}
#include "event_scan.inc"
