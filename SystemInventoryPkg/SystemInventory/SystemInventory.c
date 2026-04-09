#include <SystemInventory/SystemInventory.h>

STATIC EFI_SMBIOS_PROTOCOL *mSmbios = NULL;

STATIC
CHAR8*
GetSmbiosString (
  IN EFI_SMBIOS_TABLE_HEADER *Record,
  IN UINT8                   StringIndex
  )
{
  CHAR8 *StringArea;
  UINTN  Index;

  // If index is 0, the field is undefined/empty in SMBIOS
  if (StringIndex == 0) {
    return (CHAR8*)"N/A";
  }

  // Strings start after the formatted portion of the table
  StringArea = (CHAR8 *)Record + Record->Length;

  // SMBIOS strings are a list of null-terminated strings. 
  // We must walk them one by one.
  for (Index = 1; Index < StringIndex; Index++) {
    while (*StringArea != 0) {
      StringArea++;
    }
    StringArea++; // Skip the null terminator to reach the next string
  }

  // Final check: if the first byte is null, the string is empty
  if (*StringArea == 0) {
    return (CHAR8*)"N/A";
  }

  return StringArea;
}

// Translates a raw SMBIOS numeric type ID into string
STATIC CHAR16* GetTypeName(UINT8 Value)
{
  switch (Value) {
    case 0:  return L"BIOS Information";
    case 4:  return L"Processor Information";
    case 17: return L"Memory Device";
    default: return L"Unknown";
  }
}

// Locates the SMBIOS Type 0 table to display bios information
VOID
PrintBiosInformation(VOID)
{
    EFI_STATUS Status;
    EFI_SMBIOS_HANDLE Handle;
    EFI_SMBIOS_TYPE Type;
    EFI_SMBIOS_TABLE_HEADER *Record;
    SMBIOS_TABLE_TYPE0      *Bios;

    if (mSmbios == NULL) {
        Print(L"SMBIOS not available.\n");
        return;
    }

    Handle = SMBIOS_HANDLE_PI_RESERVED;
    Type   = SMBIOS_TYPE_BIOS_INFORMATION;

    Status = mSmbios->GetNext(
        mSmbios,
        &Handle,
        &Type,
        &Record,
        NULL
    );

    if (EFI_ERROR(Status)) {
        Print(L"BIOS Information not available\n");
        return;
    }

    Bios = (SMBIOS_TABLE_TYPE0 *)Record;
    
    Print(L"\n--------BIOS INFORMATION--------\n");

    Print(L"Type             : %s\n", GetTypeName(Record->Type));
    Print(L"Length           : %d\n", Record->Length);
    Print(L"Handle           : %u\n", Record->Handle);
    Print(L"Vendor           : %a\n", GetSmbiosString(Record, Bios->Vendor));
    Print(L"BIOS Version     : %a\n", GetSmbiosString(Record, Bios->BiosVersion));
    Print(L"Release Date     : %a\n", GetSmbiosString(Record, Bios->BiosReleaseDate));
    Print(L"ROM Size         : %d KB\n", (Bios->BiosSize + 1) * 64);
    Print(L"BIOS Characteristics         : 0x%lx\n", Bios->BiosCharacteristics);
    Print(L"System BIOS Major Release    : %d\n", Bios->SystemBiosMajorRelease);
    Print(L"System BIOS Minor Release    : %d\n",Bios->SystemBiosMinorRelease);
    Print(L"Embedded Controller Firmware Major Release : %d\n",Bios->EmbeddedControllerFirmwareMajorRelease);
    Print(L"Embedded Controller Firmware Minor         : %d\n\n",Bios->EmbeddedControllerFirmwareMinorRelease);
}

// Maps the SMBIOS processor type byte to strings
STATIC
CHAR16*
GetProcessorTypeName(UINT8 Value)
{
    switch (Value) {
        case 1: return L"Other";
        case 2: return L"Unknown";
        case 3: return L"Central Processor";
        case 4: return L"Math Processor";
        case 5: return L"DSP Processor";
        case 6: return L"Video Processor";
        default: return L"Unknown";
    }
}

