#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "ite/ug.h"
#include "ite/itc.h"

#ifdef __cplusplus
extern "C"
{
#endif

// logo
void ShowLogo(void);

//boot video
void PlayVideo(void);
void WaitPlayVideoFinish(void);

// upgrade
ITCStream* OpenUpgradePackage(void);
void DeleteUpgradePackage(void);
ITCStream* OpenRecoveryPackage(void);
void DoUsbDeviceCommands(void);
int CopyUclFile(void);
void ShowUpgradeFail(void);
void ShowLastUpgradeFail(void);

// reset
int ResetFactory(void);

// restore
void RestorePackage(void);

// boot
void* LoadImage(void* arg);
void ReleaseImage(void);
void BootImage(void);

void BootBin(ITCStream *upgradeFile);

//boot test bin
ITCStream* OpenTestBin(void);
void BootTestBin(ITCStream *upgradeFile);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_H
