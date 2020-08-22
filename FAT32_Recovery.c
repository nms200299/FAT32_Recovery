#include <stdio.h>
#include <string.h>
#include <windows.h>

int main(int argc, char* argv[]){
	if (argc != 2) {
		printf ("Use : FAT32.exe <DriveName>\n");
        printf ("(ex. FAT32.exe E");
		return 0;
	}
    // ���� ���� �����ϸ� ������ �ȳ��Ѵ�.

    HANDLE fp;
    BOOL Chk;
    unsigned char buf[512]={};
    char DrivePath[7]={};
    strcat(DrivePath,"\\\\.\\");
    strcat(DrivePath,argv[1]);
    strcat(DrivePath,":");

    fp = CreateFile(DrivePath, GENERIC_READ || GENERIC_WRITE,
    FILE_SHARE_READ || FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

    if (fp == INVALID_HANDLE_VALUE){
        printf("[ERR] %s ����̺긦 ���� �� �����ϴ�.\n", argv[1]);
        return -1;
    } else {
        printf("[INFO] %s ����̺긦 ���� �� �ֽ��ϴ�.\n", argv[1]);
    }
    // ���� �ڵ��� �����Ѵ�.

    Chk = ReadFile(fp,buf,512,0,0);
    if ((int)Chk == 0){
        printf("[ERR] ���� �ý��� �����͸� ���� �� �����ϴ�.\n");
        return -1;
    } else {
        printf("[INFO] ���� �ý��� �����͸� �о�Խ��ϴ�.\n");
    }
    // ���� �ý����� Boot Record�� �о�´�.

    if ((buf[0]==0xEB) && (buf[1]==0x58) && (buf[2]==0x90)) {
    // CPU Jump Command üũ
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
    // Ŭ������ �� ���� �� ����

    int ReservedSec;
    ReservedSec = buf[15] * 16 * 16;
    ReservedSec = ReservedSec + buf[14];
    // ���� ���� �� ����

    int FatSize;
    FatSize = buf[39] * 16 * 16 * 16 * 16 * 16 * 16;
    FatSize = FatSize + buf[38] * 16 * 16 * 16 * 16;
    FatSize = FatSize + buf[37] * 16 * 16;
    FatSize = FatSize + buf[36];
    // FAT ũ�� ����

    int RootDirStr;
    RootDirStr = ReservedSec; // (ReservedSec + BR)
    RootDirStr = RootDirStr + FatSize;
    RootDirStr = RootDirStr + FatSize;
    printf("[INFO] ��Ʈ ���͸� ���� : %d \n",RootDirStr);
    // ��Ʈ ���͸� ���� ���

    LARGE_INTEGER loc;
    loc.QuadPart = (RootDirStr * 512);
    SetFilePointerEx(fp, loc, 0, 0);
    ReadFile(fp,buf,512,0,0);
    // ��Ʈ ���͸� �̵�

    int i, offset;
    for(i=0; i<=15; i++){
        offset = i * 32;
        if (buf[offset] == 0xE5){
        // ������ �����̸� ������ ����.
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
                        // ������ ���� �̸��� �� ù ����Ʈ�� Status Byte�� �Ѽյ�. 
                
                        int j;
                        char RecoverFileName[256]={};
                        strcat(RecoverFileName,"[Recover] _");

                        for (j=1; j<=7; j++){
                            if (buf[offset+j] == 0x20) break;
                            RecoverFileName[j+10] = buf[offset+j];
                            printf ("%c",buf[offset+j]);
                        }
                        // ������ ���� �̸� ����

                        printf (".");
                        for (j=8; j<=11; j++){
                            if (buf[offset+j] == 0x20) break;
                            printf ("%c",buf[offset+j]);
                        }
                        // ������ ���� Ȯ���� ����

                        printf ("  ũ��: %d Bytes\n",FileSize);
                        // ������ ���� ũ�� ���

                        int Cluster;
                        Cluster = buf[offset+21] * 16 * 16 * 16 * 16 * 16 * 16;
                        Cluster = Cluster + buf[offset+20] * 16 * 16 * 16 * 16;
                        Cluster = Cluster + buf[offset+27] * 16 * 16;
                        Cluster = Cluster + buf[offset+26];
                        // ������ Ŭ������ ����

                        loc.QuadPart = ((Cluster - 2) * ClusterPerSector + RootDirStr) * 512;
                        SetFilePointerEx(fp, loc, 0, 0);
                        ReadFile(fp,buf,512,0,0);
                        // ������ ���� ��ġ �̵�

                        strcat(RecoverFileName,".Bin");
                        FILE *output = fopen(RecoverFileName,"wb");
                        for (j=0; j<FileSize; j++)
                        {
                            fputc(buf[j], output);
                        }
                        printf("[RECOVER] ������ ������ �����߽��ϴ�. (%s)\n",RecoverFileName);
                        // ������ ���� ����

                        loc.QuadPart = (RootDirStr * 512);
                        SetFilePointerEx(fp, loc, 0, 0);
                        ReadFile(fp,buf,512,0,0);
                        // ���� ������ ����
                    }
                }
            }
        }
    }

    CloseHandle(fp);
    // ���� �ڵ��� ����
    return 0;
}