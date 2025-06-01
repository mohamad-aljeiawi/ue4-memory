@echo off

:: Kill existing process
echo Killing existing process...
adb shell su -c "killall -9 cping_memory"

:: Build using ndk-build
echo Building with NDK...
call "C:/Users/mobil/AppData/Local/Android/Sdk/ndk/29.0.13113456/ndk-build"
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

:: Push executable
echo Pushing cping_memory to device...
adb push "C:/Users/mobil/Desktop/project/android/cping/cping-memory/libs/arm64-v8a/cping_memory" "/data/local/tmp/"
if %ERRORLEVEL% NEQ 0 (
    echo Push failed!
    pause
    exit /b 1
)

:: Set permissions
echo Setting permissions...
adb shell su -c "chmod 755 /data/local/tmp/cping_memory"

:: Start PUBG Mobile
echo Starting PUBG Mobile...
adb shell monkey -p com.tencent.ig -c android.intent.category.LAUNCHER 1
timeout /t 5 > nul

:: Run cping_memory
echo Starting cping_memory...
adb shell su -c "/data/local/tmp/cping_memory"

echo.
echo Press any key to exit...
pause
