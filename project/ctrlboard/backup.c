#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "ite/itp.h"
#include "ite/ug.h"
#include "ctrlboard.h"
#include "scene.h"

#ifdef _WIN32
    #define BACKUP_DRIVE CFG_TEMP_DRIVE
#else
    #define BACKUP_DRIVE CFG_BACKUP_DRIVE
#endif
#define CONFIG_DRIVE    CFG_PUBLIC_DRIVE
#define DEFAULT_DRIVE   CFG_PRIVATE_DRIVE
#define MAX_BACKUP_COUNT 64

#define NEED_RESTORE_FROM_BACKUP    1
#define NEED_RESTORE_DEFAULT        2

typedef struct BACKUP_FILE_PATH
{
    char path[256];
}BACKUP_FILE_PATH;

static BACKUP_FILE_PATH *gtBackupList[MAX_BACKUP_COUNT] = { 0 };
static int              gBackupFileCount = 0;
static int              gBackupDiskType = 0;


static int _GetDriveDiskMode();
/**
 * Used to traverse default drive backup section to generate backup file list.
 *
 * @return none.
 */
static void _TraverseFileList(char* path)
{
    DIR           *dir;
    struct dirent *ent;
    int ret = 0;

    dir = opendir(path);

    if (dir == NULL)
    {
        return;
    }

    //Traverse to get backup list
    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0)
            continue;

        if (strcmp(ent->d_name, "..") == 0)
            continue;

        if (ent->d_type == DT_DIR)
        {
            char dirPath[PATH_MAX];
            char srcPath[PATH_MAX];
            int ret1;

            strcpy(dirPath, path);
            strcat(dirPath, ent->d_name);
            strcat(dirPath, "/");

            _TraverseFileList(dirPath);
        }
        else
        {
            if (ent->d_name[0] != '~')
            {
                char filePath[PATH_MAX];
                strcpy(filePath, path);
                strcat(filePath, ent->d_name);
                gtBackupList[gBackupFileCount] = (BACKUP_FILE_PATH*) malloc(sizeof(BACKUP_FILE_PATH));
                strcpy(gtBackupList[gBackupFileCount]->path, &filePath[strlen(DEFAULT_DRIVE ":/backup/" CONFIG_DRIVE "/")]);
                gBackupFileCount++; 
            }
        }
    }

    if (dir)
    {
        if (closedir(dir))
        {
            printf("close dir(%s) is failed\n", path);
        }
    }
}

/**
 * Used to save files from source to destination partition.
 * @param srcPartition   source partition drive.
 * @param dstPartition   destination partition drive.
 * @return none.
 */
static void _FileSyncFile(char *srcPartition, char *dstPartition)
{
    int           i = 0;
    unsigned char srcFilePath[256] = {0};
    unsigned char dstFilePath[256] = {0};
    ITCFileStream srcStream = {0};
    ITCFileStream dstStream = {0};
    bool          bFlushFile = false;

    for (i = 0; i < gBackupFileCount; i++)
    {
        unsigned char *pSrcDataBuf = NULL;
        unsigned char *pDstDataBuf = NULL;
        bool          bUpdateFile = false;

        sprintf(srcFilePath, "%s:/%s", srcPartition, gtBackupList[i]->path);
        if (itcFileStreamOpen(&srcStream, srcFilePath, false))
        {
            printf("%s(%d): abnormal error\n", __FILE__, __LINE__);
            assert(0);
        }

        sprintf(dstFilePath, "%s:/%s", dstPartition, gtBackupList[i]->path);
        if (itcFileStreamOpen(&dstStream, dstFilePath, false))
        {
            printf("%s(%d): abnormal error\n", __FILE__, __LINE__);
            assert(0);
        }

        if (srcStream.stream.size != dstStream.stream.size)
        {
            bUpdateFile = true;
            bFlushFile = true;
        }
        else
        {
            pSrcDataBuf = (unsigned char *)malloc(srcStream.stream.size);
            if (pSrcDataBuf == NULL)
            {
                printf("%s(%d): abnormal error\n", __FILE__, __LINE__);
                assert(0);
            }
            itcStreamRead(&srcStream, pSrcDataBuf, srcStream.stream.size);

            pDstDataBuf = (unsigned char *)malloc(dstStream.stream.size);
            if (pDstDataBuf == NULL)
            {
                printf("%s(%d): abnormal error\n", __FILE__, __LINE__);
                assert(0);
            }
            itcStreamRead(&dstStream, pDstDataBuf, dstStream.stream.size);

            if (memcmp(pSrcDataBuf, pDstDataBuf, srcStream.stream.size))
            {
                bUpdateFile = true;
                bFlushFile = true;
            }
        }

        if (bUpdateFile)
        {
            printf("sync file: %s, %s\n", srcFilePath, dstFilePath);
            itcFileStreamClose((ITCStream*) &dstStream);

            if (itcFileStreamOpen(&dstStream, dstFilePath, true))
            {
                printf("%s(%d): abnormal error\n", __FILE__, __LINE__);
                assert(0);
            }

            if (pSrcDataBuf == NULL)
            {
                pSrcDataBuf = (unsigned char *)malloc(srcStream.stream.size);
                if (pSrcDataBuf == NULL)
                {
                    printf("%s(%d): abnormal error\n", __FILE__, __LINE__);
                    assert(0);
                }
                itcStreamRead(&srcStream, pSrcDataBuf, srcStream.stream.size);
            }
            itcStreamWrite(&dstStream, pSrcDataBuf, srcStream.stream.size);
        }
        itcFileStreamClose((ITCStream*) &srcStream);
        itcFileStreamClose((ITCStream*) &dstStream);

        if (pSrcDataBuf)
        {
            free(pSrcDataBuf);
        }
        if (pDstDataBuf)
        {
            free(pDstDataBuf);
        }
    }

    if (bFlushFile)
    {
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif
    }
}

