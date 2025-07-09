# UE4 Game Memory Library

This project is a small library for reading memory from **Unreal Engine 4** games on Android. It uses the **Android NDK** along with **ImGui** to display in-game information. The source is written in **C++20** and targets rooted devices.

## Prerequisites

- Android NDK installed
- ADB (Android Debug Bridge) working
- Rooted Android device
- Build with **C++20**

Before building make sure to adjust the NDK path in both `build.bat` and `.vscode/c_cpp_properties.json` to match your local installation.

## Getting the Code

```bash
git clone https://github.com/mohamad-aljeiawi/ue4-memory.git
cd ue4-memory
```

After editing the NDK paths run the build script.

## Project Layout

```
ue4-memory/
├── build.bat           Windows build and deployment script
├── jni/                Source files and NDK configuration
│   ├── Android.mk      NDK build settings
│   ├── Application.mk  ABI and compiler options
│   ├── src/            C++ sources (`main.cpp`)
│   └── include/        Headers and helper libraries
```

Key directories under `include/`:

- `utils/`  – memory helpers, process utils, touch support
- `native/` – window creation and OpenGL rendering via ImGui
- `types/`  – structures and offsets used by the target game

## Building and Running

From a Windows command prompt:

```bash
build.bat
```

The script will:

1. Build the project using the NDK
2. Push the executable to `/data/local/tmp/` on the device
3. Set executable permissions and launch it

Ensure your device is connected via ADB and has root access.

## Rendering Overview

Rendering is handled by `imgui_renderer.h`:

```cpp
namespace Renderer {
    void Init(ANativeWindow* window, int width, int height);
    void StartFrame();
    void EndFrame();
    void Shutdown();
}
```

`StartFrame` and `EndFrame` are called once per draw loop. Drawing is performed through ImGui and helper utilities in `utils`.

### Double-Buffer System

`main.cpp` implements a simple double buffer to avoid race conditions between the drawing thread and the memory reader:

```cpp
StructsGame::GameData game_buffers[2];
std::atomic<int> write_buffer_index{0};
std::atomic<int> ready_buffer_index{-1};
```

The memory reader writes to `game_buffers[write_buffer_index]` while the drawing thread consumes `game_buffers[ready_buffer_index]`. After finishing a frame the indices are swapped. This approach keeps rendering smooth while data is updated asynchronously.

To add another thread for reading data you can create a similar worker that fills the current write buffer and then updates `ready_buffer_index` once data is ready.

## Credits

- **Touch simulation** is based on [kp7742/TouchSimulation](https://github.com/kp7742/TouchSimulation), extended with additional touch zones and orientation handling.
- `ANativeWindowCreator.h` comes from [enenH/AndroidImgui](https://github.com/enenH/AndroidImgui).

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for the full text.
