/* Compile on Windows with clang, or POSIX clang -fshort-wchar. No live event log changes. */
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
typedef uint32_t DWORD; typedef uint16_t WORD; typedef unsigned char BYTE;
typedef uint64_t ULONGLONG; typedef unsigned UINT; typedef wchar_t WCHAR;
typedef const WCHAR* LPCWSTR; typedef void* HANDLE;
typedef struct {DWORD dwLowDateTime,dwHighDateTime;} FILETIME;
typedef struct { DWORD Length,Reserved,RecordNumber,TimeGenerated,TimeWritten,EventID;WORD EventType,NumStrings,EventCategory,ReservedFlags;DWORD ClosingRecordNumber,StringOffset,UserSidLength,UserSidOffset,DataLength,DataOffset; } EVENTLOGRECORD;
#define EVENTLOG_SEQUENTIAL_READ 1
#define EVENTLOG_BACKWARDS_READ 8
#define EVENTLOG_ERROR_TYPE 1
static int g_sys_err,g_app_err,g_unexpected_power,g_bsod,g_storage_faults,g_whea,g_crashes,g_hangs,g_clr_events;
static ULONGLONG g_current_event_epoch;
static int scenario,reads,closes;static DWORD last_error;static const DWORD now=1800000000U;
static unsigned upper(unsigned c){return c>='a'&&c<='z'?c-32:c;}
static int same_ci(LPCWSTR a,LPCWSTR b){while(*a&&*b){if(upper(*a++)!=upper(*b++))return 0;}return *a==*b;}
static int contains_ci(LPCWSTR a,LPCWSTR b){for(;*a;a++){unsigned i=0;while(b[i]&&a[i]&&upper(a[i])==upper(b[i]))i++;if(!b[i])return 1;}return !*b;}
static ULONGLONG ft64(FILETIME f){return ((ULONGLONG)f.dwHighDateTime<<32)|f.dwLowDateTime;}
static void GetSystemTimeAsFileTime(FILETIME*f){ULONGLONG n=(11644473600ULL+now)*10000000ULL;f->dwHighDateTime=(DWORD)(n>>32);f->dwLowDateTime=(DWORD)n;}
static DWORD GetLastError(void){return last_error;}
static HANDLE OpenEventLogW(LPCWSTR server,LPCWSTR name){(void)server;(void)name;if(scenario==1){last_error=5;return 0;}return (HANDLE)1;}
static int CloseEventLog(HANDLE h){assert(h);closes++;return 1;}
static void crash_insertions(const EVENTLOGRECORD*r){(void)r;}
static DWORD record(BYTE*b,LPCWSTR provider,DWORD id,DWORD time){
    EVENTLOGRECORD r={0};unsigned n=0;while(provider[n])n++;
    DWORD len=(DWORD)((sizeof(r)+(n+2)*sizeof(WCHAR)+4+3)&~3U);
    memset(b,0,len);r.Length=len;r.EventID=id;r.TimeGenerated=time;r.EventType=1;
    memcpy(b,&r,sizeof(r));memcpy(b+sizeof(r),provider,(n+1)*sizeof(WCHAR));memcpy(b+len-4,&len,4);return len;
}
static int ReadEventLogW(HANDLE h,DWORD flags,DWORD offset,void*dst,DWORD cap,DWORD*read,DWORD*needed){
    (void)h;(void)flags;(void)offset;(void)needed;assert(cap>=4096);BYTE*b=dst;reads++;
    if(scenario==2||(scenario==3&&reads>1)){last_error=5;return 0;}
    if(scenario==0||((scenario==3||scenario==4||scenario==5||scenario==7)&&reads>1)){last_error=38;return 0;}
    if(scenario==6){*read=4;return 1;}
    if(scenario==8){*read=record(b,L"Disk",7,now-8*86400);return 1;}
    if(scenario==9){*read=0;return 1;}
    if(scenario==10){*read=record(b,L"Disk",7,now);b[*read-4]^=1;return 1;}
    if(scenario==4){*read=record(b,L"Microsoft-Windows-Kernel-Power",41,now);*read+=record(b+*read,L"Microsoft-Windows-WER-SystemErrorReporting",1001,now);*read+=record(b+*read,L"Unrelated",1001,now);return 1;}
    if(scenario==7){*read=record(b,L".NET Runtime",1026,now);return 1;}
    if(scenario==11){*read=0;for(int i=0;i<100;i++)*read+=record(b+*read,L"Disk",7,now);assert(*read<=cap);return 1;}
    *read=record(b,L"Disk",7,now);return 1;
}
#include "../src/event_scan.inc"
static LOG_SCAN run(int mode,LPCWSTR log){scenario=mode;reads=closes=0;return scan_events(log);}
int main(void){
    assert(sizeof(WCHAR)==2);LOG_SCAN s;
    s=run(0,L"System");assert(s.status==LOG_COMPLETE&&s.count==0&&closes==1);
    s=run(1,L"System");assert(s.status==LOG_OPEN_FAILED&&s.error==5&&closes==0);
    s=run(2,L"Application");assert(s.status==LOG_READ_FAILED&&s.error==5&&closes==1);
    s=run(3,L"System");assert(s.status==LOG_READ_FAILED&&s.count==1&&closes==1);
    s=run(4,L"System");assert(s.status==LOG_COMPLETE&&s.count==3&&g_unexpected_power==1&&g_bsod==1);
    s=run(5,L"System");assert(s.status==LOG_COMPLETE&&s.count==1);
    s=run(6,L"System");assert(s.status==LOG_MALFORMED&&closes==1);
    s=run(7,L"Application");assert(s.status==LOG_COMPLETE&&g_clr_events==1);
    s=run(8,L"System");assert(s.status==LOG_COMPLETE&&s.count==0);
    s=run(9,L"System");assert(s.status==LOG_MALFORMED);
    s=run(10,L"System");assert(s.status==LOG_MALFORMED);
    s=run(11,L"System");assert(s.status==LOG_LIMIT&&s.count==3500&&closes==1);
    s=run(12,L"System");assert(s.status==LOG_LIMIT&&s.count==60&&reads==60);
    puts("PASS: 13 mocked scan scenarios");return 0;
}