// Converts the family hex code into architectural names
STATIC
CHAR16*
GetProcessorFamilyName(UINT16 Value)
{
    switch (Value) {
        case 1:   return L"Other";
        case 2:   return L"Unknown";
        case 3:   return L"8086";
        case 4:   return L"80286";
        case 5:   return L"80386";
        case 6:   return L"80486";
        case 0x0B:return L"Pentium";
        case 0x0C:return L"Pentium Pro";
        case 0x0D:return L"Pentium II";
        case 0x0E:return L"Pentium MMX";
        case 0x0F:return L"Celeron";
        case 0x10:return L"Pentium II Xeon";
        case 0x11:return L"Pentium III";
        case 0x12:return L"M1";
        case 0x13:return L"M2";
        case 0xB3:return L"Intel Core i5";
        case 0xB4:return L"Intel Core i7";
        case 0xB5:return L"Intel Core i3";
        case 0xBE:return L"AMD Ryzen 5";
        case 0xBF:return L"AMD Ryzen 7";
        case 0xC0:return L"AMD Ryzen 3";
        case 254: return L"Use ProcessorFamily2";
        default:  return L"Unknown";
    }
}

// Locates the SMBIOS Type 4 to print processor information
VOID PrintCpuInformation(VOID)
{
    EFI_STATUS Status;
    EFI_SMBIOS_HANDLE Handle;
    EFI_SMBIOS_TYPE Type;
    EFI_SMBIOS_TABLE_HEADER *Record;

    if (mSmbios == NULL) {
        Print(L"SMBIOS not available.\n");
        return;
    }

    Handle = SMBIOS_HANDLE_PI_RESERVED;
    Type   = SMBIOS_TYPE_PROCESSOR_INFORMATION; // Type 4

    Print(L"\n--------CPU INFORMATION--------\n");

    // There can be multiple CPU records (CPU0, CPU1...)
    while (TRUE) {

        Status = mSmbios->GetNext(mSmbios, &Handle, &Type, &Record, NULL);

        if (EFI_ERROR(Status) || Record == NULL) {
            break;
        }

        SMBIOS_TABLE_TYPE4 *Cpu = (SMBIOS_TABLE_TYPE4 *)Record;
        CHAR8 *Str = (CHAR8 *)Record + Record->Length;

        UINT8 i;

        // ---- Socket String ----
        CHAR8 *SocketStr = Str;
        if (Cpu->Socket == 0) {
            SocketStr = (CHAR8 *)"N/A";
        } else {
            for (i = 1; i < Cpu->Socket; i++) {
                while (*SocketStr != 0) SocketStr++;
                SocketStr++;
            }
            if (*SocketStr == 0) SocketStr = (CHAR8 *)"N/A";
        }

        // ---- Manufacturer String ----
        CHAR8 *ManufStr = Str;
        if (Cpu->ProcessorManufacturer == 0) {
            ManufStr = (CHAR8 *)"N/A";
        } else {
            for (i = 1; i < Cpu->ProcessorManufacturer; i++) {
                while (*ManufStr != 0) ManufStr++;
                ManufStr++;
            }
            if (*ManufStr == 0) ManufStr = (CHAR8 *)"N/A";
        }

        // ---- Version String (CPU model name) ----
        CHAR8 *VersionStr = Str;
        if (Cpu->ProcessorVersion == 0) {
            VersionStr = (CHAR8 *)"N/A";
        } else {
            for (i = 1; i < Cpu->ProcessorVersion; i++) {
                while (*VersionStr != 0) VersionStr++;
                VersionStr++;
            }
            if (*VersionStr == 0) VersionStr = (CHAR8 *)"N/A";
        }

        Print(L"Type                  : %s\n", GetTypeName(Record->Type));
        Print(L"Length                : %d\n", Record->Length);
        Print(L"Handle                : %u\n", Record->Handle);
        Print(L"Socket Name           : %a\n", GetSmbiosString(Record, Cpu->Socket));
        Print(L"Processor Type        : %s\n", GetProcessorTypeName(Cpu->ProcessorType));

        if (Cpu->ProcessorFamily == 254) {
            Print(L"Processor Family  : %s (%u)\n",
                GetProcessorFamilyName(Cpu->ProcessorFamily),
                Cpu->ProcessorFamily);

            Print(L"Processor Family2 : %s (%u)\n",
                GetProcessorFamilyName(Cpu->ProcessorFamily2),
                Cpu->ProcessorFamily2);
        } else {
            Print(L"Processor Family  : %s (%u)\n",
                GetProcessorFamilyName(Cpu->ProcessorFamily),
                Cpu->ProcessorFamily);
        }

        Print(L"Processor Manufacturer: %a\n", GetSmbiosString(Record, Cpu->ProcessorManufacturer));

        UINT8 *Pid = (UINT8 *)&Cpu->ProcessorId;

        Print(L"Processor ID (8 bytes): ");
        for (UINTN k = 0; k < 8; k++) {
            Print(L"%02x ", Pid[k]);
        }
        Print(L"\n");

        Print(L"Processor Version     : %a\n", GetSmbiosString(Record, Cpu->ProcessorVersion));
        Print(L"External Clock        : %u\n", Cpu->ExternalClock);
        Print(L"Max Speed             : %u\n", Cpu->MaxSpeed);
        Print(L"Current Speed         : %u\n", Cpu->CurrentSpeed);
        Print(L"Status                : 0x%02x\n", Cpu->Status);
        Print(L"L1 Cache Handle       : 0x%X\n", Cpu->L1CacheHandle);
        Print(L"L2 Cache Handle       : 0x%X\n", Cpu->L2CacheHandle);
        Print(L"L3 Cache Handle       : 0x%X\n", Cpu->L3CacheHandle);
        Print(L"Core Count            : %u\n", Cpu->CoreCount);
        Print(L"Enabled Core Count    : %u\n", Cpu->EnabledCoreCount);
        Print(L"Thread Count          : %u\n", Cpu->ThreadCount);
        Print(L"Processor Characteristics: %u\n\n", Cpu->ProcessorCharacteristics);
    }
}

