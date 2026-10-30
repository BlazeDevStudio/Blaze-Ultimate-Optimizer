/*
 * Blaze Ultimate Optimizer - low-overhead native Windows desktop utility.
 * Copyright (c) 2026. No third-party runtime. Works under a standard user.
 * Safe-by-default: no security-service shutdown, forced process termination,
 * pagefile removal, driver changes, or indiscriminate registry cleaning.
 */
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#define WIN32_LEAN_AND_MEAN 1
#define MAX_PATH 260
#define DLL __declspec(dllimport)
#define WINAPI __stdcall
#define NULL ((void*)0)
#define TRUE 1
#define FALSE 0
#define INVALID_HANDLE_VALUE ((HANDLE)(-1))

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned long long ULONGLONG, ULONG_PTR, SIZE_T, DWORDLONG, WPARAM;
typedef long long LPARAM, LRESULT;
typedef int BOOL;
typedef void *HANDLE, *HWND, *HINSTANCE, *HMENU, *HICON, *HCURSOR, *HBRUSH, *HGDIOBJ, *HKEY;
typedef unsigned short WCHAR;
typedef const WCHAR *LPCWSTR;
typedef WCHAR *LPWSTR;
typedef const char *LPCSTR;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef LRESULT (WINAPI *WNDPROC)(HWND,UINT,WPARAM,LPARAM);
typedef struct { LONG x,y; } POINT;
typedef struct { DWORD dwLowDateTime,dwHighDateTime; } FILETIME;
typedef struct { DWORD dwLength,dwMemoryLoad; DWORDLONG ullTotalPhys,ullAvailPhys,ullTotalPageFile,ullAvailPageFile,ullTotalVirtual,ullAvailVirtual,ullAvailExtendedVirtual; } MEMORYSTATUSEX;
typedef struct { DWORD cbSize,style;WNDPROC lpfnWndProc;int cbClsExtra,cbWndExtra;HINSTANCE hInstance;HICON hIcon;HCURSOR hCursor;HBRUSH hbrBackground;LPCWSTR lpszMenuName,lpszClassName;HICON hIconSm; } WNDCLASSEXW;
typedef struct { HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt; DWORD lPrivate; } MSG;
typedef struct { WORD wYear,wMonth,wDayOfWeek,wDay,wHour,wMinute,wSecond,wMilliseconds; } SYSTEMTIME;
typedef struct { DWORD dwSize,cntUsage,th32ProcessID; ULONG_PTR th32DefaultHeapID; DWORD th32ModuleID,cntThreads,th32ParentProcessID;LONG pcPriClassBase;DWORD dwFlags;WCHAR szExeFile[MAX_PATH]; } PROCESSENTRY32W;
typedef struct { DWORD cb,PageFaultCount; SIZE_T PeakWorkingSetSize,WorkingSetSize,QuotaPeakPagedPoolUsage,QuotaPagedPoolUsage,QuotaPeakNonPagedPoolUsage,QuotaNonPagedPoolUsage,PagefileUsage,PeakPagefileUsage,PrivateUsage; } PROCESS_MEMORY_COUNTERS_EX;
typedef struct { DWORD dwFileAttributes;FILETIME ftCreationTime,ftLastAccessTime,ftLastWriteTime;DWORD nFileSizeHigh,nFileSizeLow,dwReserved0,dwReserved1;WCHAR cFileName[MAX_PATH],cAlternateFileName[14]; } WIN32_FIND_DATAW;
typedef struct { DWORD Length,Reserved,RecordNumber,TimeGenerated,TimeWritten,EventID;WORD EventType,NumStrings,EventCategory,ReservedFlags;DWORD ClosingRecordNumber,StringOffset,UserSidLength,UserSidOffset,DataLength,DataOffset; } EVENTLOGRECORD;