/**
 * Used to check the init backup/source partition.
 *
 * @return result of check backup/source partition.
 */
static int _CheckBackupStatus(void)
{
    int i = 0;
    unsigned char   filePath[256] = { 0 };
    ITCFileStream   srcStream = { 0 };
    ITCFileStream   backupStream = { 0 };
    int             result = 0;

    //check drive validation, backup_special_file is used to identify the partition belonging to
    //public or backup partition.
    if (itcFileStreamOpen(&srcStream, CONFIG_DRIVE ":/backup_special_file", false))
    {
        //right public drive
        printf("right public drive volume number\n");
    }
    else
    {
        printf("invalid drive volume number, need to repartition public\n");
        result = NEED_RESTORE_FROM_BACKUP;
    }
    itcFileStreamClose((ITCStream*) &srcStream);
    if (result)
    {
        return result;
    }

    for (i = 0; i < gBackupFileCount; i++)
    {
        sprintf(filePath, "%s:/%s", CONFIG_DRIVE, gtBackupList[i]->path);
        if (itcFileStreamOpen(&srcStream, filePath, false))
        {
            //partition is broken.
            printf("--------------Public partition is crashed------------------\n");
            result = NEED_RESTORE_FROM_BACKUP;
            //usleep(1 * 1000 * 1000);
        }
        itcFileStreamClose((ITCStream*) &srcStream);

        if (result != 0)
        {
            break;
        }

        sprintf(filePath, "%s:/%s", BACKUP_DRIVE, gtBackupList[i]->path);
        if (itcFileStreamOpen(&backupStream, filePath, false))
        {
            //not existed or partition is broken.
            printf("--------------Backup partition is crashed or not existed------------------\n");
            result = NEED_RESTORE_DEFAULT;
            //usleep(1 * 1000 * 1000);
        }
        itcFileStreamClose((ITCStream*) &backupStream);

        if (result != 0)
        {
            break;
        }
    }

    return result;
}

/**
 * Used to reinit the corrupted partition.
 * @param volume   reinit volume number.
 * @return none.
 */
