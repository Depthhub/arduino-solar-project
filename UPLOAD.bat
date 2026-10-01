@echo off
title Upload Solar Temperature Controller
echo.
echo ============================================
echo   UPLOAD TO ARDUINO - SIMPLE MODE
echo ============================================
echo.
echo BEFORE YOU CONTINUE:
echo   1. Close Serial Monitor in Arduino IDE (X button)
echo   2. Arduino plugged in by USB
echo.
pause

set PATH=%PATH%;C:\Program Files\Arduino CLI

echo.
echo Compiling...
arduino-cli compile --fqbn arduino:avr:uno "c:\Users\user\arduino solar project\SolarTemperatureController"
if errorlevel 1 (
    echo COMPILE FAILED
    pause
    exit /b 1
)

echo.
echo ============================================
echo   GET READY!
echo.
echo   In 5 seconds this will try to upload.
echo.
echo   WHEN YOU SEE "Uploading..." BELOW:
echo   >>> PRESS THE RESET BUTTON ON ARDUINO ONCE <<<
echo ============================================
echo.
timeout /t 5

echo Uploading to COM4...
arduino-cli upload -p COM4 --fqbn arduino:avr:uno "c:\Users\user\arduino solar project\SolarTemperatureController"

if errorlevel 1 (
    echo.
    echo ============================================
    echo   UPLOAD FAILED - TRY AGAIN
    echo ============================================
    echo.
    echo Do this:
    echo   1. Click Upload again in this window? Close and re-run.
    echo   2. When "Uploading" appears - press RESET on Arduino
    echo   3. Try up to 5 times
    echo.
    pause
    exit /b 1
)

echo.
echo ============================================
echo   SUCCESS! Upload complete.
echo ============================================
echo.
echo Now in Arduino IDE:
echo   Tools - Serial Monitor - set to 9600
echo.
pause
