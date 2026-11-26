#include <assert.h>
#include <stdint.h>
#include <stdio.h>
typedef uint64_t ULONGLONG;
#include "../src/health_math.inc"
int main(void){
    CPU_DELTA d;ULONGLONG n;
    assert(health_cpu_delta((CPU_COUNTERS){10,20,30},(CPU_COUNTERS){20,50,50},&d)&&d.total==50&&d.busy==40&&health_percent(d.busy,d.total)==80);
    assert(!health_cpu_delta((CPU_COUNTERS){10,20,30},(CPU_COUNTERS){9,50,50},&d));
    assert(!health_cpu_delta((CPU_COUNTERS){10,20,30},(CPU_COUNTERS){11,19,50},&d));
    assert(!health_cpu_delta((CPU_COUNTERS){10,20,30},(CPU_COUNTERS){11,50,29},&d));
    assert(!health_cpu_delta((CPU_COUNTERS){10,20,30},(CPU_COUNTERS){30,30,50},&d));
    assert(!health_cpu_delta((CPU_COUNTERS){10,20,30},(CPU_COUNTERS){10,20,30},&d));
    assert(!health_cpu_delta((CPU_COUNTERS){0,0,0},(CPU_COUNTERS){0,UINT64_MAX,1},&d));
    assert(health_cpu_delta((CPU_COUNTERS){0,0,0},(CPU_COUNTERS){100,100,0},&d)&&d.busy==0);
    assert(health_cpu_delta((CPU_COUNTERS){0,0,0},(CPU_COUNTERS){0,100,0},&d)&&health_percent(d.busy,d.total)==100);
    assert(health_percent(UINT64_MAX-1,UINT64_MAX)==99&&health_percent(UINT64_MAX,UINT64_MAX)==100);
    assert(health_percent(1,3)==33&&health_percent(2,3)==66&&health_percent(1,0)==0);
    assert(!health_add(UINT64_MAX,1,&n)&&health_add(UINT64_MAX-1,1,&n)&&n==UINT64_MAX);
    HEALTH_RANGE r={0};health_range_add(&r,0);health_range_add(&r,10);assert(r.low==0&&r.high==10&&r.count==2);
    HEALTH_RANGE s={0};health_range_add(&s,10);health_range_add(&s,0);assert(s.low==0&&s.high==10&&s.count==2);
    assert(health_memory_valid(1024,0,100)&&!health_memory_valid(0,0,0)&&!health_memory_valid(100,101,10)&&!health_memory_valid(100,10,101));
    for(unsigned total=1;total<=300;total++)for(unsigned part=0;part<=total;part++)assert(health_percent(part,total)==100*part/total);
    puts("PASS: 15 health boundary groups and 45450 small-ratio checks");return 0;
}
