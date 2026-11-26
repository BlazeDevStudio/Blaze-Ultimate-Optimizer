#include <assert.h>
#include <stdio.h>
#include "../src/optimization_rules.inc"
static OPT_INPUT clean(void){
    OPT_INPUT i={0};i.diskKnown=i.ramKnown=i.cpuKnown=i.logsComplete=i.metricsComplete=i.diskComplete=i.adaptersComplete=1;return i;
}
static const OPT_ITEM* find(const OPT_PLAN*p,unsigned id){
    for(unsigned j=0;j<p->count;j++)if(p->items[j].id==id)return &p->items[j];
    return 0;
}
int main(void){
    OPT_INPUT i=clean();OPT_PLAN p=opt_evaluate(&i);assert(p.count==1&&p.items[0].id==OPT_OBSERVE);
    i=(OPT_INPUT){0};p=opt_evaluate(&i);assert(p.count==1&&p.items[0].id==OPT_COVERAGE);
    i=clean();i.diskKnown=0;i.extremeSpace=1;i.lowSpace=1;p=opt_evaluate(&i);assert(!find(&p,OPT_SPACE));
    i=clean();i.extremeSpace=1;p=opt_evaluate(&i);assert(p.count==1&&p.items[0].priority==OPT_URGENT&&p.items[0].action==OPT_CONFIRM_CLEANUP);
    i.storage=1;p=opt_evaluate(&i);assert(p.count==2&&p.items[0].id==OPT_BACKUP&&p.items[1].action==OPT_PROTECT);
    i.logsComplete=0;p=opt_evaluate(&i);assert(find(&p,OPT_BACKUP)->evidence==OPT_PARTIAL&&find(&p,OPT_COVERAGE));
    i=clean();i.lowSpace=1;p=opt_evaluate(&i);assert(find(&p,OPT_SPACE)->priority==OPT_HIGH);
    i=clean();i.highRam=i.highCpu=1;i.ramKnown=i.cpuKnown=0;p=opt_evaluate(&i);assert(!find(&p,OPT_MEMORY)&&!find(&p,OPT_CPU));
    i=clean();i.highRam=i.highCpu=1;p=opt_evaluate(&i);assert(p.count==2&&find(&p,OPT_CPU)->evidence==OPT_OBSERVED);
    i.repeatRam=i.repeatCpu=1;p=opt_evaluate(&i);assert(p.count==2&&find(&p,OPT_MEMORY)->evidence==OPT_REPEATED&&find(&p,OPT_CPU)->priority==OPT_HIGH);
    i=clean();i.slowDisk=1;i.diskComplete=0;p=opt_evaluate(&i);assert(find(&p,OPT_DISK_RESPONSE)->evidence==OPT_PARTIAL&&find(&p,OPT_COVERAGE));
    i=clean();i.startup=7;i.sysErrors=11;p=opt_evaluate(&i);assert(p.count==1&&p.items[0].id==OPT_OBSERVE);
    i.startup=8;i.sysErrors=12;p=opt_evaluate(&i);assert(find(&p,OPT_STARTUP)->evidence==OPT_PARTIAL&&find(&p,OPT_ERRORS));
    i=clean();i.crashes=1;i.hangs=2;p=opt_evaluate(&i);assert(p.count==1&&p.items[0].id==OPT_APP);
    i=clean();i.storage=i.whea=i.restart=i.bugcheck=i.crashes=i.extremeSpace=i.slowDisk=i.repeatRam=i.repeatCpu=1;i.sysErrors=12;i.startup=8;i.adaptersComplete=0;
    p=opt_evaluate(&i);assert(p.count==12&&p.items[0].id==OPT_BACKUP&&!find(&p,OPT_OBSERVE));
    for(unsigned j=1;j<p.count;j++)assert(p.items[j-1].priority<=p.items[j].priority);
    for(unsigned j=0;j<p.count;j++)for(unsigned k=j+1;k<p.count;k++)assert(p.items[j].id!=p.items[k].id);
    puts("PASS: 16 advisory policy boundary/behavior groups");return 0;
}
