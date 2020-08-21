#include <stdio.h>
#include <windows.h>

int main(){
    HANDLE fp;
    BOOL Chk;
    unsigned char buf[512]={};

    fp = CreateFile("\\\\.\\E:", GENERIC_READ || GENERIC_WRITE, FILE_SHARE_READ || FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (fp == INVALID_HANDLE_VALUE){
        printf("[ERR] ����̺긦 ���� �� �����ϴ�.\n");
        return -1;
    } else {
        printf("[INFO] ����̺긦 ���� �� �ֽ��ϴ�.\n");
    }

    Chk = ReadFile(fp,buf,512,0,0);
    if ((int)Chk == 0){
        printf("[ERR] ���� �ý��� �����͸� ���� �� �����ϴ�.\n");
        return -1;
    } else {
        printf("[INFO] ���� �ý��� �����͸� �о�Խ��ϴ�.\n");
    }

    if ((buf[82]=='F') && (buf[83]=='A') && (buf[84]=='T') && (buf[85]=='3') && (buf[86]=='2')) {
    // File System Type üũ
        if ((buf[510]==0x55) && (buf[511]==0xAA)){
        // Signature üũ
            printf("[INFO] �������� FAT32 ���� �ý����� �½��ϴ�.\n");
        } else {
            printf("[ERR] �������� FAT32 ���� �ý����� �ƴմϴ�.\n");
            return -1;
        }
    } else {
        printf("[ERR] FAT32 ���� �ý����� �ƴմϴ�.\n");
        return -1;
    }

    int ReservedSec;
    ReservedSec = buf[15] * 16 * 16;
    ReservedSec = ReservedSec + buf[14];

    int FatSize;
    FatSize = buf[39] * 16 * 16 * 16 * 16 * 16 * 16;
    FatSize = FatSize + buf[38] * 16 * 16 * 16 * 16;
    FatSize = FatSize + buf[37] * 16 * 16;
    FatSize = FatSize + buf[36];

    int RootDirStr;
    RootDirStr = ReservedSec; // (ReservedSec + BR)
    RootDirStr = RootDirStr + FatSize;
    RootDirStr = RootDirStr + FatSize;
    printf("[INFO] ��Ʈ ���͸� ���� : %d \n",RootDirStr);

    LARGE_INTEGER loc;
    loc.QuadPart = (RootDirStr * 512);
    SetFilePointerEx(fp, loc, 0, 0);
    ReadFile(fp,buf,512,0,0);

    int i, offset, FileSize;
    for(i=0; i<=15; i++){
        offset = i * 32;
        if (buf[offset] == 229){ // ������ �����Ͻ�,
            if (buf[offset+11] != 32){ // �����̸�,
                FileSize = buf[offset+31] * 16 * 16 * 16 * 16 * 16 * 16;
                FileSize = FileSize + buf[offset+30] * 16 * 16 * 16 * 16;
                FileSize = FileSize + buf[offset+29] * 16 * 16;
                FileSize = FileSize + buf[offset+28];

                if (FileSize > 0) {
                    
                }
            }
        }
    }

    // int i;
    // for (i=0; i<=511; i++){
    //     printf("%X ",buf[i]);
    // }

    CloseHandle(fp);
}