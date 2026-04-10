@echo off
setlocal enabledelayedexpansion

:: Windows Flash Script for ESP8266
:: Usage: WindowsFlash.bat <baudrate> <COM port> <esptool path> <flash file>
:: Example: WindowsFlash.bat 921600 COM15 C:\path\to\esptool.py C:\path\to\flashfile.bin
REM This script continuously flashes the ESP8266 with the specified parameters.
REM Make sure to run this script in a command prompt with administrative privileges.

set baudrate=%1
set COM=%2
set esptool=%3
set flashfile=%4
set LAST_MAC=

echo ESP8266 Auto Flash Script
echo Baudrate: %baudrate%  Port: %COM%
echo Monitoring for ESP modules on %COM%...
echo.

:loop
:: Check if an ESP module is attached by reading its MAC address
python %esptool% --chip esp8266 --port %COM% read_mac > esptool_output.txt 2>&1
if errorlevel 1 (
    <nul set /p "=."
    TIMEOUT /T 2 /NOBREAK >nul
    goto loop
)

:: Extract MAC address from detection output
set "CURRENT_MAC="
for /f "tokens=2 delims= " %%A in ('findstr /I "MAC:" esptool_output.txt') do (
    if "!CURRENT_MAC!"=="" set "CURRENT_MAC=%%A"
)

if "!CURRENT_MAC!"=="" (
    echo.
    echo ESP detected but could not read MAC address. Retrying...
    TIMEOUT /T 2 /NOBREAK >nul
    goto loop
)

echo.
echo ESP detected: !CURRENT_MAC!

:: Skip if this is the same device that was just flashed
if /i "!CURRENT_MAC!"=="!LAST_MAC!" (
    echo Device !CURRENT_MAC! was just flashed. Waiting for a new device...
    TIMEOUT /T 2 /NOBREAK >nul
    goto loop
)

:: Flash the device
echo Flashing !CURRENT_MAC!...
python %esptool% --after soft_reset --chip esp8266 --port %COM% --baud %baudrate% write_flash 0x0 %flashfile% > esptool_output.txt 2>&1
set FLASH_RESULT=!ERRORLEVEL!
type esptool_output.txt
echo.

if not "!FLASH_RESULT!"=="0" (
    echo Flash failed for !CURRENT_MAC!. Retrying in 2 seconds...
    TIMEOUT /T 2 /NOBREAK >nul
    goto loop
)

:: Build timestamp using delayed expansion to capture current time
set "TIMEPART=!time: =0!"
set "TIMESTAMP=%date% !TIMEPART:~0,8!"

:: Check if this MAC has been logged before and write entry with duplicate flag if so
findstr /i "!CURRENT_MAC!" mac_addresses.txt >nul 2>&1
if errorlevel 1 (
    echo !TIMESTAMP! !CURRENT_MAC! >> mac_addresses.txt
    echo Logged new device: !CURRENT_MAC!
) else (
    echo !TIMESTAMP! !CURRENT_MAC! DUPLICATE >> mac_addresses.txt
    echo Duplicate device logged: !CURRENT_MAC!
)

set "LAST_MAC=!CURRENT_MAC!"
echo Flash complete. Insert next device...
echo.
TIMEOUT /T 3 /NOBREAK >nul
goto loop

:: cd flashing
:: ./WindowsFlash.bat 921600 COM15 C:\Users\mbgpcsk4\.platformio\packages\tool-esptoolpy\esptool.py C:\Work\Kahuna\.pio\build\esp07s\mavesp-esp07s-2.4.0.bin