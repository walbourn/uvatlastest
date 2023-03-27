@echo off
call startup.cmd %0

echo.
echo -nologo -sdkmesh2 -o %OUTTESTDIR%\cup.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo -sdkmesh2 -o %OUTTESTDIR%\cup.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -ib32 -sdkmesh2 -o %OUTTESTDIR%\cup32bit.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo -ib32 -sdkmesh2 -o %OUTTESTDIR%\cup32bit.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -fn r11g11b10 -sdkmesh2 -o %OUTTESTDIR%\cupcompress.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo -fn r11g11b10 -sdkmesh2 -o %OUTTESTDIR%\cupcompress.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -fn r11g11b10 -fuv float16_2 -sdkmesh2 -o %OUTTESTDIR%\cupcompress2.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo -fn r11g11b10 -fuv float16_2 -sdkmesh2 -o %OUTTESTDIR%\cupcompress2.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
