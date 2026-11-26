#include "recovery_store.inc"
static void visual_optimize(void){
    g_opt_ok=0;HANDLE lock=rec_lock();if(!lock){line(L"Recovery lock unavailable; visual changes skipped.");return;}
    HKEY h=0;
    if(RegCreateKeyExW(HKEY_CURRENT_USER,recovery_path,0,0,0,KEY_READ|KEY_WRITE,0,&h,0)){line(L"Recovery store unavailable; visual changes skipped.");rec_unlock(lock);return;}
    REC_RECORD r;DWORD legacy=0;
    int state=rec_load(h,visual_record,REC_VISUAL,&r),old=rec_read_dword(h,L"BackupExists",&legacy);
    if(state==REC_INVALID||old!=REC_ABSENT){line(L"Invalid or legacy visual backup present; no new changes. Use restore/review first.");goto done;}
    if(state==REC_ABSENT){
        DWORD effect=0,t=0;int ts=transparency_read(&t);
        if(ts==REC_INVALID||!effects_read(&effect)){line(L"Original visual configuration unreadable; no changes.");goto done;}
        r=rec_make(REC_VISUAL,effect,ts==REC_VALID,t);
        if(!rec_save(h,visual_record,r)){line(L"Recovery record persistence or verification failed; no changes.");goto done;}
    }else if(RegFlushKey(h)){line(L"Existing recovery record could not be flushed; no changes.");goto done;}
    {
        int a=effects_apply(0),b=transparency_apply(1,0);g_opt_ok=a||b;
        line(a?L"UI effects off: verified.":L"UI effects change not verified; backup retained.");
        line(b?L"Transparency registry value off: verified (sign-out may be needed).":L"Transparency change not verified; backup retained.");
    }
done: RegCloseKey(h);rec_unlock(lock);
}
static void visual_restore(void){
    HANDLE lock=rec_lock();if(!lock){MessageBoxW(g_main,L"Recovery lock unavailable. No changes.",L"Restore",MB_OK|MB_ICONWARNING);return;}
    HKEY h=0;LPCWSTR msg=L"No readable saved visual preferences.";int success=0;
    if(!RegOpenKeyExW(HKEY_CURRENT_USER,recovery_path,0,KEY_READ|KEY_WRITE,&h)){
        REC_RECORD r;int state=rec_load(h,visual_record,REC_VISUAL,&r);
        DWORD marker=0,orig=0,exists=0,t=0;int legacy=rec_read_dword(h,L"BackupExists",&marker);
        if(state==REC_VALID&&legacy==REC_ABSENT){
            int a=effects_apply(r.a),b=transparency_apply(r.b,r.c);
            if(a&&b){success=rec_clear(h,visual_record,r);msg=success?L"Recorded visual preferences restored and verified. Temporary files cannot be restored.":L"Settings verified; backup retirement failed. Review before retry.";}
            else msg=L"Restoration incomplete or unverified. Recovery record retained; retry Restore Preferences.";
        }else if(state==REC_ABSENT&&legacy==REC_VALID&&marker==1&&
            rec_read_dword(h,L"OriginalUIEffects",&orig)==REC_VALID&&orig<=1&&
            rec_read_dword(h,L"TransparencyExisted",&exists)==REC_VALID&&exists<=1&&
            (!exists||(rec_read_dword(h,L"OriginalTransparency",&t)==REC_VALID&&t<=1))){
            if(MessageBoxW(g_main,L"Legacy backup has no checksum and may be incomplete historically. Restore the recorded values only?",L"Legacy recovery",MB_YESNO|MB_ICONWARNING)==IDYES){
                int a=effects_apply(orig),b=transparency_apply(exists,t);
                if(a&&b){success=!RegDeleteValueW(h,L"BackupExists")&&!RegFlushKey(h);msg=success?L"Legacy recorded values restored and verified.":L"Legacy values verified; backup retirement failed.";}
                else msg=L"Legacy restoration incomplete; backup retained.";
            }else msg=L"Legacy restoration canceled.";
        }else if(state!=REC_ABSENT||legacy!=REC_ABSENT)msg=L"Recovery record invalid or conflicting. No changes; preserve the backup for manual review.";
        RegCloseKey(h);
    }
    rec_unlock(lock);MessageBoxW(g_main,msg,L"Restore preferences",MB_OK|(success?MB_ICONINFORMATION:MB_ICONWARNING));
}
static void clean_user_temp(void){g_deleted=0;g_freed=0;WCHAR dir[512],pat[520],full[650];DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",dir,480);if(!n||n>=480){line(L"Personal LOCALAPPDATA unavailable. Cleanup skipped to avoid shared/system temp folders.");return;}wcat(dir,L"\\Temp\\",512);static WCHAR check[512];wcopy(check,dir,512);UINT checklen=wlen(check);if(checklen&&check[checklen-1]==L'\\')check[checklen-1]=0;DWORD attr=GetFileAttributesW(check);if(attr==INVALID_FILE_ATTRIBUTES||!(attr&FILE_ATTRIBUTE_DIRECTORY)||(attr&FILE_ATTRIBUTE_REPARSE_POINT)){line(L"Personal TEMP folder missing or a directory link; cleanup skipped to protect other locations.");return;}wcopy(pat,dir,520);wcat(pat,L"*",520);WIN32_FIND_DATAW f;HANDLE h=FindFirstFileW(pat,&f);if(h==INVALID_HANDLE_VALUE){line(L"Temp listing failed. Cleanup skipped.");return;}FILETIME now;GetSystemTimeAsFileTime(&now);ULONGLONG cutoff=ft64(now)-7ULL*86400ULL*10000000ULL;do{if(f.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT))continue;if(f.cFileName[0]==L'.')continue;if(ft64(f.ftLastWriteTime)>cutoff)continue;wcopy(full,dir,650);wcat(full,f.cFileName,650);if(DeleteFileW(full)){g_deleted++;g_freed+=((ULONGLONG)f.nFileSizeHigh<<32)|f.nFileSizeLow;}}while(g_deleted<25000&&FindNextFileW(h,&f));FindClose(h);append(L"Old top-level temp files removed (>7 days old): ");num(g_deleted);append(L"; apparent freed size: ");fmt_size(g_freed);nl();line(L"Safety limits: no subfolders, shortcuts/reparse points, recent files, or Windows system directories touched. Locked files were left alone.");}
static const WCHAR* riskServices[2]={L"WSearch",L"SysMain"};
static const WCHAR* riskRegistry[2]={L"ServiceRecovery_WSearch_V1",L"ServiceRecovery_SysMain_V1"};
static const WCHAR* legacyRegistry[2]={L"ServiceBackup_WSearch",L"ServiceBackup_SysMain"};
static int service_read(HANDLE svc,DWORD*start,DWORD*delayed){
    static union {ULONGLONG align;BYTE bytes[8192];} b;
    DWORD needed=0;QSVC*cfg=(QSVC*)b.bytes;
    if(!QueryServiceConfigW(svc,b.bytes,sizeof(b.bytes),&needed)||cfg->dwStartType<2||cfg->dwStartType>4)return 0;
    *start=cfg->dwStartType;*delayed=0;
    if(*start==2){BOOL flag=0;if(!QueryServiceConfig2W(svc,3,(BYTE*)&flag,sizeof(flag),&needed))return 0;*delayed=flag?1:0;}
    return 1;
}
static int service_apply(HANDLE svc,DWORD start,DWORD delayed){
    DWORD actual=0,ad=0;if(!service_read(svc,&actual,&ad))return 0;
    if(actual!=start&&!ChangeServiceConfigW(svc,SERVICE_NO_CHANGE,start,SERVICE_NO_CHANGE,0,0,0,0,0,0,0))return 0;
    if(start==2){BOOL flag=delayed?TRUE:FALSE;if(!ChangeServiceConfig2W(svc,3,&flag))return 0;}
    return service_read(svc,&actual,&ad)&&actual==start&&ad==delayed;
}
static void risky_services(int restore){
    heading(restore?L"ADVANCED RISK MODE - SERVICE ROLLBACK":L"ADVANCED RISK MODE - OPT-IN SERVICE STARTUP CHANGES");
    HANDLE lock=rec_lock();if(!lock){line(L"Recovery lock unavailable; no changes.");return;}
    HANDLE scm=OpenSCManagerW(0,0,SC_MANAGER_CONNECT);
    if(!scm){line(L"Service Control Manager unavailable; no changes.");rec_unlock(lock);return;}
    HKEY key=0,legacy=0;
    if(RegCreateKeyExW(HKEY_LOCAL_MACHINE,recovery_path,0,0,0,KEY_READ|KEY_WRITE|0x0100,0,&key,0)){line(L"Machine recovery store unavailable; no changes.");goto finish;}
    LONG le=RegOpenKeyExW(HKEY_CURRENT_USER,recovery_path,0,KEY_READ,&legacy);
    if(le&&le!=2){line(L"Legacy recovery store unreadable; no changes.");goto finish;}
    for(int i=0;i<2;i++){
        DWORD kind=i?REC_SYSMAIN:REC_SEARCH,old=0,from=0,delayed=0;
        REC_RECORD r;int state=rec_load(key,riskRegistry[i],kind,&r);
        int ls=legacy?rec_read_dword(legacy,legacyRegistry[i],&old):REC_ABSENT;
        append(riskServices[i]);append(L": ");
        if(state==REC_INVALID||ls!=REC_ABSENT){line(L"invalid/conflicting or legacy backup; preserve for manual review (legacy delayed-start state unknown).");continue;}
        if(restore&&state==REC_ABSENT){line(L"no recovery record.");continue;}
        HANDLE svc=OpenServiceW(scm,riskServices[i],SERVICE_QUERY_CONFIG|SERVICE_CHANGE_CONFIG);
        if(!svc){line(L"service unavailable or insufficient rights.");continue;}
        if(!service_read(svc,&from,&delayed)){line(L"original configuration unreadable; skipped.");CloseServiceHandle(svc);continue;}
        if(!restore&&state==REC_ABSENT){
            r=rec_make(kind,from,delayed,0);
            if(!rec_save(key,riskRegistry[i],r)){line(L"backup persistence/verification failed; refused change.");CloseServiceHandle(svc);continue;}
        }else if(!restore&&RegFlushKey(key)){line(L"backup flush failed; refused change.");CloseServiceHandle(svc);continue;}
        int ok=service_apply(svc,restore?r.a:4,restore?r.b:0);
        if(!ok)line(L"change incomplete or unverified; recovery record retained.");
        else if(restore)line(rec_clear(key,riskRegistry[i],r)?L"original startup type and delayed-start state restored and verified.":L"configuration verified; backup retirement failed.");
        else line(L"disabled startup verified; recovery record retained.");
        CloseServiceHandle(svc);
    }
finish:
    if(legacy)RegCloseKey(legacy);if(key)RegCloseKey(key);
    CloseServiceHandle(scm);rec_unlock(lock);
    line(L"Running services are not forcibly stopped. Restart may be needed. No additional services modified.");
}
