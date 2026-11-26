#include <assert.h>
#include <stdint.h>
#include <stdio.h>
typedef uint32_t DWORD;
#include "../src/recovery_record.inc"
int main(void){
    REC_RECORD r=rec_make(REC_VISUAL,1,1,0);assert(rec_valid(r,REC_VISUAL)&&rec_same(r,r));
    assert(sizeof(r)==24&&!rec_valid(r,REC_SEARCH));
    REC_RECORD x=r;x.a=0;assert(!rec_valid(x,REC_VISUAL)&&!rec_same(x,r));
    x=r;x.version=2;x.checksum=rec_checksum(x);assert(!rec_valid(x,REC_VISUAL));
    assert(rec_valid(rec_make(REC_VISUAL,0,0,0),REC_VISUAL));
    assert(!rec_valid(rec_make(REC_VISUAL,0,0,1),REC_VISUAL));
    assert(!rec_valid(rec_make(REC_VISUAL,2,1,0),REC_VISUAL));
    assert(!rec_valid(rec_make(REC_VISUAL,1,2,0),REC_VISUAL));
    assert(!rec_valid(rec_make(REC_VISUAL,1,1,2),REC_VISUAL));
    for(DWORD k=REC_SEARCH;k<=REC_SYSMAIN;k++){
        for(DWORD s=2;s<=4;s++)assert(rec_valid(rec_make(k,s,0,0),k));
        assert(rec_valid(rec_make(k,2,1,0),k));
        assert(!rec_valid(rec_make(k,3,1,0),k));
        assert(!rec_valid(rec_make(k,4,1,0),k));
        assert(!rec_valid(rec_make(k,0,0,0),k));
        assert(!rec_valid(rec_make(k,5,0,0),k));
        assert(!rec_valid(rec_make(k,2,0,1),k));
    }
    assert(!rec_valid(rec_make(99,0,0,0),99));
    puts("PASS: recovery record corruption, kind, version and domain validation");return 0;
}