// Identifies the physical shape of the memory
STATIC CHAR16* GetFormFactorName(UINT8 Value)
{
  switch (Value) {
    case 9:  return L"DIMM";
    case 10: return L"TSOP";
    case 13: return L"SODIMM";
    case 14: return L"SRIMM";
    case 15: return L"FB-DIMM";
    default: return L"Unknown";
  }
}

// Translates the memory type into string
STATIC
CHAR16*
GetMemoryTypeName(UINT8 Value)
{
    switch (Value) {
        case 1:  return L"Other";
        case 2:  return L"Unknown";
        case 3:  return L"DRAM";
        case 4:  return L"EDRAM";
        case 5:  return L"VRAM";
        case 6:  return L"SRAM";
        case 7:  return L"RAM";
        case 8:  return L"ROM";
        case 9:  return L"FLASH";
        case 10: return L"EEPROM";
        case 11: return L"FEPROM";
        case 12: return L"EPROM";
        case 13: return L"CDRAM";
        case 14: return L"3DRAM";
        case 15: return L"SDRAM";
        case 16: return L"SGRAM";
        case 17: return L"RDRAM";
        case 18: return L"DDR";
        case 19: return L"DDR2";
        case 20: return L"DDR2 FB-DIMM";
        case 21: return L"Reserved";
        case 22: return L"Reserved";
        case 23: return L"Reserved";
        case 24: return L"DDR3";
        case 25: return L"DDR4";
        case 26: return L"LPDDR";
        case 27: return L"LPDDR2";
        case 28: return L"LPDDR3";
        case 29: return L"LPDDR4";
        case 30: return L"Logical non-volatile device";
        case 31: return L"HBM (High Bandwidth Memory)";
        case 32: return L"HBM2 (High Bandwidth Memory Generation 2)";
        case 33: return L"DDR5";
        case 34: return L"LPDDR5";
        default: return L"Unknown";
    }
}

