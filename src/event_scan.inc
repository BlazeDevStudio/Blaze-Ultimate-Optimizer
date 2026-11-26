/* Read-only legacy Event Log scan. A zero count is not a coverage result. */
enum { LOG_COMPLETE=1, LOG_OPEN_FAILED, LOG_READ_FAILED, LOG_LIMIT, LOG_MALFORMED };
typedef struct { DWORD count; DWORD error; int status; } LOG_SCAN;

static LOG_SCAN scan_events(LPCWSTR logname){
    LOG_SCAN result={0,0,LOG_OPEN_FAILED};
    HANDLE h=OpenEventLogW(0,logname);
    if(!h){result.error=GetLastError();return result;}
    static union { ULONGLONG align; BYTE bytes[65536]; } buffer;
    DWORD read=0,needed=0;
    FILETIME ft;GetSystemTimeAsFileTime(&ft);
    ULONGLONG seconds=ft64(ft)/10000000ULL;
    ULONGLONG cur=seconds>=11644473600ULL?seconds-11644473600ULL:0;
    g_current_event_epoch=cur;
    DWORD limit=(cur>7ULL*86400)?(DWORD)(cur-7ULL*86400):0;
    int isSystem=same_ci(logname,L"System");
    result.status=LOG_LIMIT;
    for(unsigned loops=0;loops<60;loops++){
        read=0;needed=0;
        if(!ReadEventLogW(h,EVENTLOG_SEQUENTIAL_READ|EVENTLOG_BACKWARDS_READ,0,buffer.bytes,sizeof(buffer.bytes),&read,&needed)){
            DWORD err=GetLastError();
            if(err==38){result.status=LOG_COMPLETE;} /* ERROR_HANDLE_EOF, including empty logs */
            else{result.status=LOG_READ_FAILED;result.error=err;}
            break;
        }
        if(!read||read>sizeof(buffer.bytes)){result.status=LOG_MALFORMED;break;}
        DWORD off=0;
        while(off<read){
            if(read-off<sizeof(EVENTLOGRECORD)){result.status=LOG_MALFORMED;goto done;}
            const EVENTLOGRECORD*r=(const EVENTLOGRECORD*)(buffer.bytes+off);
            if(r->Length<sizeof(EVENTLOGRECORD)+4||r->Length>read-off||(r->Length&3)){
                result.status=LOG_MALFORMED;goto done;
            }
            /* Trailing record length is part of the classic Event Log format. */
            if(*(const DWORD*)(buffer.bytes+off+r->Length-4)!=r->Length){
                result.status=LOG_MALFORMED;goto done;
            }
            if(r->TimeGenerated<limit){result.status=LOG_COMPLETE;goto done;}
            int id=(int)(r->EventID&0xFFFF);
            WCHAR provider[120];UINT pn=0;DWORD po=sizeof(EVENTLOGRECORD);int foundEnd=0;
            while(po+2<=r->Length-4){
                WCHAR wc=*(const WCHAR*)((const BYTE*)r+po);po+=2;
                if(!wc){foundEnd=1;break;}
                if(pn+1<120)provider[pn++]=wc;
            }
            provider[pn]=0;
            if(!foundEnd){result.status=LOG_MALFORMED;goto done;}
            if(r->EventType==EVENTLOG_ERROR_TYPE){if(isSystem)g_sys_err++;else g_app_err++;}
            if(isSystem){
                if(id==41&&(same_ci(provider,L"Kernel-Power")||same_ci(provider,L"Microsoft-Windows-Kernel-Power")))g_unexpected_power++;
                if(id==1001&&(same_ci(provider,L"BugCheck")||same_ci(provider,L"Microsoft-Windows-WER-SystemErrorReporting")))g_bsod++;
                if((id==7||id==51||id==55||id==129||id==153||id==157)&&(contains_ci(provider,L"Disk")||contains_ci(provider,L"stor")||contains_ci(provider,L"Ntfs")||contains_ci(provider,L"volmgr")||contains_ci(provider,L"iaStor")))g_storage_faults++;
                if((id==17||id==18||id==19)&&contains_ci(provider,L"WHEA"))g_whea++;
            }else{
                if(id==1000&&contains_ci(provider,L"Application Error")){g_crashes++;crash_insertions(r);}
                if(id==1002&&contains_ci(provider,L"Application Hang"))g_hangs++;
                if(id==1026&&contains_ci(provider,L".NET Runtime"))g_clr_events++;
            }
            result.count++;off+=r->Length;
            if(result.count>=3500)goto done;
        }
    }
done:
    CloseEventLog(h);
    return result;
}
