@echo off
REM ============================================================
REM  DOL BENGKULU - Windows build & installer (one-click)
REM  Build VST3 lalu susun installer EXE dengan Inno Setup.
REM
REM  Penggunaan:
REM    build_windows.bat                     (cari JUCE otomatis)
REM    build_windows.bat -JUCE C:\JUCE       (beri tau path JUCE)
REM ============================================================
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_windows.ps1" %*
if errorlevel 1 (
    echo.
    echo [!] Build gagal. Pastikan CMake, JUCE, dan Inno Setup 6 sudah terinstall.
    pause
)
