/* Deterministic, advisory-only policy. No operating-system APIs or write actions. */
enum { OPT_BACKUP=1,OPT_HARDWARE,OPT_RESTART,OPT_BUGCHECK,OPT_SPACE,OPT_DISK_RESPONSE,OPT_MEMORY,OPT_CPU,OPT_APP,OPT_STARTUP,OPT_ERRORS,OPT_COVERAGE,OPT_OBSERVE };
enum { OPT_URGENT=1,OPT_HIGH,OPT_REVIEW,OPT_INFO };
enum { OPT_PROTECT=1,OPT_MANUAL,OPT_REMEASURE,OPT_CONFIRM_CLEANUP };
enum { OPT_OBSERVED=1,OPT_REPEATED,OPT_PARTIAL };
typedef struct {
    unsigned storage,whea,restart,bugcheck,crashes,hangs,sysErrors,appErrors,startup;
    unsigned diskKnown,extremeSpace,lowSpace,slowDisk,ramKnown,highRam,cpuKnown,highCpu,repeatRam,repeatCpu;
    unsigned logsComplete,metricsComplete,diskComplete,adaptersComplete;
} OPT_INPUT;
typedef struct {unsigned id,priority,action,evidence;} OPT_ITEM;
#define OPT_CAP 13
typedef struct {OPT_ITEM items[OPT_CAP];unsigned count;} OPT_PLAN;
static void opt_add(OPT_PLAN*p,unsigned id,unsigned priority,unsigned action,unsigned evidence){
    if(p->count>=OPT_CAP)return;
    unsigned j=p->count++;
    while(j&&p->items[j-1].priority>priority){p->items[j]=p->items[j-1];j--;}
    OPT_ITEM item={id,priority,action,evidence};p->items[j]=item;
}
static OPT_PLAN opt_evaluate(const OPT_INPUT*i){
    OPT_PLAN p={0};
    unsigned logEvidence=i->logsComplete?OPT_OBSERVED:OPT_PARTIAL;
    if(i->storage)opt_add(&p,OPT_BACKUP,OPT_URGENT,OPT_PROTECT,logEvidence);
    if(i->diskKnown&&(i->extremeSpace||i->lowSpace))
        opt_add(&p,OPT_SPACE,i->extremeSpace?OPT_URGENT:OPT_HIGH,i->storage?OPT_PROTECT:OPT_CONFIRM_CLEANUP,OPT_OBSERVED);
    if(i->whea)opt_add(&p,OPT_HARDWARE,OPT_HIGH,OPT_MANUAL,logEvidence);
    if(i->bugcheck)opt_add(&p,OPT_BUGCHECK,OPT_HIGH,OPT_MANUAL,logEvidence);
    if(i->restart)opt_add(&p,OPT_RESTART,OPT_HIGH,OPT_MANUAL,logEvidence);
    if(i->slowDisk)opt_add(&p,OPT_DISK_RESPONSE,OPT_HIGH,OPT_REMEASURE,i->diskComplete?OPT_OBSERVED:OPT_PARTIAL);
    if(i->repeatRam||(i->ramKnown&&i->highRam))
        opt_add(&p,OPT_MEMORY,i->repeatRam?OPT_HIGH:OPT_REVIEW,OPT_MANUAL,i->repeatRam?OPT_REPEATED:OPT_OBSERVED);
    if(i->repeatCpu||(i->cpuKnown&&i->highCpu))
        opt_add(&p,OPT_CPU,i->repeatCpu?OPT_HIGH:OPT_REVIEW,OPT_MANUAL,i->repeatCpu?OPT_REPEATED:OPT_OBSERVED);
    if(i->crashes||i->hangs)opt_add(&p,OPT_APP,OPT_REVIEW,OPT_MANUAL,logEvidence);
    if(i->startup>=8)opt_add(&p,OPT_STARTUP,OPT_REVIEW,OPT_MANUAL,OPT_PARTIAL);
    if(i->sysErrors>=12||i->appErrors>=12)opt_add(&p,OPT_ERRORS,OPT_REVIEW,OPT_MANUAL,logEvidence);
    if(!i->logsComplete||!i->metricsComplete||!i->diskComplete||!i->adaptersComplete)
        opt_add(&p,OPT_COVERAGE,OPT_REVIEW,OPT_REMEASURE,OPT_PARTIAL);
    if(!p.count)opt_add(&p,OPT_OBSERVE,OPT_INFO,OPT_REMEASURE,OPT_OBSERVED);
    return p;
}
