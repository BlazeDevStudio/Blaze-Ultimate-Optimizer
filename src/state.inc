static HINSTANCE g_inst;
static HWND g_main,g_editor,g_status,g_buttons[8],g_heading;
static HBRUSH g_darkBrush,g_panelBrush;
static volatile int g_busy=0;
static WCHAR g_report[24000];
static UINT g_pos=0;
static WCHAR g_reportPath[512];
static int g_events_ok=0,g_run_count=0,g_highmem=0,g_lowdisk=0,g_extremedisk=0,g_storage_faults=0,g_crashes=0,g_hangs=0,g_bsod=0,g_whea=0,g_sys_err=0,g_app_err=0,g_clr_events=0;
static ULONGLONG g_ram=0,g_free=0,g_disk=0,g_uptime=0;
static unsigned g_memload=0;
static unsigned g_cpu=0,g_cpuTopPct=0;
static WCHAR g_cpuTopName[MAX_PATH];
static int g_cpu_ok=0;
static unsigned g_deleted=0;
static ULONGLONG g_freed=0;
static int g_opt_ok=0;
static int g_mode=0;
static WCHAR g_top1[MAX_PATH];
static int g_app_names_count=0;
static ULONGLONG g_current_event_epoch=0;
typedef struct {WCHAR name[120]; WCHAR module[120]; WCHAR exception[32]; WCHAR appPath[260]; unsigned count; DWORD newest;} FAULT;
static FAULT g_faults[12];
typedef struct {WCHAR name[MAX_PATH];ULONGLONG priv,working;unsigned processes;} GROUP;
static GROUP g_group[64];
static int g_group_count=0;
typedef struct {DWORD dwServiceType,dwStartType,dwErrorControl;WCHAR *lpBinaryPathName,*lpLoadOrderGroup;DWORD dwTagId;WCHAR *lpDependencies,*lpServiceStartName,*lpDisplayName;} QSVC;


typedef struct { WCHAR name[MAX_PATH]; DWORD pid;ULONGLONG usage; } TOP;
static TOP g_top[8];
typedef struct {HANDLE h; DWORD pid; WCHAR name[MAX_PATH]; ULONGLONG ticks;} CPU_ENTRY;
static CPU_ENTRY g_cpu_samples[320];

