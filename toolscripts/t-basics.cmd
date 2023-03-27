@echo off
call startup.cmd %0

echo.
echo -nologo -o %OUTTESTDIR%\cup.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo -o %OUTTESTDIR%\cup.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -sdkmesh -o %OUTTESTDIR%\cup2.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo -sdkmesh -o %OUTTESTDIR%\cup2.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -sdkmesh2 -o %OUTTESTDIR%\cupPBR.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo -sdkmesh2 -o %OUTTESTDIR%\cupPBR.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -cmo -o %OUTTESTDIR%\cup.cmo %REFDIR%\cup._obj
"%RUN%" -nologo -cmo -o %OUTTESTDIR%\cup.cmo %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -vbo -o %OUTTESTDIR%\cup.vbo %REFDIR%\cup._obj
"%RUN%" -nologo -vbo -o %OUTTESTDIR%\cup.vbo %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
