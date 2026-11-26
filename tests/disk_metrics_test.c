#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
typedef uint32_t DWORD;typedef int32_t LONG;typedef uint64_t ULONGLONG,ULONG_PTR;
typedef const wchar_t* LPCWSTR;typedef void* HANDLE;
typedef struct {DWORD CStatus;union {LONG longValue;long long largeValue;double doubleValue;} value;} BLAZE_PDH_VALUE;
static int scenario,adds,scales,collects,formats,closes;
static LONG PdhOpenQueryW(LPCWSTR path,ULONG_PTR u,HANDLE*h){assert(!path&&!u);if(scenario==1)return 5;*h=(HANDLE)(uintptr_t)1;return 0;}
static LONG PdhAddEnglishCounterW(HANDLE q,LPCWSTR path,ULONG_PTR u,HANDLE*h){assert(q&&path[0]==L'\\'&&!u);int i=adds++;if(scenario==2&&i==0)return 5;*h=(HANDLE)(uintptr_t)(i+2);return 0;}
static LONG PdhSetCounterScaleFactor(HANDLE h,LONG scale){int i=(int)(uintptr_t)h-2;assert(scale==(i<2?0:3));scales++;return scenario==3&&i==1?5:0;}
static LONG PdhCollectQueryData(HANDLE q){assert(q);collects++;return (scenario==4&&collects==1)||(scenario==5&&collects==2)?5:0;}
static LONG PdhGetFormattedCounterValue(HANDLE h,DWORD fmt,DWORD*type,BLAZE_PDH_VALUE*v){
    int i=(int)(uintptr_t)h-2;assert(fmt==0x8400&&!type&&collects==2);formats++;
    if(scenario==6&&i==0)return 5;
    v->CStatus=(scenario==7&&i==1)?2:1;
    v->value.largeValue=(scenario==8&&i==0)?-1:(i==4?50:i==5?20:1000);
    if(scenario==9&&i==2)v->value.largeValue=0;
    if(scenario==10&&i==4)v->value.largeValue=49;
    return 0;
}
static LONG PdhCloseQuery(HANDLE q){assert(q);closes++;return 0;}
#include "../src/disk_metrics.inc"
static DISK_CAPTURE run(int mode){DISK_CAPTURE c;scenario=mode;adds=scales=collects=formats=closes=0;disk_capture_begin(&c);disk_capture_end(&c);return c;}
int main(void){
    DISK_CAPTURE c=run(0);assert(adds==7&&scales==7&&collects==2&&formats==7&&closes==1&&!c.query&&disk_high_latency(&c));
    c=run(1);assert(!c.query&&c.error==5&&closes==0&&formats==0);
    c=run(2);assert(c.m[0].status==DISK_NOT_ADDED&&formats==6&&closes==1);
    c=run(3);assert(c.m[1].status==DISK_NOT_ADDED&&formats==6&&closes==1);
    c=run(4);assert(collects==1&&formats==0&&c.m[0].status==DISK_FAILED&&closes==1);
    c=run(5);assert(collects==2&&formats==0&&c.m[0].status==DISK_FAILED&&closes==1);
    c=run(6);assert(c.m[0].status==DISK_FAILED&&c.m[1].status==DISK_VALID);
    c=run(7);assert(c.m[1].status==DISK_FAILED);
    c=run(8);assert(c.m[0].status==DISK_FAILED);
    c=run(9);assert(!disk_latency_available(&c,DISK_READ_MS,DISK_READ_MIOPS)&&!disk_high_latency(&c));
    c=run(10);assert(!disk_high_latency(&c));
    puts("PASS: 11 mocked physical disk collection scenarios");return 0;
}
