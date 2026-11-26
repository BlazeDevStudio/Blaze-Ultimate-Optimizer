/* Pure integer helpers shared by native collector and synthetic tests. */
#define HEALTH_MAX_U64 (~(ULONGLONG)0)
typedef struct {ULONGLONG idle,kernel,user;} CPU_COUNTERS;
typedef struct {ULONGLONG total,busy;} CPU_DELTA;
static int health_add(ULONGLONG a,ULONGLONG b,ULONGLONG*out){
    if(b>HEALTH_MAX_U64-a)return 0;
    *out=a+b;return 1;
}
static unsigned health_percent(ULONGLONG part,ULONGLONG total){
    if(!total||part>total)return 0;
    /* Exact floor(100*part/total), without multiplying full-width counters. */
    unsigned lo=0,hi=100;
    while(lo<hi){
        unsigned mid=(lo+hi+1)/2;
        ULONGLONG threshold=(total/100)*mid+((total%100)*mid+99)/100;
        if(part>=threshold)lo=mid;else hi=mid-1;
    }
    return lo;
}
static int health_cpu_delta(CPU_COUNTERS a,CPU_COUNTERS b,CPU_DELTA*out){
    if(b.idle<a.idle||b.kernel<a.kernel||b.user<a.user)return 0;
    ULONGLONG idle=b.idle-a.idle,kernel=b.kernel-a.kernel,user=b.user-a.user,total;
    if(idle>kernel||!health_add(kernel,user,&total)||!total)return 0;
    out->total=total;out->busy=total-idle;return 1;
}
typedef struct {ULONGLONG low,high;unsigned count;} HEALTH_RANGE;
static void health_range_add(HEALTH_RANGE*r,ULONGLONG value){
    if(!r->count||value<r->low)r->low=value;
    if(!r->count||value>r->high)r->high=value;
    r->count++;
}
static int health_memory_valid(ULONGLONG total,ULONGLONG available,unsigned load){
    return total>0&&available<=total&&load<=100;
}
