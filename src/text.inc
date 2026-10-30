__attribute__((optnone)) void *memcpy(void *dst,const void *src,SIZE_T n){BYTE*d=(BYTE*)dst;const BYTE*s=(const BYTE*)src;for(SIZE_T i=0;i<n;i++)d[i]=s[i];return dst;}
static UINT wlen(const WCHAR*s){UINT i=0;if(!s)return 0;while(s[i])i++;return i;}
static void wcopy(WCHAR*d,const WCHAR*s,UINT cap){UINT i=0;if(!cap)return;while(s&&s[i]&&i+1<cap){d[i]=s[i];i++;}d[i]=0;}
static void wcat(WCHAR*d,const WCHAR*s,UINT cap){UINT n=wlen(d);if(n<cap)wcopy(d+n,s,cap-n);}
static UINT ascii_upper(UINT c){return (c>=97&&c<=122)?c-32:c;}
static BOOL same_ci(LPCWSTR a,LPCWSTR b){while(*a&&*b){if(ascii_upper(*a)!=ascii_upper(*b))return 0;a++;b++;}return *a==*b;}
static BOOL contains_ci(LPCWSTR a,LPCWSTR b){UINT bl=wlen(b);if(!bl)return 1;for(;*a;a++){UINT i=0;while(i<bl&&a[i]&&ascii_upper(a[i])==ascii_upper(b[i]))i++;if(i==bl)return 1;}return 0;}
static ULONGLONG ft64(FILETIME a){return ((ULONGLONG)a.dwHighDateTime<<32)|a.dwLowDateTime;}
static void append(LPCWSTR s){while(*s&&g_pos+1<sizeof(g_report)/sizeof(g_report[0]))g_report[g_pos++]=*s++;g_report[g_pos]=0;}
static void num(ULONGLONG n){WCHAR a[32];UINT i=0;do{a[i++]=(WCHAR)(L'0'+n%10);n/=10;}while(n&&i<31);while(i&&g_pos+1<sizeof(g_report)/sizeof(g_report[0]))g_report[g_pos++]=a[--i];g_report[g_pos]=0;}
static void nl(void){append(L"\r\n");}
static void line(LPCWSTR s){append(s);nl();}
static void fmt_size(ULONGLONG n){num(n/1048576);append(L" MB");}
static void heading(LPCWSTR s){nl();line(L"------------------------------------------------------------");line(s);line(L"------------------------------------------------------------");}
static void reset(void){g_report[0]=0;g_pos=0;g_events_ok=0;g_run_count=0;g_highmem=0;g_lowdisk=0;g_extremedisk=0;g_storage_faults=0;g_crashes=0;g_hangs=0;g_bsod=0;g_whea=0;g_sys_err=0;g_app_err=0;g_clr_events=0;g_cpu_ok=0;g_cpu=0;g_cpuTopPct=0;g_cpuTopName[0]=0;g_top1[0]=0;for(int i=0;i<8;i++)g_top[i].usage=0;g_group_count=0;g_app_names_count=0;for(int i=0;i<12;i++)g_faults[i].count=0;}
