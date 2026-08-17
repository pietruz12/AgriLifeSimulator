@echo off
REM ============================================================
REM  build.bat — Compila Agri Life Simulator per Windows
REM  Richiede: MSYS2 + MinGW-ucrt64 installato
REM ============================================================

title Compilazione Agri Life Simulator

echo.
echo  ====================================================
echo   Agri Life Simulator - Build Script
echo  ====================================================
echo.

REM --- Aggiungi ucrt64 al PATH ---
set "UCRT64=C:\msys64\ucrt64\bin"
set "PATH=%UCRT64%;%PATH%"

REM --- Verifica che g++ esista ---
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERRORE] g++ non trovato in %UCRT64%
    echo.
    echo Assicurati che MSYS2 sia installato in C:\msys64
    echo e che il pacchetto mingw-w64-ucrt-x86_64-gcc sia installato.
    echo.
    echo Apri MSYS2 e digita:
    echo   pacman -S mingw-w64-ucrt-x86_64-gcc
    echo.
    pause
    exit /b 1
)

echo [1/2] Compilazione in corso...
g++.exe main.cpp ^
    -o AgriLifeSimulator.exe ^
    -mwindows ^
    -municode ^
    -lshell32 ^
    -lshlwapi ^
    -O2 ^
    -std=c++17 ^
    -static-libgcc ^
    -static-libstdc++ ^
    -static

if %errorlevel% neq 0 (
    echo.
    echo [ERRORE] Compilazione fallita!
    pause
    exit /b 1
)

echo [2/2] Compilazione completata!
echo.
echo  File creato: AgriLifeSimulator.exe
echo  Fai doppio clic su AgriLifeSimulator.exe per avviare il gioco a schermo intero.
echo.
pause
