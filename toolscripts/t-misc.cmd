@echo off
call startup.cmd %0

echo.
echo -nologo -o %OUTTESTDIR%\cup.sdkmesh -it %REFDIR%\reftexture.dds %REFDIR%\cup._obj
"%RUN%" -nologo -o %OUTTESTDIR%\cup.sdkmesh -it %REFDIR%\reftexture.dds %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -o %OUTTESTDIR%\cup2.sdkmesh -c %REFDIR%\cup._obj
"%RUN%" -nologo -o %OUTTESTDIR%\cup2.sdkmesh -c %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -o %OUTTESTDIR%\cup.obj -c %REFDIR%\cup._obj
"%RUN%" -nologo -o %OUTTESTDIR%\cup.obj -c %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -o %OUTTESTDIR%\cup3.sdkmesh -m %REFDIR%\cup._obj
"%RUN%" -nologo -o %OUTTESTDIR%\cup3.sdkmesh -m %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -o %OUTTESTDIR%\cup4.sdkmesh -t %REFDIR%\cup._obj
"%RUN%" -nologo -o %OUTTESTDIR%\cup4.sdkmesh -t %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