// Locates the SMBIOS Type 17 table to display memory device information
VOID PrintMemoryInformation(VOID){

    EFI_STATUS Status;
    EFI_SMBIOS_HANDLE Handle;
    EFI_SMBIOS_TYPE Type;
    EFI_SMBIOS_TABLE_HEADER *Record;

    if (mSmbios == NULL) {
        Print(L"SMBIOS not available.\n");
        return;
    }

    Handle=SMBIOS_HANDLE_PI_RESERVED;
    Type= SMBIOS_TYPE_MEMORY_DEVICE;

    Print(L"--------MEMORY INFORMATION--------\n");

    while (TRUE) {
        Status = mSmbios->GetNext(mSmbios, &Handle, &Type, &Record, NULL);
        if (EFI_ERROR(Status) || Record == NULL) {
            break;
        }

        SMBIOS_TABLE_TYPE17 *Mem = (SMBIOS_TABLE_TYPE17 *)Record;

        Print(L"Type            : %s\n", GetTypeName(Record->Type));
        Print(L"Length          : %d\n", Record->Length);
        Print(L"Handle          : %u\n", Record->Handle);
        Print(L"Physical Memory Array Handle   : 0x%04x\n", Mem->MemoryArrayHandle);
        Print(L"Memory Error Information Handle: 0x%04x\n", Mem->MemoryErrorInformationHandle);
        Print(L"Total Width     : 0x%04x\n", Mem->TotalWidth);
        Print(L"Data Width      : 0x%04x\n", Mem->DataWidth);
        Print(L"Size            : %u\n", Mem->Size);
        Print(L"Form Factor     : %s\n", GetFormFactorName(Mem->FormFactor));
        Print(L"Device Set      : 0x%x\n", Mem->DeviceSet);
        Print(L"Device Locator  : %a\n", GetSmbiosString(Record, Mem->DeviceLocator));
        Print(L"Bank Locator    : %a\n", GetSmbiosString(Record, Mem->BankLocator));
        Print(L"Manufacturer    : %a\n", GetSmbiosString(Record, Mem->Manufacturer));
        Print(L"Serial Number   : %a\n", GetSmbiosString(Record, Mem->SerialNumber));
        Print(L"Memory Type     : %s\n\n", GetMemoryTypeName(Mem->MemoryType));
        Print(L"Type Detail     : %u\n", Mem->TypeDetail);
        Print(L"Speed           : 0x%x\n", Mem->Speed);

        if (Mem->AssetTag == 0) {
            Print(L"Asset Tag   : <Null String>\n");
        } else {
            Print(L"Asset Tag   : %u\n", Mem->AssetTag);
        }

        if (Mem->PartNumber == 0) {
            Print(L"Part Number : <Null String>\n");
        } else {
            Print(L"Part Number : %u\n", Mem->PartNumber);
        }

        Print(L"Attributes      : 0x%x\n", Mem->Attributes);
        Print(L"Extended Size   : %u\n", Mem->ExtendedSize);
        Print(L"Configured Memory Speed : %u\n", Mem->ConfiguredMemoryClockSpeed);
        Print(L"Minimum Voltage    : %u\n", Mem->MinimumVoltage);
        Print(L"Maximum Voltage    : %u\n", Mem->MaximumVoltage);
        Print(L"Configured Voltage : %u\n", Mem->ConfiguredVoltage);

    }

}

// Maps the PCI Base Class code to a device category 
STATIC
CHAR16 *
GetPciDeviceType (
  UINT8 BaseClass
  )
{
  switch (BaseClass) {
    case 0x01: return L"Mass Storage Controller";
    case 0x02: return L"Network Controller";
    case 0x03: return L"Display Controller";
    case 0x04: return L"Multimedia Controller";
    case 0x05: return L"Memory Controller";
    case 0x06: return L"Bridge device";
    case 0x07: return L"Simple Communication Controller";
    case 0x08: return L"Base System Peripheral";
    case 0x09: return L"Input Device Controller";
    case 0x0A: return L"Docking Station";
    case 0x0B: return L"Processor";
    case 0x0C: return L"Serial Bus Controller";
    case 0x0D: return L"Wireless Controller";
    case 0x0E: return L"Intelligent I/O Controller";
    case 0x0F: return L"Satellite Communication Controller";
    case 0x10: return L"Encryption Controller";
    case 0x11: return L"Data acquisition and signal processing Controller";
    default:   return L"Unknown device";
  }
}

