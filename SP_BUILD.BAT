rem Compiler used - gcc version 6.3.0 (MinGW.org GCC-6.3.0-1)
set PATHGCC=C:\mingw32\bin
set PATH=%PATHGCC%;%PATH%
set CC=%PATHGCC%\g++.exe
set WINDRES=%PATHGCC%\windres.exe

set SOURCES=SRC/SPLAYER.C SRC/VFW_PLAY.C SRC/SP_RES.C MISC/SP_WINRES.RES
set SDL_PATH=./SDL-1.2-win95
set LIBRARIES= -lmingw32 -lSDLmain -lSDL -lvfw32 -lgdi32 -lwinmm -ldxguid
set FLAGS= -mwindows -O2 -s -static
set OUTPUT_EXE=SPLAYER.EXE

rem Windows resource file compilation
%WINDRES% MISC/SP_WINRES.RC -O coff -o MISC/SP_WINRES.RES

rem Compiling the application
%CC% %SOURCES% -o %OUTPUT_EXE% %FLAGS% %LIBRARIES% -L%SDL_PATH%/lib -I%SDL_PATH%/include

pause