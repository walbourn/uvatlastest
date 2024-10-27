@echo off
call startup.cmd %0

echo.
echo -nologo --normals-by-angle -o %OUTTESTDIR%\cupn.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo --normals-by-angle -o %OUTTESTDIR%\cupn.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --normals-by-area -o %OUTTESTDIR%\cupna.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo --normals-by-area -o %OUTTESTDIR%\cupna.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --normals-by-equal -o %OUTTESTDIR%\cupne.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo --normals-by-equal -o %OUTTESTDIR%\cupne.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --clockwise -o %OUTTESTDIR%\cupcw.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo --clockwise -o %OUTTESTDIR%\cupcw.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --index-buffer-32-bit -o %OUTTESTDIR%\cupib32.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo --index-buffer-32-bit -o %OUTTESTDIR%\cupib32.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --normal-format r11g11b10 -o %OUTTESTDIR%\cupfna.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo --normal-format r11g11b10 -o %OUTTESTDIR%\cupfna.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --normal-format:r11g11b10 -o %OUTTESTDIR%\cupfnb.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo --normal-format:r11g11b10 -o %OUTTESTDIR%\cupfnb.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --normal-format=r11g11b10 -o %OUTTESTDIR%\cupfnc.sdkmesh %REFDIR%\cup._obj
"%RUN%" -nologo --normal-format=r11g11b10 -o %OUTTESTDIR%\cupfnc.sdkmesh %REFDIR%\cup._obj
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