/*Uses the EFI_PCI_IO_PROTOCOL to scan 
    the PCI bus and list the Segment/Bus/Device/Function (SBDF) address, 
    Vendor ID, and Device ID for all detected hardware.*/
VOID
PrintPcieDeviceInformation (
  VOID
  )
{
  EFI_STATUS           Status;
  EFI_HANDLE           *HandleBuffer;
  UINTN                HandleCount;
  UINTN                Index;
  UINTN                DeviceCount;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  PCI_TYPE00           Pci;
  UINTN                Segment;
  UINTN                Bus;
  UINTN                Device;
  UINTN                Function;
  UINT8                BaseClass;

  HandleBuffer = NULL;
  HandleCount  = 0;
  DeviceCount  = 0;

  Print(L"\n--------PCIE DEVICE INFORMATION--------\n\n");

  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  if (EFI_ERROR(Status)) {
    Print(L"Failed to locate PCI IO handles: %r\n", Status);
    return;
  }

  for (Index = 0; Index < HandleCount; Index++) {

    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    Status = PciIo->GetLocation(
                      PciIo,
                      &Segment,
                      &Bus,
                      &Device,
                      &Function
                      );
    if (EFI_ERROR(Status)) {
      continue;
    }

    Status = PciIo->Pci.Read(
                           PciIo,
                           EfiPciIoWidthUint32,
                           0,
                           sizeof(Pci) / sizeof(UINT32),
                           &Pci
                           );
    if (EFI_ERROR(Status)) {
      continue;
    }

    if (Pci.Hdr.VendorId == 0xFFFF) {
      continue;
    }

    BaseClass = Pci.Hdr.ClassCode[2];

    DeviceCount++;

    Print(L"Pcie Device: %d\n", DeviceCount);
    Print(L"Seg/Bus/Dev/Func %d %d %d %d\n", Segment, Bus, Device, Function);
    Print(L"VendorID: %04x\n", Pci.Hdr.VendorId);
    Print(L"DeviceID: %04x\n", Pci.Hdr.DeviceId);
    Print(L"Device Type: %s\n\n", GetPciDeviceType(BaseClass));
  }

  if (HandleBuffer != NULL) {
    FreePool(HandleBuffer);
  }
}

/*Captures user keystrokes from the UEFI console, 
    handles backspaces, and stores the resulting string for command processing*/
VOID
ReadInputLine (
    OUT CHAR16 *Buffer,
    IN  UINTN   BufferSize
    )
{
    EFI_INPUT_KEY Key;
    UINTN Index = 0;

    if (Buffer == NULL || BufferSize == 0) return;

    while (TRUE) {
        while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) != EFI_SUCCESS);

        if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            Print(L"\n");
            break;
        }

        if (Key.UnicodeChar == CHAR_BACKSPACE) {
            if (Index > 0) {
                Index--;
                Print(L"\b \b");
            }
            continue;
        }

        if (Key.UnicodeChar >= 0x20 && Index < BufferSize - 1) {
            Buffer[Index++] = Key.UnicodeChar;
            Print(L"%c", Key.UnicodeChar);
        }
    }

    Buffer[Index] = L'\0';
}

/*comparing the user input against lower, upper, and title-case versions of a command*/
BOOLEAN
IsValidOptionForm (
    IN CHAR16 *Input,
    IN CHAR16 *LowerForm,
    IN CHAR16 *UpperForm,
    IN CHAR16 *TitleForm
    )
{
    if (StrCmp(Input, LowerForm) == 0) {
        return TRUE;
    }

    if (StrCmp(Input, UpperForm) == 0) {
        return TRUE;
    }

    if (StrCmp(Input, TitleForm) == 0) {
        return TRUE;
    }

    return FALSE;
}

/*Splits a raw input string into two distinct parts: 
    the primary command and an optional parameter*/
