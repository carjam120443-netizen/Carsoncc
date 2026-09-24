@echo off
setlocal
where gcc >nul 2>nul
if errorlevel 1 (
  echo GCC was not found. Install/use MinGW-w64 or MSYS2 UCRT64.
  exit /b 1
)
gcc -std=c11 -Wall -Wextra -O2 -mwindows gui\main.c -o CarsonCC.exe -lcomdlg32
if errorlevel 1 exit /b 1
echo Built CarsonCC.exe
