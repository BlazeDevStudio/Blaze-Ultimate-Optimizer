#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t DWORD;typedef int32_t LONG;typedef uint64_t ULONGLONG,ULONG_PTR;
typedef unsigned char BYTE;typedef wchar_t WCHAR;typedef const WCHAR*LPCWSTR;typedef WCHAR*LPWSTR;typedef void*HANDLE;
typedef struct {DWORD CStatus;union {LONG longValue;long long largeValue;double doubleValue;} value;} BLAZE_PDH_VALUE;
typedef struct {LPWSTR name;BLAZE_PDH_VALUE value;} BLAZE_PDH_ITEM;
typedef struct {DWORD length,type,version,status;LONG scale,defaultScale;ULONG_PTR userData,queryData;LPWSTR fullPath;} BLAZE_PDH_INFO_PREFIX;
static int health_add(ULONGLONG a,ULONGLONG b,ULONGLONG*out){if(b>UINT64_MAX-a)return 0;*out=a+b;return 1;}
static int scenario,opens,adds,localized,removes,collects,arrays,closes;
static LONG PdhOpenQueryW(LPCWSTR path,ULONG_PTR u,HANDLE*q){assert(!path&&!u);opens++;if(scenario==1)return 5;*q=(HANDLE)(uintptr_t)1;return 0;}
static LONG PdhAddEnglishCounterW(HANDLE q,LPCWSTR p,ULONG_PTR u,HANDLE*h){assert(q&&p&&!u);adds++;*h=(HANDLE)(uintptr_t)(adds+1);return 0;}
static LONG PdhGetCounterInfoW(HANDLE h,int explain,DWORD*b,void*dst){assert(h&&!explain);if(scenario==2)return 5;BLAZE_PDH_INFO_PREFIX*p=dst;WCHAR*n=(WCHAR*)((BYTE*)dst+sizeof(*p));static const WCHAR path[]=L"\\Localized(*)\\Counter";memcpy(n,path,sizeof(path));*b=sizeof(*p)+sizeof(path);p->fullPath=scenario==3?(LPWSTR)(uintptr_t)2:n;return 0;}
static LONG PdhAddCounterW(HANDLE q,LPCWSTR p,ULONG_PTR u,HANDLE*h){assert(q&&p[0]==L'\\'&&!u);localized++;*h=(HANDLE)(uintptr_t)(100+localized);return 0;}
static LONG PdhRemoveCounter(HANDLE h){assert(h);removes++;return 0;}
static LONG PdhSetCounterScaleFactor(HANDLE h,LONG scale){assert(h&&scale==0);return scenario==4?5:0;}
static LONG PdhCollectQueryData(HANDLE q){assert(q);collects++;return (scenario==5&&collects==1)||(scenario==6&&collects==2)?5:0;}
static LONG PdhCloseQuery(HANDLE q){assert(q);closes++;return 0;}
static LONG PdhGetFormattedCounterArrayW(HANDLE h,DWORD fmt,DWORD*b,DWORD*c,BLAZE_PDH_ITEM*p){
    assert(h&&fmt==0x8400);arrays++;
    if(scenario==7)return 5;
    if(scenario==8){*c=UINT32_MAX;*b=24;return 0;}
    if(scenario==9){*c=0;*b=0;return 0;}
    *c=3;WCHAR*n=(WCHAR*)(p+3);
    static const WCHAR a[]=L"instance1",d[]=L"instance2",t[]=L"_Total";
    memcpy(n,a,sizeof(a));p[0].name=n;n+=sizeof(a)/sizeof(WCHAR);
    memcpy(n,d,sizeof(d));p[1].name=n;n+=sizeof(d)/sizeof(WCHAR);
    memcpy(n,t,sizeof(t));p[2].name=n;n+=sizeof(t)/sizeof(WCHAR);
    *b=(DWORD)((BYTE*)n-(BYTE*)p);
    for(int i=0;i<3;i++){p[i].value.CStatus=1;p[i].value.value.largeValue=i==0?10:i==1?20:100;}
    if(scenario==10)p[1].value.CStatus=2;
    if(scenario==11)p[1].name=(LPWSTR)(uintptr_t)2;
    if(scenario==12)p[1].value.value.largeValue=-1;
    if(scenario==13)p[1].value.value.largeValue=101;
    if(scenario==14){p[0].value.value.largeValue=INT64_MAX;p[1].value.value.largeValue=INT64_MAX;p[2].name=p[1].name;p[2].value.value.largeValue=100;}
    return 0;
}
#include "../src/adapter_metrics.inc"
static ADAPTER_CAPTURE run(int mode){ADAPTER_CAPTURE c;scenario=mode;opens=adds=localized=removes=collects=arrays=closes=0;adapter_capture_begin(&c);adapter_capture_end(&c);return c;}
int main(void){
    assert(sizeof(WCHAR)==2);ADAPTER_CAPTURE c=run(0);assert(opens==1&&adds==5&&localized==5&&removes==5&&collects==2&&arrays==5&&closes==1);
    assert(c.m[NET_RX].total==30&&c.m[NET_RX].valid==2&&c.m[NET_RX].skipped==1&&c.m[GPU_ENGINE].peak==20);
    c=run(1);assert(!closes&&!arrays);
    c=run(2);assert(removes==5&&!localized&&!collects&&closes==1);
    c=run(3);assert(!localized&&removes==5);
    c=run(4);assert(removes==10&&!collects);
    c=run(5);assert(collects==1&&!arrays&&closes==1);
    c=run(6);assert(collects==2&&!arrays&&closes==1);
    c=run(7);assert(!c.m[NET_RX].available);
    c=run(8);assert(!c.m[NET_RX].available);
    c=run(9);assert(!c.m[NET_RX].available);
    c=run(10);assert(c.m[NET_RX].valid==1&&c.m[NET_RX].invalid==1);
    c=run(11);assert(c.m[NET_RX].invalid==1);
    c=run(12);assert(c.m[NET_RX].invalid==1);
    c=run(13);assert(c.m[GPU_ENGINE].invalid==1&&c.m[GPU_ENGINE].peak==10&&c.m[NET_RX].total==111);
    c=run(14);assert(c.m[NET_RX].overflow&&c.m[GPU_ENGINE].invalid==2);
    puts("PASS: 15 mocked adapter collector scenarios");return 0;
}
