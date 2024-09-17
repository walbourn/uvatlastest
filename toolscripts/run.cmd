@echo off

setlocal

set TARGETEXE=..\..\out\build\x64-Debug-VCPKG\bin\uvatlastool.exe
REM set TARGETEXE=..\..\UVAtlasTool\Bin\Desktop_2022\x64\Debug\uvatlastool.exe
REM set TARGETEXE=..\..\UVAtlasTool\Bin\Desktop_2022\x64\Debug\uvatlastool.exe
REM set TARGETEXE=..\..\UVAtlasTool\Bin\Desktop_2022\Win32\Release\uvatlastool.exe
set SPEC=*

call onetime.cmd

:args
if '%1' EQU '' goto ready
if '%~x1' NEQ '' set TARGETEXE=%1
if '%~x1' EQU '' set SPEC=%1

:ready
echo Target Exe: %TARGETEXE%
echo   Run Spec: %SPEC%

for %%f in ("%~dp0t-%SPEC%.cmd") do (echo. && echo %%~nf ... && cmd.exe /C "%%f")

if exist "%FAILURELOG%" ( echo. && echo FAILURE LOG & type "%FAILURELOG%" )

endlocal
