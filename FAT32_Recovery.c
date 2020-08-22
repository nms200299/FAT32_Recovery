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

    int ClusterPerSector;
    ClusterPerSector = buf[13];

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

    int i, offset;
    for(i=0; i<=15; i++){
        offset = i * 32;
        if (buf[offset] == 0xE5){ // ������ �����Ͻ�,
            if (buf[offset+11] == 0x20){
                if (buf[offset-21] != 0x0F){
                    int FileSize;
                    FileSize = buf[offset+31] * 16 * 16 * 16 * 16 * 16 * 16;
                    FileSize = FileSize + buf[offset+30] * 16 * 16 * 16 * 16;
                    FileSize = FileSize + buf[offset+29] * 16 * 16;
                    FileSize = FileSize + buf[offset+28];

                    if (FileSize > 0) {
                        printf ("[FIND] ������ ������ ã�ҽ��ϴ�.\n");
                        printf ("       ���ϸ�: _");

                        int j;
                        for (j=1; j<=7; j++){
                            if (buf[offset+j] == 0x20) break;
                            printf ("%c",buf[offset+j]);
                        } // ������ ���� �̸� ���
                        printf (".");
                        for (j=8; j<=11; j++){
                            if (buf[offset+j] == 0x20) break;
                            printf ("%c",buf[offset+j]);
                        } // ������ ���� Ȯ���� ���
                        printf ("  ũ��: %d Bytes",FileSize);
                        // ������ ���� ũ�� ���

                        int Cluster;
                        Cluster = buf[offset+21] * 16 * 16 * 16 * 16 * 16 * 16;
                        Cluster = Cluster + buf[offset+20] * 16 * 16 * 16 * 16;
                        Cluster = Cluster + buf[offset+27] * 16 * 16;
                        Cluster = Cluster + buf[offset+26];

                        loc.QuadPart = ((Cluster - 2) * ClusterPerSector + RootDirStr) * 512;
                        printf(" %d \n",((Cluster - 2) * ClusterPerSector + RootDirStr) * 512);
                        SetFilePointerEx(fp, loc, 0, 0);
                        ReadFile(fp,buf,512,0,0);

    int x;
    for (x=0; x<=511; x++){
        printf("%X ",buf[x]);
    }
                    }
                }
            }
        }
    }



    CloseHandle(fp);
}