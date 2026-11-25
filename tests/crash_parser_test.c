/* Synthetic classic Event Log fixtures; no actual Windows state changes. */
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
typedef uint32_t DWORD;typedef uint16_t WORD;typedef unsigned char BYTE;
typedef unsigned UINT;typedef wchar_t WCHAR;typedef const WCHAR*LPCWSTR;
typedef struct {DWORD Length,Reserved,RecordNumber,TimeGenerated,TimeWritten,EventID;WORD EventType,NumStrings,EventCategory,ReservedFlags;DWORD ClosingRecordNumber,StringOffset,UserSidLength,UserSidOffset,DataLength,DataOffset;} EVENTLOGRECORD;
typedef struct {WCHAR name[120],module[120],exception[32];unsigned count;DWORD newest;} FAULT;
static FAULT g_faults[12];
static int g_app_names_count;
static unsigned g_crash_grouped,g_crash_rejected,g_crash_ungrouped;
static unsigned upper(unsigned c){return c>='a'&&c<='z'?c-32:c;}
static int same_ci(LPCWSTR a,LPCWSTR b){while(*a&&*b){if(upper(*a++)!=upper(*b++))return 0;}return *a==*b;}
static void wcopy(WCHAR*d,LPCWSTR s,UINT cap){UINT i=0;while(s[i]&&i+1<cap){d[i]=s[i];i++;}d[i]=0;}
#include "../src/crash_parser.inc"
static union {uint64_t align;BYTE bytes[4096];} buf;
static DWORD put(DWORD off,LPCWSTR s){do{WCHAR c=*s++;memcpy(buf.bytes+off,&c,2);off+=2;if(!c)break;}while(1);return off;}
static EVENTLOGRECORD* fixture(LPCWSTR app,LPCWSTR module,LPCWSTR ex){
    memset(&buf,0,sizeof(buf));EVENTLOGRECORD*r=(EVENTLOGRECORD*)buf.bytes;
    DWORD off=sizeof(*r);off=put(off,L"Application Error");off=put(off,L"PC-PRIVATE");
    r->StringOffset=off;r->NumStrings=11;r->TimeGenerated=100;
    const WCHAR*fields[]={app,L"1",L"2",module,L"3",L"4",ex,L"5",L"6",L"7",L"C:\\Users\\Private\\hidden.exe"};
    for(unsigned i=0;i<11;i++)off=put(off,fields[i]);
    off=(off+3)&~3U;r->Length=off+4;memcpy(buf.bytes+off,&r->Length,4);return r;
}
static void clear(void){g_app_names_count=0;g_crash_grouped=g_crash_rejected=g_crash_ungrouped=0;}
int main(void){
    assert(sizeof(WCHAR)==2);EVENTLOGRECORD*r;
    clear();r=fixture(L"C:\\Users\\Private\\app.exe",L"D:/secret/mod.dll",L"0xc0000005");crash_insertions(r);
    assert(g_crash_grouped==1&&g_app_names_count==1&&same_ci(g_faults[0].name,L"app.exe")&&same_ci(g_faults[0].module,L"mod.dll")&&same_ci(g_faults[0].exception,L"C0000005"));
    r=fixture(L"APP.EXE",L"MOD.DLL",L"C0000005");r->TimeGenerated=120;crash_insertions(r);assert(g_app_names_count==1&&g_faults[0].count==2&&g_faults[0].newest==120);
    r=fixture(L"app.exe",L"mod.dll",L"c0000409");crash_insertions(r);assert(g_app_names_count==2);
    r=fixture(L"app.exe",L"mod.dll",L"bad");crash_insertions(r);assert(g_crash_rejected==1);
    r=fixture(L"app.exe",L"mod.dll",L"C0000005");r->NumStrings=6;crash_insertions(r);assert(g_crash_rejected==2);
    r=fixture(L"app.exe",L"mod.dll",L"C0000005");r->StringOffset++;crash_insertions(r);assert(g_crash_rejected==3);
    r=fixture(L"app.exe",L"mod.dll",L"C0000005");r->StringOffset=sizeof(*r);crash_insertions(r);assert(g_crash_rejected==4);
    r=fixture(L"app.exe",L"mod.dll",L"C0000005");r->DataOffset=r->StringOffset+4;r->DataLength=2;crash_insertions(r);assert(g_crash_rejected==5);
    r=fixture(L"app.exe",L"mod.dll",L"C0000005");r->UserSidOffset=r->StringOffset;r->UserSidLength=8;crash_insertions(r);assert(g_crash_rejected==6);
    r=fixture(L"app.exe",L"mod.dll",L"C0000005");r->NumStrings=65535;crash_insertions(r);assert(g_crash_rejected==7);
    r=fixture(L"app\r\nfake.exe",L"mod.dll",L"C0000005");crash_insertions(r);assert(g_crash_rejected==8);
    WCHAR longname[130];for(int i=0;i<129;i++)longname[i]=L'a';longname[129]=0;
    r=fixture(longname,L"mod.dll",L"C0000005");crash_insertions(r);assert(g_crash_rejected==9);
    r=fixture(L"app.exe",L"",L"C0000005");crash_insertions(r);assert(g_crash_rejected==10);
    clear();WCHAR app[]=L"a.exe";
    for(int i=0;i<13;i++){app[0]=(WCHAR)(L'a'+i);r=fixture(app,L"mod.dll",L"C0000005");crash_insertions(r);}
    assert(g_app_names_count==12&&g_crash_grouped==12&&g_crash_ungrouped==1);
    app[0]=L'a';r=fixture(app,L"mod.dll",L"C0000005");crash_insertions(r);assert(g_crash_grouped==13&&g_faults[0].count==2);
    puts("PASS: 15 crash parser and privacy scenarios");return 0;
}
