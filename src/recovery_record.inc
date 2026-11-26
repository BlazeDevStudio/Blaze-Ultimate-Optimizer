/* Versioned fixed-size records, checksum for accidental corruption, not authentication. */
enum { REC_VISUAL=1,REC_SEARCH=2,REC_SYSMAIN=3 };
enum { REC_ABSENT=0,REC_VALID=1,REC_INVALID=2 };
typedef struct {DWORD version,kind,a,b,c,checksum;} REC_RECORD;
static DWORD rec_checksum(REC_RECORD r){
    DWORD v[5]={r.version,r.kind,r.a,r.b,r.c},hash=2166136261u;
    for(unsigned i=0;i<5;i++)for(unsigned j=0;j<4;j++){hash^=(v[i]>>(8*j))&255u;hash*=16777619u;}
    return hash;
}
static REC_RECORD rec_make(DWORD kind,DWORD a,DWORD b,DWORD c){
    REC_RECORD r={1,kind,a,b,c,0};r.checksum=rec_checksum(r);return r;
}
static int rec_valid(REC_RECORD r,DWORD kind){
    if(r.version!=1||r.kind!=kind||r.checksum!=rec_checksum(r))return 0;
    if(kind==REC_VISUAL)return r.a<=1&&r.b<=1&&r.c<=1&&(r.b||!r.c);
    if(kind==REC_SEARCH||kind==REC_SYSMAIN)return r.a>=2&&r.a<=4&&r.b<=1&&!r.c&&(r.a==2||!r.b);
    return 0;
}
static int rec_same(REC_RECORD a,REC_RECORD b){
    return a.version==b.version&&a.kind==b.kind&&a.a==b.a&&a.b==b.b&&a.c==b.c&&a.checksum==b.checksum;
}
