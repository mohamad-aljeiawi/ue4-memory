@echo off

:: Kill existing process
echo Killing existing process...
adb shell su -c "killall -9 memlib"

:: Build using ndk-build
echo Building with NDK...
call "C:/Users/mobil/AppData/Local/Android/Sdk/ndk/28.0.12674087/ndk-build"
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

:: Push to device
echo Pushing to device...
adb push "C:/Users/mobil/Desktop/project/C++/yun-ang/libs/arm64-v8a/memlib" "/data/local/tmp/"
if %ERRORLEVEL% NEQ 0 (
    echo Push failed!
    pause
    exit /b 1
)

:: Set permissions
echo Setting permissions...
adb shell su -c "chmod 777 /data/local/tmp/memlib"
adb shell chmod 777 /data/local/tmp/memlib

:: Start game
echo Starting PUBG Mobile...
adb shell monkey -p com.tencent.ig -c android.intent.category.LAUNCHER 1
:: Wait for game to load
timeout /t 5

:: Start our lib
echo Starting memlib...
adb shell su -c "/data/local/tmp/memlib"

echo Press any key to exit...
pause