DLL HINSTANCE WINAPI GetModuleHandleW(LPCWSTR);
DLL DWORD WINAPI GetLastError(void);
DLL void WINAPI ExitProcess(UINT);
DLL void WINAPI Sleep(DWORD);
DLL ULONGLONG WINAPI GetTickCount64(void);
DLL BOOL WINAPI GlobalMemoryStatusEx(MEMORYSTATUSEX*);
DLL BOOL WINAPI GetDiskFreeSpaceExW(LPCWSTR,ULONGLONG*,ULONGLONG*,ULONGLONG*);
DLL DWORD WINAPI GetEnvironmentVariableW(LPCWSTR,LPWSTR,DWORD);
DLL DWORD WINAPI GetTempPathW(DWORD,LPWSTR);
DLL void WINAPI GetLocalTime(SYSTEMTIME*);
DLL void WINAPI GetSystemTimeAsFileTime(FILETIME*);
DLL BOOL WINAPI GetSystemTimes(FILETIME*,FILETIME*,FILETIME*);
DLL BOOL WINAPI GetProcessTimes(HANDLE,FILETIME*,FILETIME*,FILETIME*,FILETIME*);
DLL HANDLE WINAPI CreateToolhelp32Snapshot(DWORD,DWORD);
DLL BOOL WINAPI Process32FirstW(HANDLE,PROCESSENTRY32W*);
DLL BOOL WINAPI Process32NextW(HANDLE,PROCESSENTRY32W*);
DLL HANDLE WINAPI OpenProcess(DWORD,BOOL,DWORD);
DLL BOOL WINAPI K32GetProcessMemoryInfo(HANDLE,void*,DWORD);
DLL BOOL WINAPI GetProcessIoCounters(HANDLE,void*);
DLL BOOL WINAPI CloseHandle(HANDLE);
DLL HANDLE WINAPI FindFirstFileW(LPCWSTR,WIN32_FIND_DATAW*);
DLL BOOL WINAPI FindNextFileW(HANDLE,WIN32_FIND_DATAW*);
DLL BOOL WINAPI FindClose(HANDLE);
DLL BOOL WINAPI DeleteFileW(LPCWSTR);
DLL DWORD WINAPI GetFileAttributesW(LPCWSTR);
DLL HANDLE WINAPI CreateFileW(LPCWSTR,DWORD,DWORD,void*,DWORD,DWORD,HANDLE);
DLL BOOL WINAPI WriteFile(HANDLE,LPCVOID,DWORD,DWORD*,void*);
DLL HANDLE WINAPI CreateThread(void*,SIZE_T,DWORD (WINAPI*)(void*),void*,DWORD,DWORD*);
DLL DWORD WINAPI GetCurrentThreadId(void);
DLL LONG WINAPI RegOpenKeyExW(HKEY,LPCWSTR,DWORD,DWORD,HKEY*);
DLL LONG WINAPI RegCreateKeyExW(HKEY,LPCWSTR,DWORD,LPWSTR,DWORD,DWORD,void*,HKEY*,DWORD*);
DLL LONG WINAPI RegQueryValueExW(HKEY,LPCWSTR,DWORD*,DWORD*,BYTE*,DWORD*);
DLL LONG WINAPI RegSetValueExW(HKEY,LPCWSTR,DWORD,DWORD,const BYTE*,DWORD);
DLL LONG WINAPI RegDeleteValueW(HKEY,LPCWSTR);
DLL LONG WINAPI RegEnumValueW(HKEY,DWORD,LPWSTR,DWORD*,DWORD*,DWORD*,BYTE*,DWORD*);
DLL LONG WINAPI RegCloseKey(HKEY);
DLL HANDLE WINAPI OpenEventLogW(LPCWSTR,LPCWSTR);
DLL BOOL WINAPI ReadEventLogW(HANDLE,DWORD,DWORD,LPVOID,DWORD,DWORD*,DWORD*);
DLL BOOL WINAPI CloseEventLog(HANDLE);
DLL BOOL WINAPI SystemParametersInfoW(UINT,UINT,void*,UINT);
DLL int WINAPI MessageBoxW(HWND,LPCWSTR,LPCWSTR,UINT);
DLL BOOL WINAPI PostMessageW(HWND,UINT,WPARAM,LPARAM);
DLL LRESULT WINAPI SendMessageW(HWND,UINT,WPARAM,LPARAM);
DLL LRESULT WINAPI DefWindowProcW(HWND,UINT,WPARAM,LPARAM);
DLL HCURSOR WINAPI LoadCursorW(HINSTANCE,LPCWSTR);
DLL BOOL WINAPI RegisterClassExW(const WNDCLASSEXW*);
DLL HWND WINAPI CreateWindowExW(DWORD,LPCWSTR,LPCWSTR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,void*);
DLL BOOL WINAPI ShowWindow(HWND,int);
DLL BOOL WINAPI UpdateWindow(HWND);
DLL BOOL WINAPI SetWindowTextW(HWND,LPCWSTR);
DLL BOOL WINAPI MoveWindow(HWND,int,int,int,int,BOOL);
DLL BOOL WINAPI EnableWindow(HWND,BOOL);
DLL BOOL WINAPI DestroyWindow(HWND);
DLL void WINAPI PostQuitMessage(int);
DLL int WINAPI GetMessageW(MSG*,HWND,UINT,UINT);
DLL BOOL WINAPI TranslateMessage(const MSG*);
DLL LRESULT WINAPI DispatchMessageW(const MSG*);
DLL HGDIOBJ WINAPI GetStockObject(int);
DLL void* WINAPI ShellExecuteW(HWND,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR,int);
DLL HANDLE WINAPI OpenSCManagerW(LPCWSTR,LPCWSTR,DWORD);
DLL HANDLE WINAPI OpenServiceW(HANDLE,LPCWSTR,DWORD);
DLL BOOL WINAPI QueryServiceConfigW(HANDLE,void*,DWORD,DWORD*);
DLL BOOL WINAPI ChangeServiceConfigW(HANDLE,DWORD,DWORD,DWORD,LPCWSTR,LPCWSTR,DWORD*,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR);
DLL BOOL WINAPI CloseServiceHandle(HANDLE);
DLL HICON WINAPI LoadIconW(HINSTANCE,LPCWSTR);
DLL HBRUSH WINAPI CreateSolidBrush(DWORD);
DLL DWORD WINAPI SetTextColor(HANDLE,DWORD);
DLL int WINAPI SetBkMode(HANDLE,int);
DLL BOOL WINAPI DeleteObject(void*);
DLL HBRUSH WINAPI SetClassLongPtrW(HWND,int,HBRUSH); /* reserved; unused */


