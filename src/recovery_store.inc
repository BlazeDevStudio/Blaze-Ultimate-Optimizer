#include "recovery_record.inc"
static const WCHAR* recovery_path=L"Software\\BlazeSystems\\UltimateOptimizer";
static const WCHAR* personalize_path=L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
static const WCHAR* visual_record=L"RecoveryVisualV1";
static int rec_read_dword(HKEY h,LPCWSTR name,DWORD*out){
    DWORD t=0,n=4;LONG e=RegQueryValueExW(h,name,0,&t,(BYTE*)out,&n);
    if(e==2)return REC_ABSENT;
    return !e&&t==REG_DWORD&&n==4?REC_VALID:REC_INVALID;
}
static int rec_load(HKEY h,LPCWSTR name,DWORD kind,REC_RECORD*out){
    DWORD type=0,n=sizeof(*out);LONG e=RegQueryValueExW(h,name,0,&type,(BYTE*)out,&n);
    if(e==2)return REC_ABSENT;
    return !e&&type==3&&n==sizeof(*out)&&rec_valid(*out,kind)?REC_VALID:REC_INVALID;
}
static int rec_save(HKEY h,LPCWSTR name,REC_RECORD r){
    REC_RECORD old;
    if(!rec_valid(r,r.kind)||rec_load(h,name,r.kind,&old)!=REC_ABSENT)return 0;
    if(RegSetValueExW(h,name,0,3,(const BYTE*)&r,sizeof(r))||RegFlushKey(h))return 0;
    return rec_load(h,name,r.kind,&old)==REC_VALID&&rec_same(r,old);
}
static int rec_clear(HKEY h,LPCWSTR name,REC_RECORD r){
    REC_RECORD current;
    if(rec_load(h,name,r.kind,&current)!=REC_VALID||!rec_same(r,current))return 0;
    if(RegDeleteValueW(h,name))return 0;
    return !RegFlushKey(h);
}
static HANDLE rec_lock(void){
    HANDLE m=CreateMutexW(0,0,L"Global\\BlazeSystems.UltimateOptimizer.RecoveryV1");
    if(!m)return 0;
    DWORD w=WaitForSingleObject(m,0);
    if(w==0||w==0x80)return m;
    CloseHandle(m);return 0;
}
static void rec_unlock(HANDLE m){ReleaseMutex(m);CloseHandle(m);}
static int transparency_read(DWORD*out){
    HKEY h=0;LONG e=RegOpenKeyExW(HKEY_CURRENT_USER,personalize_path,0,KEY_READ,&h);
    if(e==2){*out=0;return REC_ABSENT;}if(e)return REC_INVALID;
    int s=rec_read_dword(h,L"EnableTransparency",out);RegCloseKey(h);
    if(s==REC_VALID&&*out>1)return REC_INVALID;
    if(s==REC_ABSENT)*out=0;
    return s;
}
static int transparency_apply(DWORD exists,DWORD value){
    HKEY h=0;LONG e;
    if(exists){
        if(RegCreateKeyExW(HKEY_CURRENT_USER,personalize_path,0,0,0,KEY_READ|KEY_WRITE,0,&h,0))return 0;
        e=RegSetValueExW(h,L"EnableTransparency",0,REG_DWORD,(BYTE*)&value,4);
    }else{
        e=RegOpenKeyExW(HKEY_CURRENT_USER,personalize_path,0,KEY_READ|KEY_WRITE,&h);
        if(e==2)return 1;
        if(e)return 0;
        e=RegDeleteValueW(h,L"EnableTransparency");if(e==2)e=0;
    }
    int ok=!e&&!RegFlushKey(h);RegCloseKey(h);
    DWORD actual=0;int s=transparency_read(&actual);
    return ok&&(exists?s==REC_VALID&&actual==value:s==REC_ABSENT);
}
static int effects_read(DWORD*out){
    BOOL value=0;if(!SystemParametersInfoW(SPI_GETUIEFFECTS,0,&value,0))return 0;
    *out=value?1:0;return 1;
}
static int effects_apply(DWORD value){
    DWORD actual=0;
    /* SPI_SETUIEFFECTS accepts the BOOL value in pvParam, not a pointer to BOOL. */
    return SystemParametersInfoW(SPI_SETUIEFFECTS,0,(void*)(ULONG_PTR)value,SPIF_UPDATEINIFILE|SPIF_SENDCHANGE)&&effects_read(&actual)&&actual==value;
}