static int _ReinitFatVolume(int volume)
{
    ITPDriveStatus* driveStatusTable;
    ITPDisk         remountDiskType[ITP_MAX_DRIVE];    
    int             remountDiskCount = 0;
    int             diskMode = _GetDriveDiskMode();

    int i = 0, j = 0;
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_DISABLE, NULL);
    usleep(100 * 1000);

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    //Try to find any other mounted device on FAT and remove them to prvent such device
    //been assigned lower volume number.
    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];
        if (driveStatus->disk != diskMode && driveStatus->device == ITP_DEVICE_FAT && driveStatus->avail == 1)
        {
            for (j = 0; j < remountDiskCount; j++)
            {
                if (remountDiskType[j] == driveStatus->disk)
                {
                    break;
                }
            }
            if (j >= remountDiskCount)
            {
                remountDiskType[remountDiskCount] = driveStatus->disk;
                remountDiskCount++;
            }
            printf("disc: %d, device: %d, ava: %d, name: %s\n", driveStatus->disk, driveStatus->device, driveStatus->avail, driveStatus->name);
        }
    }

    printf("unmount all volume except NOR, count: %d\n", remountDiskCount);
    for (j = 0; j < remountDiskCount; j++)
    {
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)remountDiskType[j]);
    }

    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)diskMode);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)diskMode);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)volume);
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_ENABLE, NULL);

    printf("remount all volume except NOR, count: %d\n", remountDiskCount);
    for (j = 0; j < remountDiskCount; j++)
    {
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_MOUNT, (void*)remountDiskType[j]);
    }
}

static int _GetDriveDiskMode()
{
    int i;
    ITPDriveStatus* driveStatusTable;
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];
        if (driveStatus->device == ITP_DEVICE_FAT && driveStatus->avail == 1)
        {
            return driveStatus->disk;
        }
    }
    return ITP_DISK_NOR;
}

/**
 * Used to init backup feature.
 *
 * @return none.
 */
void BackupInit(void)
{
    char VolumeBase = 0;
    char VolumeTarget = 0;

    _TraverseFileList(DEFAULT_DRIVE ":/backup/" CONFIG_DRIVE "/");

#if 0 //print backup file path
    {
        int i = 0;
        for (i = 0; i < gBackupFileCount; i++)
        {
            printf("[%d]: %s\n", i, gtBackupList[i]->path);
        }
    }
#endif

    switch (_CheckBackupStatus())
    {
        case NEED_RESTORE_FROM_BACKUP:
        {
            printf("restore from backup........\n");
            VolumeBase = 0;
            VolumeTarget = 0;
            memcpy(&VolumeBase, DEFAULT_DRIVE, 1);
            memcpy(&VolumeTarget, CONFIG_DRIVE, 1);
            _ReinitFatVolume((int)VolumeTarget - (int)VolumeBase);
            ugRestoreDir(CONFIG_DRIVE ":", DEFAULT_DRIVE ":/backup/" CONFIG_DRIVE);
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
            ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif
            _FileSyncFile(BACKUP_DRIVE, CONFIG_DRIVE);
            break;
        }
        case NEED_RESTORE_DEFAULT:
        {
            ITCFileStream   specialStream = { 0 };
            printf("need restore to default\n");
            VolumeBase = 0;
            VolumeTarget = 0;
            memcpy(&VolumeBase, DEFAULT_DRIVE, 1);
            memcpy(&VolumeTarget, BACKUP_DRIVE, 1);
            _ReinitFatVolume((int)VolumeTarget - (int)VolumeBase);
            ugRestoreDir(BACKUP_DRIVE ":", DEFAULT_DRIVE ":/backup/" CONFIG_DRIVE);
            if (itcFileStreamOpen(&specialStream, BACKUP_DRIVE ":/backup_special_file", true))
            {
                assert(0);
            }
            itcStreamWrite(&specialStream, "1234", sizeof("1234"));
            itcFileStreamClose((ITCStream*) &specialStream);

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
            ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif
            break;
        }
        default:
            printf("noting to do...\n");
            break;
    }
}

/**
 * Used to sync source and backup partition file.
 * @return none.
 */
void BackupSyncFile(void)
{
    printf("backup file check....\n");
    _FileSyncFile(CONFIG_DRIVE, BACKUP_DRIVE);
}

/**
 * Used to restore the backup files to source.
 * @return none.
 */
void BackupRestore(void)
{
    printf("restore backup file to source....\n");
    _FileSyncFile(BACKUP_DRIVE, CONFIG_DRIVE);
}

/**
 * Used to save the source files to backup.
 * @return none.
 */
void BackupSave(void)
{
    printf("backup file save....\n");
    _FileSyncFile(CONFIG_DRIVE, BACKUP_DRIVE);
}

/**
 * Used to destroy backup feature.
 *
 * @return none.
 */
void BackupDestroy(void)
{
    int i = 0;
    printf("destroy backup feature....\n");

    for (i = 0; i < gBackupFileCount; i++)
        if (gtBackupList[i])
            free(gtBackupList[i]);

    gBackupFileCount = 0;
}
