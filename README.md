# UE4 Game Memory Library

This project is a program that reads memory of Unreal Engine 4 games on Android using the NDK. The project can read data structures specific to the UE4 engine and dsiplay information about objects present in the game.

## Community

<div align="center">
  <h3>Join Our Telegram Community</h3>
  <a href="https://t.me/komyunitihak">
    <img src="https://img.shields.io/badge/Telegram-Join%20Channel-blue?style=for-the-badge&logo=telegram" alt="Telegram Channel">
  </a>
  <p>Join <a href="https://t.me/komyunitihak"><strong>كوميونطي هاك</strong></a> on Telegram for discussions, updates, and support!</p>
</div>

## Build Requirements

- Android NDK
- ADB (Android Debug Bridge) installed and working
- Android device with root access

## Downloading and Installation

### Clone from GitHub

```
git clone https://github.com/CP0004/ue4-memory.git
cd ue4-memory-library
```

### Or Download ZIP

1. Go to `https://github.com/CP0004/ue4-memory`
2. Click the green "Code" button
3. Select "Download ZIP"
4. Extract the downloaded ZIP file to your preferred location

## Project Structure

### Main Files

- `build.bat` - Build and run file for Android. It builds the project, transfers it to the device, and runs it. You must have ADB installed and working.

### Directories

- `jni/` - Directory containing source code and files necessary for building the project

### Build Files

- `jni/Android.mk` - File that defines how to build the project using NDK
- `jni/Application.mk` - File that specifies application settings such as target ABI (arm64-v8a) and Android version

### Source Code Files

- `jni/src/main.cpp` - Main file containing the `main()` function and program logic
- `jni/include/main.h` - Main header file

### Libraries and Tools Directory

- `jni/include/utils/` - Directory containing helper header files:
  - `memory.h` - Functions for memory handling and reading
  - `ue4.h` - Functions for working with the UE4 engine
  - `socket_server.h` - Socket server for communicating with the program
  - `offset.h` - Offset constants for accessing data in game memory
  - `structs.h` - Structure definitions used in the project
  - `utils.h` - Various helper functions
  - `frame_time.h` - For controlling frame rate (FPS)
  - `process.h` - Functions for process handling
  - `logger.h` - For event logging

## How to Build and Run

### Executing build.bat

1. Open Command Prompt or PowerShell
2. Navigate to the project directory
   ```
   cd path/to/ue4-memory-library
   ```
3. Run the build script
   ```
   build.bat
   ```

If you encounter permission issues, run as administrator:

1. Right-click on `build.bat`
2. Select "Run as administrator"

The build script will:

1. Make sure Android NDK and SDK are on your machine
2. Make sure ADB is working and can connect to your Android device
3. Make sure your Android device has root access
4. Build, transfer, and run the project

The program performs the following steps:

1. Builds the project using NDK
2. Transfers the resulting file to `/data/local/tmp/` on the device
3. Sets permissions
4. Launches the UE4 game
5. Runs our `memlib` program

## How the Program Works

The program searches for a UE4 game process and then obtains the base address of the `libUE4.so` library. After that, it uses specific offsets to access data structures within the game and read information about objects. The program can also control the frame rate (FPS) and create a socket server to communicate with other applications.

## Customizing for Different UE4 Games

To use this library with different UE4 games:

1. Update the package name in `main.cpp`
2. Adjust the offsets in `offset.h` to match the target game's memory layout
3. Modify the structure definitions in `structs.h` if necessary

Different UE4 games may have slightly different memory layouts, but the basic principles remain the same. You may need to analyze the specific game to find the correct offsets.
