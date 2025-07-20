[![bugsplat-github-banner-basic-outline](https://user-images.githubusercontent.com/20464226/149019306-3186103c-5315-4dad-a499-4fd1df408475.png)](https://bugsplat.com)
<br/>

# <div align="center">BugSplat</div>

### **<div align="center">Crash and error reporting built for busy developers.</div>**

<div align="center">
    <a href="https://bsky.app/profile/bugsplatco.bsky.social"><img alt="Follow @bugsplatco on Bluesky" src="https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor%3Dbugsplatco.bsky.social&query=%24.followersCount&style=social&logo=bluesky&label=Follow%20%40bugsplatco.bsky.social"></a>
    <a href="https://discord.gg/bugsplat"><img alt="Join BugSplat on Discord" src="https://img.shields.io/discord/664965194799251487?label=Join%20Discord&logo=Discord&style=social"></a>
</div>

<br/>

# MyCMakeCrasher Project

A cross-platform application demonstrating Crashpad integration with CMake.

## About ℹ️

This project demonstrates how to:
- Set up a CMake project for cross-platform development
- Integrate Google's Crashpad library for crash reporting
- Create a simple crashing application that generates crash reports

## Prerequisites ☑️

- CMake (version 3.10 or higher)
- A C++ compiler (gcc, clang, MSVC, etc.)
- Install [depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up) - Google's tools for working with Chromium source code
- A BugSplat account and database (sign up at [bugsplat.com](https://www.bugsplat.com))

## Configuration ⚙️

### BugSplat Setup

1. Sign up for a BugSplat account at [bugsplat.com](https://www.bugsplat.com) if you haven't already
2. Create a new database in your BugSplat dashboard
3. Open `main.h` and define your database name:
   ```cpp
   #define BUGSPLAT_DATABASE "your-database-name"  // Replace with your database name from BugSplat
   ```

### Installing depot_tools

1. Clone the depot_tools repository:

```bash
# For Linux/macOS
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
```

2. Add depot_tools to your PATH:

```bash
# For Linux/macOS - add to your .bashrc or .zshrc
export PATH="$PATH:/path/to/depot_tools"
```

```powershell
# For Windows - add to system environment variables or use:
$env:Path = "C:\path\to\depot_tools;$env:Path"
```

3. For Windows, you also need to run:

```powershell
# From the depot_tools directory
gclient
```

### Symbol Uploads

Symbol uploads must be configured so that crash reports contain function names, file names, and line numbers. The symbol upload process uses BugSplat's [symbol-upload](https://github.com/BugSplat-Git/symbol-upload) utility, which is automatically downloaded as needed.

On Windows, symbol-upload-windows.exe will search for `.pdb` files, on macOS symbol-upload-macos will search for `.dSYM` files, and on Linux, symbol-upload-linux will search for `.debug` files. All files are automatically converted to Crashpad/Breakpad compatible `.sym` files before uploading.

To configure symbol upload, ensure you have:

1. Defined your BugSplat database name in `main.h` as described in the [BugSplat Setup](#bugsplat-setup) section
2. Set up your BugSplat API credentials (`BUGSPLAT_CLIENT_ID` and `BUGSPLAT_CLIENT_SECRET`) in the `.env` file. You can generate a Client ID/Client Secret pair on the [Integrations](https://app.bugsplat.com/v2/database/integrations#oauth) page.

## Building the Project 🏗️

The build scripts will automatically fetch and build Crashpad using `depot_tools`, then build the main application using CMake.

### macOS

```bash
# Execute the build script
./scripts/build_macos.sh

# Run the application
./build/MyCMakeCrasher
```

### Linux

```bash
# Execute the build script
./scripts/build_linux.sh

# Run the application
./build/MyCMakeCrasher
```

### Windows

```powershell
# Execute the build script
.\scripts\build_windows_msvc.ps1

# Run the application
.\build\Debug\MyCMakeCrasher.exe
```

## Testing Crash Reporting 🧪

The application will crash immediately upon launch to demonstrate the crash reporting functionality. Crashes will be automatically uploaded to BugSplat. To view crashes, navigate to the [Dashboard](https://app.bugsplat.com/v2/dashboard) page and ensure the correct database is selected.

<img width="1728" alt="BugSplat Dashboard" src="https://github.com/user-attachments/assets/36572a23-991d-416b-8bdb-bb3627c803cb" />

Click the value in the `ID` column to see the report's stack trace and associated metadata.

<img width="1728" alt="image" src="https://github.com/user-attachments/assets/da7bfbbb-7340-46ad-8d33-5e6ef03052e7" />

Thanks for using BugSplat!
