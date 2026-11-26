static void reg_count(HKEY root,LPCWSTR key,const WCHAR*title){HKEY h=0;if(RegOpenKeyExW(root,key,0,KEY_READ,&h)!=0)return;DWORD idx=0;WCHAR name[256];WCHAR data[1024];DWORD namelen,type,datalen;int c=0;for(;idx<120;idx++){namelen=255;datalen=sizeof(data);type=0;if(RegEnumValueW(h,idx,name,&namelen,0,&type,(BYTE*)data,&datalen)!=0)break;name[namelen]=0;if(c<20){append(L"  ");append(title);append(L": ");append(name);nl();}c++;}g_run_count+=c;RegCloseKey(h);}
static void group_process(LPCWSTR name,ULONGLONG priv,ULONGLONG ws){
    int k=0;for(;k<g_group_count;k++)if(same_ci(g_group[k].name,name))break;
    if(k==g_group_count){if(k>=64)return;wcopy(g_group[k].name,name,MAX_PATH);g_group[k].priv=0;g_group[k].working=0;g_group[k].processes=0;g_group_count++;}
    g_group[k].priv+=priv;g_group[k].working+=ws;g_group[k].processes++;
}
#include "crash_parser.inc"
static void process_snapshot(void){HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snap==INVALID_HANDLE_VALUE)return;PROCESSENTRY32W p;p.dwSize=sizeof(p);if(Process32FirstW(snap,&p)){do{HANDLE h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ,0,p.th32ProcessID);if(!h)h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,0,p.th32ProcessID);if(h){PROCESS_MEMORY_COUNTERS_EX pm;pm.cb=sizeof(pm);if(K32GetProcessMemoryInfo(h,&pm,sizeof(pm))){ULONGLONG m=(ULONGLONG)pm.WorkingSetSize;group_process(p.szExeFile,(ULONGLONG)pm.PrivateUsage,m);for(int k=0;k<8;k++){if(m>g_top[k].usage){for(int q=7;q>k;q--)g_top[q]=g_top[q-1];g_top[k].pid=p.th32ProcessID;g_top[k].usage=m;wcopy(g_top[k].name,p.szExeFile,MAX_PATH);break;}}}CloseHandle(h);}}while(Process32NextW(snap,&p));}CloseHandle(snap);}
#include "event_scan.inc"
