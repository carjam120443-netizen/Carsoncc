# CarsonCC Windows GUI

The Windows GUI is a native Win32 frontend for CarsonCC.

It provides:
- Carson source editor
- Open/save .car files
- Windows x64 target indicator
- Build, Run, and Build & Run buttons
- Integrated compiler output panel
- Program exit-code reporting
- A persistent Windows application window

The GUI does not replace the command-line compiler. It launches carsoncc.exe as a separate process, keeping the compiler backend and CLI intact.

## Build

With a MinGW-w64 GCC environment:

    gcc -std=c11 -Wall -Wextra -O2 -mwindows gui/main.c -o CarsonCC.exe -lcomdlg32

Place CarsonCC.exe beside carsoncc.exe, or put carsoncc.exe on PATH.

Then launch:

    CarsonCC.exe
