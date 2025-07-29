# Disk Serial Spoofer Driver

A Windows kernel driver that uses kdmapper to spoof physical disk serial numbers by hooking storage device queries.

## ⚠️ WARNING

This tool is for educational and testing purposes only. Use only in controlled environments. Modifying system hardware identifiers may violate terms of service of various software and could be used maliciously. The author is not responsible for any misuse.

## Prerequisites

1. **Windows 10/11** (x64)
2. **Visual Studio 2019 or later** with C++ support
3. **Windows Driver Kit (WDK) 10**
4. **Administrator privileges**
5. **kdmapper.exe** (already included in your workspace)

## Quick Setup

### Option 1: Automated Build (Recommended)

1. **Open Command Prompt as Administrator**
2. **Navigate to the project directory**
3. **Run the build script**:
   ```cmd
   build_and_deploy.bat
   ```

### Option 2: Manual Build

1. **Open Visual Studio Developer Command Prompt as Administrator**

2. **Build the driver**:
   ```cmd
   msbuild disk_spoofer.vcxproj /p:Configuration=Release /p:Platform=x64
   ```

3. **Deploy with kdmapper**:
   ```cmd
   kdmapper.exe x64\Release\disk_spoofer.sys
   ```

## Detailed Instructions

### Step 1: Prepare Your System

1. **Enable Test Signing** (if not already enabled):
   ```cmd
   bcdedit /set testsigning on
   ```
   
2. **Disable Driver Signature Enforcement** (if needed):
   ```cmd
   bcdedit /set nointegritychecks on
   ```
   
3. **Reboot your system** after making these changes

### Step 2: Build the Driver

#### Using Visual Studio:
1. Open `disk_spoofer.vcxproj` in Visual Studio
2. Set configuration to "Release" and platform to "x64"
3. Build the solution (Ctrl+Shift+B)

#### Using Command Line:
```cmd
# Navigate to project directory
cd /path/to/disk_spoofer

# Build using MSBuild
msbuild disk_spoofer.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Step 3: Deploy with kdmapper

1. **Ensure kdmapper.exe is in the project directory**
2. **Run as Administrator**:
   ```cmd
   kdmapper.exe disk_spoofer.sys
   ```

### Step 4: Verify the Spoofing

Test if the serial number has been changed:

```cmd
# Check disk serial numbers
wmic diskdrive get serialnumber

# Or use PowerShell
Get-WmiObject -Class Win32_DiskDrive | Select-Object SerialNumber
```

## How It Works

The driver works by:

1. **Hooking the Disk Driver**: Intercepts `IRP_MJ_DEVICE_CONTROL` requests to the Windows disk driver
2. **Filtering IOCTL Requests**: Specifically targets `IOCTL_STORAGE_QUERY_PROPERTY` requests for `StorageDeviceProperty`
3. **Modifying Responses**: Replaces the original disk serial number with a randomly generated one
4. **Transparent Operation**: All other disk operations remain unaffected

## File Structure

```
disk_spoofer/
├── disk_spoofer.c          # Main driver source code
├── disk_spoofer.h          # Header file with declarations
├── disk_spoofer.vcxproj    # Visual Studio project file
├── sources                 # WDK build configuration
├── makefile               # WDK makefile
├── build_and_deploy.bat   # Automated build script
├── kdmapper.exe          # Driver mapper utility
└── README.md             # This file
```

## Troubleshooting

### Common Issues:

1. **"Access Denied" when running kdmapper**
   - Solution: Run Command Prompt as Administrator

2. **"Unable to load driver" error**
   - Check if test signing is enabled: `bcdedit /enum | findstr testsigning`
   - Ensure driver signature enforcement is disabled
   - Verify no antivirus is blocking the operation

3. **Build errors**
   - Install Visual Studio 2019+ with C++ support
   - Install Windows Driver Kit (WDK) 10
   - Ensure Windows 10 SDK is installed

4. **Driver loads but serial not changed**
   - Check debug output with DebugView
   - Verify the target application is querying the correct disk
   - Some applications may cache serial numbers

### Debug Information

To see debug output:
1. Download **DebugView** from Microsoft Sysinternals
2. Run DebugView as Administrator
3. Enable "Capture Kernel" option
4. Look for `[DiskSpoofer]` messages

## Advanced Usage

### Customizing the Serial Number

Edit `disk_spoofer.c` and modify the `g_SpoofedSerial` variable:

```c
// Change this line to set a custom serial
CHAR g_SpoofedSerial[] = "YOUR_CUSTOM_SERIAL_HERE";
```

### Building for Different Architectures

Currently supports x64 only. For x86 support, modify the project configuration.

## Unloading the Driver

The driver hooks are automatically removed when the system is rebooted. To manually remove:

1. **Reboot the system** (recommended)
2. Or use a driver unloader tool (advanced users only)

## Legal Disclaimer

This software is provided for educational purposes only. Users are responsible for complying with all applicable laws and regulations. The modification of hardware identifiers may:

- Violate software license agreements
- Circumvent anti-cheat systems (which may be against terms of service)
- Be illegal in some jurisdictions

Use responsibly and only in authorized testing environments.

## Technical Details

- **Target OS**: Windows 10/11 x64
- **Driver Type**: WDM (Windows Driver Model)
- **Hook Method**: Direct function pointer replacement
- **Persistence**: Non-persistent (removed on reboot)
- **Stealth**: Minimal footprint, no registry modifications

## Support

This is an educational project. For issues:

1. Check the troubleshooting section
2. Verify your build environment
3. Ensure you're following the instructions exactly
4. Test in a virtual machine first

---

**Remember**: Always test in a safe environment before using on production systems!