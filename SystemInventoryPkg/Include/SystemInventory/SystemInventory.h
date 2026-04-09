#ifndef _SYSTEM_INVENTORY_H_
#define _SYSTEM_INVENTORY_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ShellLib.h>
#include <Protocol/ShellParameters.h>
#include <Protocol/Smbios.h>
#include <Protocol/PciIo.h>
#include <IndustryStandard/SmBios.h>
#include <IndustryStandard/Pci.h>

// Information Retrieval Functions
VOID PrintBiosInformation (VOID);
VOID PrintCpuInformation (VOID);
VOID PrintMemoryInformation (VOID);
VOID PrintPcieDeviceInformation (VOID);


// Input Handling Functions
VOID 
ReadInputLine (
  OUT CHAR16 *Buffer, 
  IN UINTN BufferSize
);

VOID 
ParseInput (
  IN CHAR16 *Input, 
  OUT CHAR16 *Cmd, 
  IN UINTN MaxCmd, 
  OUT CHAR16 *Option, 
  IN UINTN MaxOpt
);

BOOLEAN 
IsValidOptionForm(
  IN CHAR16 *Input, 
  IN CHAR16 *LowerForm, 
  IN CHAR16 *UpperForm, 
  IN CHAR16 *TitleForm
);

#endif