#include <stdio.h>
#include <string.h>
#include <windows.h>

int main(int argc, char* argv[]){
	if (argc != 2) {
		printf ("Use : FAT32.exe <DriveName>\n");
        printf ("(ex. FAT32.exe E");
		return 0;
	}
    // 인자 값이 부족하면 사용법을 안내한다.

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
        printf("[ERR] %s 드라이브를 읽을 수 없습니다.\n", argv[1]);
        return -1;
    } else {
        printf("[INFO] %s 드라이브를 읽을 수 있습니다.\n", argv[1]);
    }
    // 파일 핸들을 생성한다.

    Chk = ReadFile(fp,buf,512,0,0);
    if ((int)Chk == 0){
        printf("[ERR] 파일 시스템 데이터를 읽을 수 없습니다.\n");
        return -1;
    } else {
        printf("[INFO] 파일 시스템 데이터를 읽어왔습니다.\n");
    }
    // 파일 시스템의 Boot Record를 읽어온다.

    if ((buf[0]==0xEB) && (buf[1]==0x58) && (buf[2]==0x90)) {
    // CPU Jump Command 체크
        if ((buf[510]==0x55) && (buf[511]==0xAA)){
        // Signature 체크
            printf("[INFO] 정상적인 FAT32 파일 시스템이 맞습니다.\n");
        } else {
            printf("[ERR] 정상적인 FAT32 파일 시스템이 아닙니다.\n");
            return -1;
        }
    } else {
        printf("[ERR] FAT32 파일 시스템이 아닙니다.\n");
        return -1;
    }

    int ClusterPerSector;
    ClusterPerSector = buf[13];
    // 클러스터 당 섹터 값 추출

    int ReservedSec;
    ReservedSec = buf[15] * 16 * 16;
    ReservedSec = ReservedSec + buf[14];
    // 예약 섹터 값 추출

    int FatSize;
    FatSize = buf[39] * 16 * 16 * 16 * 16 * 16 * 16;
    FatSize = FatSize + buf[38] * 16 * 16 * 16 * 16;
    FatSize = FatSize + buf[37] * 16 * 16;
    FatSize = FatSize + buf[36];
    // FAT 크기 추출

    int RootDirStr;
    RootDirStr = ReservedSec; // (ReservedSec + BR)
    RootDirStr = RootDirStr + FatSize;
    RootDirStr = RootDirStr + FatSize;
    printf("[INFO] 루트 디렉터리 섹터 : %d \n",RootDirStr);
    // 루트 디렉터리 섹터 계산

    LARGE_INTEGER loc;
    loc.QuadPart = (RootDirStr * 512);
    SetFilePointerEx(fp, loc, 0, 0);
    ReadFile(fp,buf,512,0,0);
    // 루트 디렉터리 이동

    int i, offset;
    for(i=0; i<=15; i++){
        offset = i * 32;
        if (buf[offset] == 0xE5){
        // 삭제된 파일이면 복구를 진행.
            if (buf[offset+11] == 0x20){
                if (buf[offset-21] != 0x0F){
                    int FileSize;
                    FileSize = buf[offset+31] * 16 * 16 * 16 * 16 * 16 * 16;
                    FileSize = FileSize + buf[offset+30] * 16 * 16 * 16 * 16;
                    FileSize = FileSize + buf[offset+29] * 16 * 16;
                    FileSize = FileSize + buf[offset+28];

                    if (FileSize > 0) {
                        printf ("[FIND] 삭제된 파일을 찾았습니다.\n");
                        printf ("       파일명: _");
                        // 삭제된 파일 이름의 맨 첫 바이트는 Status Byte로 훼손됨. 
                
                        int j;
                        char RecoverFileName[256]={};
                        strcat(RecoverFileName,"[Recover] _");

                        for (j=1; j<=7; j++){
                            if (buf[offset+j] == 0x20) break;
                            RecoverFileName[j+10] = buf[offset+j];
                            printf ("%c",buf[offset+j]);
                        }
                        // 삭제된 파일 이름 추출

                        printf (".");
                        for (j=8; j<=11; j++){
                            if (buf[offset+j] == 0x20) break;
                            printf ("%c",buf[offset+j]);
                        }
                        // 삭제된 파일 확장자 추출

                        printf ("  크기: %d Bytes\n",FileSize);
                        // 삭제된 파일 크기 출력

                        int Cluster;
                        Cluster = buf[offset+21] * 16 * 16 * 16 * 16 * 16 * 16;
                        Cluster = Cluster + buf[offset+20] * 16 * 16 * 16 * 16;
                        Cluster = Cluster + buf[offset+27] * 16 * 16;
                        Cluster = Cluster + buf[offset+26];
                        // 파일의 클러스터 추출

                        loc.QuadPart = ((Cluster - 2) * ClusterPerSector + RootDirStr) * 512;
                        SetFilePointerEx(fp, loc, 0, 0);
                        ReadFile(fp,buf,512,0,0);
                        // 삭제된 파일 위치 이동

                        strcat(RecoverFileName,".Bin");
                        FILE *output = fopen(RecoverFileName,"wb");
                        for (j=0; j<FileSize; j++)
                        {
                            fputc(buf[j], output);
                        }
                        printf("[RECOVER] 삭제된 파일을 복구했습니다. (%s)\n",RecoverFileName);
                        // 삭제된 파일 덤프

                        loc.QuadPart = (RootDirStr * 512);
                        SetFilePointerEx(fp, loc, 0, 0);
                        ReadFile(fp,buf,512,0,0);
                        // 파일 포인터 복구
                    }
                }
            }
        }
    }

    CloseHandle(fp);
    // 파일 핸들을 닫음
    return 0;
}