#define HKEY_CURRENT_USER ((HKEY)(LONG)0x80000001)
#define HKEY_LOCAL_MACHINE ((HKEY)(LONG)0x80000002)
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define REG_DWORD 4
#define REG_SZ 1
#define ERROR_SUCCESS 0
#define WM_CREATE 1
#define WM_DESTROY 2
#define WM_SIZE 5
#define WM_COMMAND 0x0111
#define WM_SETFONT 0x0030
#define WM_APP 0x8000
#define WS_OVERLAPPEDWINDOW 0x00CF0000
#define WS_CHILD 0x40000000
#define WS_VISIBLE 0x10000000
#define WS_VSCROLL 0x00200000
#define WS_BORDER 0x00800000
#define ES_MULTILINE 0x0004
#define ES_AUTOVSCROLL 0x0040
#define ES_READONLY 0x0800
#define ES_NOHIDESEL 0x0100
#define SW_SHOWNORMAL 1
#define MB_OK 0
#define MB_ICONWARNING 0x30
#define MB_ICONINFORMATION 0x40
#define MB_YESNO 4
#define IDYES 6
#define SPI_GETUIEFFECTS 0x103E
#define SPI_SETUIEFFECTS 0x103F
#define SPIF_UPDATEINIFILE 1
#define SPIF_SENDCHANGE 2
#define TH32CS_SNAPPROCESS 0x00000002
#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000
#define PROCESS_VM_READ 0x0010
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_REPARSE_POINT 0x400
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#define GENERIC_WRITE 0x40000000
#define FILE_SHARE_READ 1
#define CREATE_ALWAYS 2
#define FILE_ATTRIBUTE_NORMAL 0x80
#define EVENTLOG_SEQUENTIAL_READ 1
#define EVENTLOG_BACKWARDS_READ 8
#define EVENTLOG_ERROR_TYPE 1
#define EVENTLOG_WARNING_TYPE 2
#define DEFAULT_GUI_FONT 17
#define IDC_ARROW ((LPCWSTR)(ULONG_PTR)32512)
#define ID_DIAGNOSE 1001
#define ID_FIX 1002
#define ID_CLEAN 1003
#define ID_RESTORE 1004
#define ID_REPORT 1005
#define ID_STARTUP 1006
#define ID_ADVANCED 1007
#define ID_ROLLBACK 1008
#define WM_CTLCOLORSTATIC 0x0138
#define WM_CTLCOLOREDIT 0x0133
#define WM_CTLCOLORBTN 0x0135
#define BS_PUSHBUTTON 0x00000000
#define WS_TABSTOP 0x00010000
#define TRANSPARENT 1
#define DT_CENTER 1
#define DT_VCENTER 4
#define DT_SINGLELINE 0x20
#define SC_MANAGER_CONNECT 0x0001
#define SERVICE_QUERY_CONFIG 0x0001
#define SERVICE_CHANGE_CONFIG 0x0002
#define SERVICE_NO_CHANGE 0xFFFFFFFF
#define SERVICE_DISABLED 4
#define ICON_RESOURCE 101


