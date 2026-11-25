/* Bounded, testable parsing for native inventory. No writes or persistent IDs. */
static int inventory_text(WCHAR*s,DWORD bytes,DWORD capacity){
    if(!bytes||bytes>capacity*sizeof(WCHAR)||(bytes%sizeof(WCHAR)))return 0;
    DWORD n=bytes/sizeof(WCHAR);
    for(DWORD i=0;i<n;i++){
        WCHAR c=s[i];
        if(!c)return i>0;
        if(c<32||(c>=127&&c<=159)||c==0x2028||c==0x2029||(c>=0x202a&&c<=0x202e)||(c>=0x2066&&c<=0x2069))s[i]=L' ';
    }
    return 0;
}
static DWORD inventory_u32(const BYTE*p){
    return (DWORD)p[0]|((DWORD)p[1]<<8)|((DWORD)p[2]<<16)|((DWORD)p[3]<<24);
}
static int inventory_core_count(const BYTE*b,DWORD bytes,DWORD*out){
    DWORD pos=0,n=0;*out=0;
    while(pos<bytes){
        if(bytes-pos<8)return 0;
        DWORD relation=inventory_u32(b+pos),size=inventory_u32(b+pos+4);
        /* 8 byte header + 24 byte processor relationship prefix + >=1 16 byte group affinity. */
        if(relation!=0||size<48||size>bytes-pos||(size&7))return 0;
        DWORD groups=(DWORD)b[pos+30]|((DWORD)b[pos+31]<<8);
        if(!groups||groups>(size-32)/16)return 0;
        n++;pos+=size;
    }
    if(!n)return 0;
    *out=n;return 1;
}
