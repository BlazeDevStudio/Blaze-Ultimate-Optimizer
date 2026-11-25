#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t DWORD;typedef unsigned char BYTE;typedef wchar_t WCHAR;
#include "../src/inventory_helpers.inc"
int main(void){
    assert(sizeof(WCHAR)==2);
    WCHAR ok[]=L"Device";assert(inventory_text(ok,sizeof(ok),7));
    WCHAR ctrl[]=L"X\r\nY";assert(inventory_text(ctrl,sizeof(ctrl),5)&&ctrl[1]==L' '&&ctrl[2]==L' ');
    WCHAR bidi[]={L'X',0x202e,L'Y',0};assert(inventory_text(bidi,sizeof(bidi),4)&&bidi[1]==L' ');
    WCHAR bad[]={L'X',L'Y'};assert(!inventory_text(bad,sizeof(bad),2));
    assert(!inventory_text(ok,3,7)&&!inventory_text(ok,sizeof(ok),2)&&!inventory_text(ok,0,7));
    WCHAR empty[]={0};assert(!inventory_text(empty,sizeof(empty),1));
    BYTE b[96]={0};DWORD n=99;b[4]=48;b[30]=1;b[52]=48;b[78]=1;
    assert(inventory_core_count(b,96,&n)&&n==2);
    assert(inventory_core_count(b,48,&n)&&n==1);
    assert(!inventory_core_count(b,95,&n)&&n==0);
    assert(!inventory_core_count(b,0,&n));
    b[0]=1;assert(!inventory_core_count(b,96,&n));b[0]=0;
    b[30]=2;assert(!inventory_core_count(b,96,&n));b[30]=1;
    b[4]=0;assert(!inventory_core_count(b,96,&n));b[4]=48;
    b[4]=49;assert(!inventory_core_count(b,96,&n));
    puts("PASS: 14 inventory parser boundary groups");return 0;
}
