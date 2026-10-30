static void save_report(void){WCHAR base[512];DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",base,512);if(!n||n>=512){n=GetTempPathW(512,base);if(!n||n>=512)return;}else wcat(base,L"\\",512);wcopy(g_reportPath,base,512);wcat(g_reportPath,L"BlazeUltimateOptimizer-Report.txt",512);HANDLE h=CreateFileW(g_reportPath,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);if(h==INVALID_HANDLE_VALUE)return;DWORD wrote;WORD bom=0xFEFF;WriteFile(h,&bom,2,&wrote,0);WriteFile(h,g_report,g_pos*2,&wrote,0);CloseHandle(h);}
static DWORD WINAPI worker(LPVOID ignored){
    (void)ignored;
    collect();
    if(g_mode==1){
        ULONGLONG beforeDisk=g_free;
        unsigned beforeMem=g_memload;
        /* Safe actions are performed only after explicit confirmation on the UI thread. */
        visual_optimize();
        clean_user_temp();
        ULONGLONG deletedBytes=g_freed;
        unsigned deletedFiles=g_deleted;
        int optApplied=g_opt_ok;
        collect(); /* Refresh the verdict based on the actual post-fix condition. */
        heading(L"ONE-CLICK FIX RECEIPT AND RECHECK");
        append(L"Visual effects/transparency update: ");line(optApplied?L"attempted and at least one setting applied; reversible via RESTORE PREFERENCES":L"not applied; consult Windows display settings");
        append(L"Old personal temp files deleted: ");num(deletedFiles);append(L" (approx ");fmt_size(deletedBytes);line(L" apparent file size)");
        append(L"Free disk: before ");fmt_size(beforeDisk);append(L", after ");fmt_size(g_free);nl();
        append(L"Memory load: before ");num(beforeMem);append(L"%, after ");num(g_memload);line(L"% (naturally fluctuates; not a RAM boost)");
if(g_memload+4<=beforeMem)line(L"OUTCOME: lower observed RAM load, but this snapshot cannot attribute the change to the repair.");
else line(L"OUTCOME: NO MEASURABLE RAM RELIEF from safe fix. Focus on unused application sessions or the specific crash clusters above.");
if(g_crashes)line(L"UNRESOLVED: historical application crashes remain. SAFE FIX does not repair printer/audio drivers or third-party apps.");
        line(L"Deleted temp files cannot be restored. No system services, pagefile, or security settings were modified.");
    }
    if(g_mode==2){heading(L"TEMP CLEANUP ACTION");clean_user_temp();line(L"Rerun DIAGNOSE to refresh the verdict.");}
    if(g_mode==3)risky_services(0);
    if(g_mode==4)risky_services(1);
    save_report();
    PostMessageW(g_main,WM_APP+1,0,0);
    return 0;
}
static void start_worker(int mode){if(g_busy)return;g_mode=mode;g_busy=1;for(int i=0;i<8;i++)EnableWindow(g_buttons[i],FALSE);SetWindowTextW(g_status,mode==0?L"Scanning resource pressure, processes, startup items and 7-day event logs...":L"Scanning and running safe low-spec optimizations...");DWORD tid=0;HANDLE t=CreateThread(0,0,worker,0,0,&tid);if(t)CloseHandle(t);else{g_busy=0;for(int i=0;i<8;i++)EnableWindow(g_buttons[i],TRUE);SetWindowTextW(g_status,L"Could not create diagnostic worker thread.");}}
static void risk_start(void){
   if(g_busy)return;
   if(MessageBoxW(g_main,L"ADVANCED RISK MODE - HIGH RISK\r\n\r\nOnly optional Windows Search (WSearch) and SysMain startup configurations are eligible. Disabling them can break search, indexing, prefetch functionality or worsen performance. This does not improve every PC. Administrator rights required.\r\n\r\nYou can restore the saved startup types with RESTORE SERVICES. A failed write or external registry cleanup can still prevent rollback.\r\n\r\nContinue to the mandatory 3-second warning countdown?",L"Blaze Systems - Advanced Risk Mode",MB_YESNO|MB_ICONWARNING)!=IDYES)return;
   const WCHAR*steps[3]={L"HIGH-RISK WARNING - 3 seconds until final confirmation...",L"HIGH-RISK WARNING - 2 seconds until final confirmation...",L"HIGH-RISK WARNING - 1 second until final confirmation..."};
   for(int i=0;i<3;i++){SetWindowTextW(g_status,steps[i]);UpdateWindow(g_status);Sleep(1000);}
   if(MessageBoxW(g_main,L"3-second warning completed. Last confirmation: disable startup for WSearch and SysMain only? This may make search or loading slower. Save your work and ensure you can restore the services. No essential security/boot services will be disabled.",L"CONFIRM - High-risk change",MB_YESNO|MB_ICONWARNING)==IDYES)start_worker(3);
   else SetWindowTextW(g_status,L"Advanced changes canceled; nothing altered.");
}
