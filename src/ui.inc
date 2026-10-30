static void layout(int w,int h){
    /* Phone/tablet conversions: two spacious columns down to 520px client width.
       Keep each label legible at 100-150% UI scaling and let Windows paint the text. */
    if(w<460)w=460;
    if(h<350)h=350;
    MoveWindow(g_heading,14,10,w-28,26,TRUE);
    MoveWindow(g_status,14,36,w-28,28,TRUE);
    int top=70;
    int gap=8;
    int cols=(w>=520)?2:1;
    int rows=(8+cols-1)/cols;
    int bw=(w-28-(cols-1)*gap)/cols;
    int bh=(w>=520)?35:32;
    for(int i=0;i<8;i++){
        int r=i/cols,c=i%cols;
        MoveWindow(g_buttons[i],14+c*(bw+gap),top+r*(bh+gap),bw,bh,TRUE);
    }
    int etop=top+rows*(bh+gap)+3;
    int eh=h-etop-14;
    if(eh<35)eh=35;
    MoveWindow(g_editor,14,etop,w-28,eh,TRUE);
}
static LRESULT WINAPI window_proc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam){if(msg==WM_CREATE){g_heading=CreateWindowExW(0,L"STATIC",L"BLAZE ULTIMATE OPTIMIZER  |  BLAZE SYSTEMS  |  v1.3",WS_CHILD|WS_VISIBLE,15,12,700,22,hwnd,0,g_inst,0);g_status=CreateWindowExW(0,L"STATIC",L"Ready. Start with DIAGNOSE (read-only). No changes without permission.",WS_CHILD|WS_VISIBLE,15,38,700,24,hwnd,0,g_inst,0);
LPCWSTR labels[8]={L"1-CLICK DIAGNOSE",L"DIAGNOSE + SAFE FIX",L"OLD TEMP CLEANUP",L"RESTORE PREFERENCES",L"OPEN REPORT FILE",L"STARTUP APPS",L"ADVANCED RISK MODE",L"RESTORE SERVICES"};for(int i=0;i<8;i++){g_buttons[i]=CreateWindowExW(0,L"BUTTON",labels[i],WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,0,0,150,32,hwnd,(HMENU)(ULONG_PTR)(ID_DIAGNOSE+i),g_inst,0);SendMessageW(g_buttons[i],WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);}g_editor=CreateWindowExW(0,L"EDIT",L"BLAZE SYSTEMS | Loumer Cabaluna Manreal\r\nUltra-low-spec Windows diagnostics + opt-in controls\r\n\r\nPress 1-CLICK DIAGNOSE for a read-only assessment, or DIAGNOSE + SAFE FIX to also apply reversible UI tweaks and remove >7-day-old top-level user-temp files.\r\n\r\nNothing is sent online. Reports are kept on this PC.\r\n\r\nNo optimizer can make 1 GB behave exactly like 8 GB; this app aims to reduce avoidable load and pinpoint actionable problems.",WS_CHILD|WS_VISIBLE|WS_BORDER|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|ES_NOHIDESEL,15,147,690,360,hwnd,0,g_inst,0);SendMessageW(g_editor,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);SendMessageW(g_status,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);SendMessageW(g_heading,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);return 0;}
if(msg==WM_SIZE){layout((int)(short)(lparam&0xFFFF),(int)(short)((lparam>>16)&0xFFFF));return 0;}
if(msg==WM_COMMAND){int id=(int)(wparam&0xFFFF);if(g_busy)return 0;if(id==ID_DIAGNOSE)start_worker(0);else if(id==ID_FIX){if(MessageBoxW(hwnd,L"Run diagnosis, disable optional UI animation/transparency, and delete ONLY top-level files in your personal TEMP directory that have not changed in over 7 days?\r\n\r\nRegistry visual preferences will be backed up. TEMP deletion is irreversible. Core services, Defender, and pagefile are NOT touched.",L"Confirm safe optimization",MB_YESNO|MB_ICONWARNING)==IDYES)start_worker(1);}else if(id==ID_CLEAN){if(MessageBoxW(hwnd,L"Delete only top-level files older than 7 days from your personal LOCALAPPDATA\\Temp folder? This deletion cannot be undone.",L"Confirm cleanup",MB_YESNO|MB_ICONWARNING)==IDYES)start_worker(2);}else if(id==ID_REPORT){if(g_reportPath[0])ShellExecuteW(hwnd,L"open",g_reportPath,0,0,SW_SHOWNORMAL);else MessageBoxW(hwnd,L"Run a scan first; your local report will be saved automatically.",L"Report",MB_OK);}else if(id==ID_STARTUP)ShellExecuteW(hwnd,L"open",L"ms-settings:startupapps",0,0,SW_SHOWNORMAL);else if(id==ID_RESTORE)visual_restore();else if(id==ID_ADVANCED)risk_start();else if(id==ID_ROLLBACK){if(MessageBoxW(hwnd,L"Restore recorded startup types for WSearch and SysMain? Requires administrator rights. Changes may need a restart.",L"Restore advanced settings",MB_YESNO|MB_ICONWARNING)==IDYES)start_worker(4);}return 0;}
if(msg==WM_APP+1){g_busy=0;for(int i=0;i<8;i++)EnableWindow(g_buttons[i],TRUE);SetWindowTextW(g_editor,g_report);SetWindowTextW(g_status,L"Analysis complete. A UTF-16 report was saved locally; OPEN REPORT FILE to view/copy it.");return 0;}
if(msg==WM_CTLCOLOREDIT){SetBkMode((HANDLE)wparam,1);SetTextColor((HANDLE)wparam,0x00FFFFFF);return (LRESULT)g_panelBrush;}
if(msg==WM_CTLCOLORSTATIC){SetBkMode((HANDLE)wparam,TRANSPARENT);SetTextColor((HANDLE)wparam,0x00FFFFFF);return (LRESULT)((HWND)lparam==g_editor?g_panelBrush:g_darkBrush);}
if(msg==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProcW(hwnd,msg,wparam,lparam);}
void mainCRTStartup(void){g_inst=GetModuleHandleW(NULL);g_darkBrush=CreateSolidBrush(0x00200E05);g_panelBrush=CreateSolidBrush(0x002D180C);WNDCLASSEXW cls;/* no C runtime: initialize field by field */
cls.cbSize=sizeof(cls);cls.style=0;cls.lpfnWndProc=window_proc;cls.cbClsExtra=0;cls.cbWndExtra=0;cls.hInstance=g_inst;cls.hIcon=LoadIconW(g_inst,(LPCWSTR)(ULONG_PTR)ICON_RESOURCE);cls.hCursor=LoadCursorW(0,IDC_ARROW);cls.hbrBackground=g_darkBrush;cls.lpszMenuName=0;cls.lpszClassName=L"BlazeUltimateOptimizerWindowV1";cls.hIconSm=cls.hIcon;
if(!RegisterClassExW(&cls)){MessageBoxW(0,L"Failed to initialize window class.",L"Blaze Ultimate Optimizer",MB_OK);ExitProcess(2);}g_main=CreateWindowExW(0,cls.lpszClassName,L"Blaze Ultimate Optimizer 1.3 Preview  |  Blaze Systems",WS_OVERLAPPEDWINDOW|WS_VISIBLE,80,60,835,630,0,0,g_inst,0);if(!g_main){MessageBoxW(0,L"Failed to create main window.",L"Blaze Ultimate Optimizer",MB_OK);ExitProcess(3);}ShowWindow(g_main,SW_SHOWNORMAL);UpdateWindow(g_main);MSG m;while(GetMessageW(&m,0,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}ExitProcess(0);}