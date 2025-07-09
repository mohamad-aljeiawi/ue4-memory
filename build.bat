@echo off

:: Kill existing process
echo Killing existing process...
adb shell su -c "kill -9 $(pidof ue4_memory)"

:: Build using ndk-build
echo Building with NDK...
call "C:/Users/mobil/AppData/Local/Android/Sdk/ndk/29.0.13113456/ndk-build"
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

:: Push executable
echo Pushing ue4_memory to device...
adb push "C:/Users/mobil/Desktop/project/c++/ue4-memory/libs/arm64-v8a/ue4_memory" "/data/local/tmp/"
if %ERRORLEVEL% NEQ 0 (
    echo Push failed!
    pause
    exit /b 1
)

:: Set permissions
echo Setting permissions...
adb shell su -c "chmod 755 /data/local/tmp/ue4_memory"

:: Start Ue4
echo Starting Ue4...
adb shell su -c "/data/local/tmp/ue4_memory"

echo.
echo Press any key to exit...
pause