VOID
ParseInput (
    IN  CHAR16 *Input,
    OUT CHAR16 *Cmd,
    IN  UINTN   MaxCmd,
    OUT CHAR16 *Option,
    IN  UINTN   MaxOpt
    )
{
    UINTN i = 0, j = 0;

    while (Input[i] == L' ') i++;

    // Read Command with bounds check
    while (Input[i] != L' ' && Input[i] != L'\0' && j < MaxCmd - 1) {
        Cmd[j++] = Input[i++];
    }
    Cmd[j] = L'\0';

    while (Input[i] != L' ' && Input[i] != L'\0') i++; // Skip if input exceeded MaxCmd
    while (Input[i] == L' ') i++;

    // Read Option with bounds check
    j = 0;
    while (Input[i] != L' ' && Input[i] != L'\0' && j < MaxOpt - 1) {
        Option[j++] = Input[i++];
    }
    Option[j] = L'\0';
}

/*The application entry point; 
    it initializes the shell, locates the SMBIOS protocol, 
    and runs the main loop that waits for user commands to trigger the print functions above*/
EFI_STATUS
EFIAPI
UefiMain (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_SHELL_PARAMETERS_PROTOCOL *ShellParams;
    CHAR16 Input[50];
    CHAR16 Cmd[20];
    CHAR16 Opt[10];

    Status = ShellInitialize();
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **) &mSmbios);
    if (EFI_ERROR(Status)) {
        Print(L"Warning: SMBIOS Protocol not found. System info will be unavailable.\n");
        mSmbios = NULL; // Ensure it's NULL so functions can check it
    }

    Status = gBS->OpenProtocol(
        ImageHandle,
        &gEfiShellParametersProtocolGuid,
        (VOID **)&ShellParams,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );


    if (EFI_ERROR(Status)) {
        Print(L"Not running from UEFI Shell\n");
        return Status;
    }


    while (TRUE) {
        Print(L"\n-------- System Inventory --------\n");
        Print(L"Available commands:\n");
        Print(L"       -bios | -Bios | -BIOS  -  Displays BIOS information\n");
        Print(L"       -cpu | -Cpu | -CPU     -  Displays Processor information\n");
        Print(L"       -dimm | -Dimm | - DIMM -  Displays Memory Device information\n");
        Print(L"       -pcie | -Pcie | -PCIE  -  Displays PCIe Devices information\n");
        Print(L"       -all | -All | -ALL     -  Displays above all information\n");
        Print(L"       -exit | -Exit | -EXIT  -  Terminates the application and returns to the UEFI Shell\n");
        Print(L"\nOptional Parameter:\n");
        Print(L"       -b       Enable page break for long data outputs (Usage Example: -all -b)\n");
        Print(L"\nEnter your choice: ");

        ReadInputLine(Input, sizeof(Input) / sizeof(CHAR16));

        // split input
        ParseInput(Input, Cmd, 20, Opt, 10);

        if (StrCmp(Opt, L"-b") == 0 || StrCmp(Opt, L"-B") == 0) {
            ShellSetPageBreakMode(TRUE); // Manually turn on Paging for this command
        } else {
            ShellSetPageBreakMode(FALSE); // Turn it off if -b is NOT typed
        }

        // BOOLEAN CommandExecuted = FALSE;

        if (IsValidOptionForm(Cmd, L"-bios", L"-BIOS", L"-Bios")) {
            PrintBiosInformation();
        }
        else if (IsValidOptionForm(Cmd, L"-cpu", L"-CPU", L"-Cpu")) {
            PrintCpuInformation();
        }
        else if (IsValidOptionForm(Cmd, L"-dimm", L"-DIMM", L"-Dimm")) {
            PrintMemoryInformation();
        }
        else if (IsValidOptionForm(Cmd, L"-pcie", L"-PCIE", L"-Pcie")) {
            PrintPcieDeviceInformation();
        }
        else if (IsValidOptionForm(Cmd, L"-all", L"-ALL", L"-All")) {
            PrintBiosInformation();
            PrintCpuInformation();
            PrintMemoryInformation();
            PrintPcieDeviceInformation();
        }
        else if (IsValidOptionForm(Cmd, L"-exit", L"-EXIT", L"-Exit")) {
            Print(L"Exiting application...\n");
            return EFI_SUCCESS;
        }
        else {
            Print(L"Invalid option.\n");
        }
    }
    return EFI_SUCCESS;
}