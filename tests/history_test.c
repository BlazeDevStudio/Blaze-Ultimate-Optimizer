#include <assert.h>
#include <stdint.h>
#include <stdio.h>
typedef uint64_t ULONGLONG;
#include "../src/history_ring.inc"
static HISTORY_POINT point(ULONGLONG tick,unsigned cpu,unsigned ram,unsigned flags,unsigned phase){
    HISTORY_POINT p={0};p.tick=tick;p.cpu=cpu;p.ram=ram;p.valid=flags;p.phase=phase;return p;
}
int main(void){
    HISTORY_RING r={0};HISTORY_STATS s=history_stats(&r,HIST_CPU,85);
    assert(!s.count&&!history_at(&r,0)&&!history_repeat_high(&r,HIST_CPU,85));
    history_push(&r,point(0,0,100,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));
    history_push(&r,point(1000,101,40,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));
    s=history_stats(&r,HIST_CPU,85);assert(s.count==1&&s.average==0&&s.peak==0&&!s.hot);
    s=history_stats(&r,HIST_RAM,83);assert(s.count==2&&s.average==70&&s.peak==100&&s.hot==1);
    assert(!history_stats(&r,HIST_NET_RX,85).count);
    for(unsigned i=2;i<132;i++)history_push(&r,point(i*1000,90,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));
    assert(r.count==120&&history_at(&r,0)->tick==12000&&history_at(&r,119)->tick==131000&&!history_at(&r,120));
    s=history_stats(&r,HIST_CPU,85);assert(s.count==120&&s.average==90&&s.hot==120);
    assert(history_repeat_high(&r,HIST_CPU,85));
    history_push(&r,point(132000,90,90,HIST_RAM,HIST_DIAGNOSE));assert(!history_repeat_high(&r,HIST_CPU,85)&&history_repeat_high(&r,HIST_RAM,83));
    history_push(&r,point(133000,90,90,HIST_CPU|HIST_RAM,HIST_PRE_ACTION));assert(!history_repeat_high(&r,HIST_RAM,83));
    history_push(&r,point(1,90,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));assert(r.count==1&&history_at(&r,0)->tick==1);
    history_push(&r,point(10001,90,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));
    history_push(&r,point(20001,90,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));assert(!history_repeat_high(&r,HIST_CPU,85));
    history_push(&r,point(30001,90,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));assert(history_repeat_high(&r,HIST_CPU,85));
    history_push(&r,point(30002,10,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));assert(!history_repeat_high(&r,HIST_CPU,85));
    history_push(&r,point(1900000,90,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));assert(!history_repeat_high(&r,HIST_RAM,83));
    HISTORY_RING d={0};for(unsigned i=0;i<3;i++)history_push(&d,point(100,90,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));
    assert(!history_repeat_high(&d,HIST_RAM,83));
    history_push(&d,point(30100,90,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));assert(!history_repeat_high(&d,HIST_RAM,83));
    history_push(&d,point(60100,90,90,HIST_CPU|HIST_RAM,HIST_DIAGNOSE));assert(history_repeat_high(&d,HIST_RAM,83));
    history_push(&d,point(60101,90,90,HIST_CPU|HIST_RAM,HIST_POST_FIX));assert(!history_repeat_high(&d,HIST_RAM,83));
    assert(sizeof(HISTORY_RING)<8192);
    puts("PASS: 17 session history boundary/behavior groups");return 0;
}
