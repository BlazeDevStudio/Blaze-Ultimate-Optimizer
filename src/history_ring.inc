/* Bounded session history. Only serial diagnostic worker accesses this ring. */
#define HISTORY_CAP 120
enum { HIST_CPU=1,HIST_RAM=2,HIST_DISK_READ=4,HIST_DISK_WRITE=8,HIST_NET_RX=16,HIST_NET_TX=32,HIST_GPU=64 };
enum { HIST_DIAGNOSE=0,HIST_PRE_ACTION=1,HIST_POST_FIX=2 };
typedef struct {
    ULONGLONG tick,readBps,writeBps,rxBps,txBps;
    unsigned valid,cpu,ram,gpu,phase;
} HISTORY_POINT;
typedef struct {HISTORY_POINT p[HISTORY_CAP];unsigned next,count;} HISTORY_RING;
typedef struct {unsigned count,average,peak,hot;} HISTORY_STATS;
static void history_push(HISTORY_RING*r,HISTORY_POINT p){
    p.valid&=127;
    if(p.cpu>100)p.valid&=~HIST_CPU;
    if(p.ram>100)p.valid&=~HIST_RAM;
    if(p.gpu>100)p.valid&=~HIST_GPU;
    if(r->count){
        unsigned last=(r->next+HISTORY_CAP-1)%HISTORY_CAP;
        if(p.tick<r->p[last].tick){r->next=0;r->count=0;}
    }
    r->p[r->next]=p;
    r->next=(r->next+1)%HISTORY_CAP;
    if(r->count<HISTORY_CAP)r->count++;
}
static const HISTORY_POINT* history_at(const HISTORY_RING*r,unsigned i){
    if(i>=r->count)return 0;
    return &r->p[(r->next+HISTORY_CAP-r->count+i)%HISTORY_CAP];
}
static HISTORY_STATS history_stats(const HISTORY_RING*r,unsigned field,unsigned threshold){
    HISTORY_STATS s={0,0,0,0};unsigned sum=0;
    if(field!=HIST_CPU&&field!=HIST_RAM&&field!=HIST_GPU)return s;
    for(unsigned i=0;i<r->count;i++){
        const HISTORY_POINT*p=history_at(r,i);
        if(!(p->valid&field))continue;
        unsigned v=field==HIST_CPU?p->cpu:field==HIST_RAM?p->ram:p->gpu;
        sum+=v;s.count++;if(v>s.peak)s.peak=v;if(v>=threshold)s.hot++;
    }
    if(s.count)s.average=sum/s.count;
    return s;
}
/* A repeat observation, not continuous pressure: require three distinct
 * recent scan windows at least 30s apart overall, within 30 minutes.
 * Action-adjacent scans are excluded; missing or low readings break the run. */
static int history_repeat_high(const HISTORY_RING*r,unsigned field,unsigned threshold){
    if(r->count<3||(field!=HIST_CPU&&field!=HIST_RAM))return 0;
    const HISTORY_POINT*last=history_at(r,r->count-1);
    if(last->phase!=HIST_DIAGNOSE)return 0;
    unsigned n=0;ULONGLONG earliest=last->tick,previous=0;
    for(unsigned j=r->count;j;j--){
        const HISTORY_POINT*p=history_at(r,j-1);
        if(p->phase!=HIST_DIAGNOSE||!(p->valid&field)||last->tick-p->tick>1800000ULL)break;
        unsigned value=field==HIST_CPU?p->cpu:p->ram;
        if(value<threshold)break;
        if(n&&previous==p->tick)continue;
        n++;previous=p->tick;earliest=p->tick;
        if(n>=3&&last->tick-earliest>=30000ULL)return 1;
    }
    return 0;
}
