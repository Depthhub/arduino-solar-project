@echo off
title UPLOAD - Hold Reset Method
color 0A
echo.
echo  ==========================================
echo    ARDUINO UPLOAD - EASIEST METHOD
echo  ==========================================
echo.
echo  READ THIS CAREFULLY:
echo.
echo  1. Unplug ALL wires from Arduino except USB
echo     (remove DHT11, RTC, everything)
echo.
echo  2. Close Serial Monitor in Arduino IDE
echo.
echo  3. When ready, press any key here...
echo.
pause >nul

set "CLI=C:\Program Files\Arduino CLI\arduino-cli.exe"
set "SKETCH=c:\Users\user\arduino solar project\SolarTemperatureController"

echo.
echo  Compiling...
"%CLI%" compile --fqbn arduino:avr:uno "%SKETCH%"
if errorlevel 1 goto fail

echo.
echo  ==========================================
echo.
echo    NOW DO THIS EXACTLY:
echo.
echo    A) HOLD DOWN the RESET button on Arduino
echo    B) Press any key HERE (while still holding RESET)
echo    C) RELEASE RESET when you see "Uploading..."
echo.
echo  ==========================================
echo.
pause >nul

echo  Uploading to COM11...
"%CLI%" upload -p COM11 --fqbn arduino:avr:uno "%SKETCH%"

if errorlevel 1 goto fail

echo.
echo  ==========================================
echo    SUCCESS!
echo  ==========================================
echo  Now plug your wires back in.
echo  Open Serial Monitor at 9600.
echo.
pause
exit /b 0

:fail
echo.
echo  ==========================================
echo    Did not work - try again
echo  ==========================================
echo.
echo  Try 3 more times with these steps:
echo    1. Only USB connected (no wires)
echo    2. HOLD reset - press key - RELEASE when Uploading
echo.
echo  OR try different USB cable / USB port
echo.
pause
exit /b 1
