#include "inventory_helpers.inc"
static int inventory_reg_text(HKEY h,LPCWSTR value,WCHAR*out,DWORD cap){
    DWORD type=0,bytes=cap*sizeof(WCHAR);out[0]=0;
    if(RegQueryValueExW(h,value,0,&type,(BYTE*)out,&bytes)||type!=REG_SZ)return 0;
    return inventory_text(out,bytes,cap);
}
static void inventory_field(HKEY h,LPCWSTR value,LPCWSTR label){
    WCHAR buf[160];append(label);
    if(h&&inventory_reg_text(h,value,buf,160))line(buf);else line(L"unavailable");
}
static void inventory_system(void){
    heading(L"HARDWARE AND WINDOWS INVENTORY");
    BLAZE_SYSINFO info;GetNativeSystemInfo(&info);
    append(L"Native processor architecture: ");
    switch(info.architecture){case 9:line(L"x64");break;case 12:line(L"ARM64");break;case 0:line(L"x86");break;default:line(L"other/unknown");}
    DWORD logical=GetActiveProcessorCount(0xffff);
    append(L"Active logical processors (all groups): ");if(logical)num(logical);else append(L"unavailable");nl();
    static union {ULONGLONG align;BYTE bytes[65536];} topology;
    DWORD bytes=sizeof(topology.bytes),cores=0;
    append(L"Active physical cores: ");
    if(GetLogicalProcessorInformationEx(0,topology.bytes,&bytes)&&bytes<=sizeof(topology.bytes)&&inventory_core_count(topology.bytes,bytes,&cores))num(cores);
    else append(L"unavailable");nl();
    ULONGLONG kib=0;append(L"Firmware-reported installed RAM: ");
    if(GetPhysicallyInstalledSystemMemory(&kib)&&kib){num(kib/1024);line(L" MiB");}else line(L"unavailable");
    DWORD firmware=0;append(L"Firmware boot type: ");
    if(GetFirmwareType(&firmware)&&firmware==1)line(L"BIOS");else if(firmware==2)line(L"UEFI");else line(L"unavailable");
    HKEY h=0;
    RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",0,KEY_READ,&h);
    inventory_field(h,L"ProcessorNameString",L"Processor 0 name: ");if(h)RegCloseKey(h);h=0;
    RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"HARDWARE\\DESCRIPTION\\System\\BIOS",0,KEY_READ,&h);
    inventory_field(h,L"SystemManufacturer",L"System manufacturer: ");
    inventory_field(h,L"SystemProductName",L"System model: ");
    inventory_field(h,L"BaseBoardManufacturer",L"Board manufacturer: ");
    inventory_field(h,L"BaseBoardProduct",L"Board model: ");
    inventory_field(h,L"BIOSVendor",L"BIOS vendor: ");
    inventory_field(h,L"BIOSVersion",L"BIOS version: ");
    if(h)RegCloseKey(h);h=0;
    RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",0,KEY_READ|0x0100,&h);
    inventory_field(h,L"EditionID",L"Windows edition identifier: ");
    inventory_field(h,L"DisplayVersion",L"Windows display version: ");
    inventory_field(h,L"CurrentBuildNumber",L"Windows build: ");
    DWORD ubr=0,type=0,len=sizeof(ubr);append(L"Windows update build revision: ");
    if(h&&!RegQueryValueExW(h,L"UBR",0,&type,(BYTE*)&ubr,&len)&&type==REG_DWORD&&len==sizeof(ubr))num(ubr);else append(L"unavailable");nl();
    if(h)RegCloseKey(h);
    line(L"Firmware/registry fields may be absent or vendor placeholders. No serial numbers or machine identifiers requested.");
}
static const BLAZE_GUID inventory_disk={0x4d36e967,0xe325,0x11ce,{0xbf,0xc1,0x08,0x00,0x2b,0xe1,0x03,0x18}};
static const BLAZE_GUID inventory_display={0x4d36e968,0xe325,0x11ce,{0xbf,0xc1,0x08,0x00,0x2b,0xe1,0x03,0x18}};
static const BLAZE_GUID inventory_net={0x4d36e972,0xe325,0x11ce,{0xbf,0xc1,0x08,0x00,0x2b,0xe1,0x03,0x18}};
static void inventory_devices(const BLAZE_GUID*guid,LPCWSTR label){
    line(label);
    HANDLE set=SetupDiGetClassDevsW(guid,0,0,2); /* DIGCF_PRESENT */
    if(set==INVALID_HANDLE_VALUE){line(L"  enumeration unavailable");return;}
    DWORD count=0,shown=0,missing=0;int complete=0;
    for(DWORD i=0;i<256;i++){
        BLAZE_DEVINFO dev;dev.cbSize=sizeof(dev);
        if(!SetupDiEnumDeviceInfo(set,i,&dev)){complete=GetLastError()==259;break;}
        count++;
        if(shown>=6)continue;
        WCHAR name[128];DWORD type=0,bytes=0;
        /* Generic device description, not instance ID, path or user-assigned name. */
        if(SetupDiGetDeviceRegistryPropertyW(set,&dev,0,&type,(BYTE*)name,sizeof(name),&bytes)&&type==REG_SZ&&inventory_text(name,bytes,128)){
            append(L"  ");line(name);shown++;
        }else missing++;
    }
    SetupDiDestroyDeviceInfoList(set);
    append(L"  Present entries observed: ");num(count);append(L"; descriptions shown: ");num(shown);append(L"; unavailable among attempted: ");num(missing);
    line(complete?L" (enumeration ended)":L" (partial: limit/error)");
}
static void inventory_software(HKEY root,DWORD view,LPCWSTR label){
    line(label);HKEY h=0;
    LONG opened=RegOpenKeyExW(root,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",0,KEY_READ|view,&h);
    if(opened){line(opened==2?L"  uninstall key not present":L"  uninstall key unavailable");return;}
    DWORD scanned=0,named=0,shown=0,missing=0;int complete=0;
    for(DWORD i=0;i<512;i++){
        WCHAR key[256];DWORD len=256;
        LONG status=RegEnumKeyExW(h,i,key,&len,0,0,0,0);
        if(status==259){complete=1;break;}
        if(status==234){scanned++;missing++;continue;}
        if(status)break;
        scanned++;if(len>=256){missing++;continue;}key[len]=0;
        HKEY child=0;if(RegOpenKeyExW(h,key,0,KEY_READ|view,&child)){missing++;continue;}
        WCHAR name[128],version[48];
        if(inventory_reg_text(child,L"DisplayName",name,128)){
            named++;
            if(shown<5){append(L"  ");append(name);if(inventory_reg_text(child,L"DisplayVersion",version,48)){append(L" | ");append(version);}nl();shown++;}
        }else missing++;
        RegCloseKey(child);
    }
    RegCloseKey(h);
    append(L"  Registry entries observed: ");num(scanned);append(L"; readable display names: ");num(named);append(L"; unnamed/unreadable: ");num(missing);
    line(complete?L" (enumeration ended)":L" (partial: limit/error)");
}
static void inventory_report(void){
    inventory_system();
    heading(L"PRESENT DEVICE INVENTORY (BOUNDED)");
    inventory_devices(&inventory_disk,L"Disk device descriptions:");
    inventory_devices(&inventory_display,L"Display adapter descriptions:");
    inventory_devices(&inventory_net,L"Network adapter descriptions:");
    line(L"Present device entries can include virtual devices; no link state, media type or driver version inferred.");
    heading(L"REGISTERED DESKTOP SOFTWARE (BOUNDED)");
    inventory_software(HKEY_LOCAL_MACHINE,0x0100,L"Machine 64-bit registry view:");
    inventory_software(HKEY_LOCAL_MACHINE,0x0200,L"Machine 32-bit registry view:");
    inventory_software(HKEY_CURRENT_USER,0,L"Current-user default registry view:");
    line(L"Views may overlap; entries are not unique installed app counts. Store apps, portable software and other users are not enumerated.");
    line(L"Inventory names can be sensitive. Review this local report before sharing.");
}
