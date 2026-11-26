#include "dashboard_layout.inc"
#define DASH_HISTORY 12
static HWND dash_metrics[6],dash_priority,dash_chart;
static HISTORY_POINT dash_points[DASH_HISTORY];
static unsigned dash_count;
static HBRUSH dash_cpu_brush,dash_ram_brush,dash_grid_brush;
static void dash_addnum(WCHAR*s,ULONGLONG value,UINT cap){
    WCHAR rev[24],buf[24];UINT n=0;
    do{rev[n++]=(WCHAR)(L'0'+value%10);value/=10;}while(value&&n<23);
    for(UINT j=0;j<n;j++)buf[j]=rev[n-1-j];buf[n]=0;wcat(s,buf,cap);
}
static void dash_value(int index,LPCWSTR label,int valid,ULONGLONG value,LPCWSTR unit){
    WCHAR text[100];wcopy(text,label,100);
    if(valid){dash_addnum(text,value,100);wcat(text,unit,100);}else wcat(text,L"unavailable",100);
    SetWindowTextW(dash_metrics[index],text);
}
static void dash_pair(int index,LPCWSTR label,const HISTORY_POINT*p,unsigned first,unsigned second,ULONGLONG a,ULONGLONG b){
    WCHAR text[100];wcopy(text,label,100);
    if(p&&(p->valid&first))dash_addnum(text,a/1024,100);else wcat(text,L"-",100);
    wcat(text,L" / ",100);
    if(p&&(p->valid&second))dash_addnum(text,b/1024,100);else wcat(text,L"-",100);
    wcat(text,L" KiB/s",100);SetWindowTextW(dash_metrics[index],text);
}
static void dashboard_publish(void){
    /* UI thread only; worker has finished all writes before posting completion. */
    unsigned first=g_history.count>DASH_HISTORY?g_history.count-DASH_HISTORY:0;
    dash_count=0;
    for(unsigned i=first;i<g_history.count;i++)dash_points[dash_count++]=*history_at(&g_history,i);
    const HISTORY_POINT*p=dash_count?&dash_points[dash_count-1]:0;
    dash_value(0,L"CPU avg: ",p&&(p->valid&HIST_CPU),p?p->cpu:0,L"%");
    dash_value(1,L"RAM avg: ",p&&(p->valid&HIST_RAM),p?p->ram:0,L"%");
    dash_value(2,L"GPU engine peak: ",p&&(p->valid&HIST_GPU),p?p->gpu:0,L"%");
    dash_value(3,L"System free: ",g_disk_ok,g_free/1048576,L" MiB");
    dash_pair(4,L"Disk R/W: ",p,HIST_DISK_READ,HIST_DISK_WRITE,p?p->readBps:0,p?p->writeBps:0);
    dash_pair(5,L"Net RX/TX: ",p,HIST_NET_RX,HIST_NET_TX,p?p->rxBps:0,p?p->txBps:0);
    WCHAR priority[128];wcopy(priority,L"Last scan: ",128);
    wcat(priority,g_last_priority==OPT_URGENT?L"URGENT":g_last_priority==OPT_HIGH?L"HIGH":g_last_priority==OPT_REVIEW?L"REVIEW":L"INFO",128);
    wcat(priority,L" | ",128);dash_addnum(priority,g_last_findings,128);wcat(priority,L" advisory items | details below",128);
    SetWindowTextW(dash_priority,priority);InvalidateRect(dash_chart,0,TRUE);
}
static void dash_rect(HANDLE dc,int l,int t,int r,int b,HBRUSH brush){
    if(r>l&&b>t&&brush){BLAZE_RECT x={l,t,r,b};FillRect(dc,&x,brush);}
}
static LRESULT WINAPI dashboard_chart_proc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    if(msg==0x000f){
        BLAZE_PAINT ps;HANDLE dc=BeginPaint(hwnd,&ps);
        if(dc){
            BLAZE_RECT rc;if(GetClientRect(hwnd,&rc)){
                int w=rc.right,h=rc.bottom;
                FillRect(dc,&rc,g_panelBrush);
                HANDLE old=SelectObject(dc,GetStockObject(DEFAULT_GUI_FONT));
                SetBkMode(dc,TRANSPARENT);SetTextColor(dc,0x00FFFFFF);
                BLAZE_RECT label={8,2,w-8,22};
                DrawTextW(dc,L"Recent scans | CPU left / RAM right (%) | gaps not sampled",-1,&label,0x20|0x800);
                if(w>=100&&h>=45){
                    int bottom=h-10,top=27,height=bottom-top,slot=(w-20)/DASH_HISTORY;
                    dash_rect(dc,8,bottom,w-8,bottom+1,dash_grid_brush);
                    dash_rect(dc,8,top,w-8,top+1,dash_grid_brush);
                    for(unsigned i=0;i<dash_count;i++){
                        const HISTORY_POINT*p=&dash_points[i];
                        int left=10+(int)i*slot,bw=(slot-4)/2;if(bw<1)bw=1;
                        if(p->valid&HIST_CPU){int a=dash_bar_height(p->cpu,height);if(!a)a=1;dash_rect(dc,left,bottom-a,left+bw,bottom,dash_cpu_brush);}
                        if(p->valid&HIST_RAM){int b=dash_bar_height(p->ram,height);if(!b)b=1;dash_rect(dc,left+bw+1,bottom-b,left+2*bw+1,bottom,dash_ram_brush);}
                    }
                }
                if(old)SelectObject(dc,old);
            }
        }
        EndPaint(hwnd,&ps);return 0;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
static int dashboard_register(void){
    WNDCLASSEXW c={0};c.cbSize=sizeof(c);c.lpfnWndProc=dashboard_chart_proc;c.hInstance=g_inst;c.hbrBackground=g_panelBrush;c.lpszClassName=L"BlazeHistoryChartV1";
    dash_cpu_brush=CreateSolidBrush(0x00e3aa36);dash_ram_brush=CreateSolidBrush(0x006dc6f5);dash_grid_brush=CreateSolidBrush(0x00605249);
    return RegisterClassExW(&c)!=0;
}
static void dashboard_create(HWND hwnd){
    LPCWSTR initial[6]={L"CPU avg: not scanned",L"RAM avg: not scanned",L"GPU engine peak: not scanned",L"System free: not scanned",L"Disk R/W: not scanned",L"Net RX/TX: not scanned"};
    for(int i=0;i<6;i++){
        dash_metrics[i]=CreateWindowExW(0,L"STATIC",initial[i],WS_CHILD|WS_VISIBLE|0x4000,0,0,10,10,hwnd,0,g_inst,0);
        SendMessageW(dash_metrics[i],WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
    }
    dash_priority=CreateWindowExW(0,L"STATIC",L"Run DIAGNOSE for a resource overview and recommendations.",WS_CHILD|WS_VISIBLE|0x4000,0,0,10,10,hwnd,0,g_inst,0);
    SendMessageW(dash_priority,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
    dash_chart=CreateWindowExW(0,L"BlazeHistoryChartV1",L"CPU (left blue) and RAM (right amber) recent scan history; exact values in report",WS_CHILD,0,0,10,10,hwnd,0,g_inst,0);
}
