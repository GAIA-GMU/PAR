@echo off
cls
setlocal

:: Setting variables
set logPath=.\
set logExt=.log
set logName=compilation%logExt%
for /f "tokens=1-3 delims=/" %%a in ("%DATE:~4%") do (set fileDate=%%a-%%b-%%c)
for /f "tokens=1-2 delims=/:" %%a in ("%TIME%") do (set fileTime=%%a%%b)
set logFile=%logPath%%fileDate%_%fileTime%_%logName%

echo. && echo COMPILING OPENPAR

:: Creating the log file
echo %Date% >>"%logFile%"
echo %Time% >>"%logFile%"

echo Writing to  %logFile%
echo. & echo.

:: Windows VS2013 build
call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" x86 >>"%logFile%" 2>&1
echo.==================================================>>"%logFile%"

echo COMPILING LWNETS && echo.>>"%logFile%" 2>&1

    echo lwnet.sln>>"%logFile%" 2>&1
    echo ./lwnets/lwnet.sln /build Release
    devenv ./lwnets/lwnet.sln /build Release >>"%logFile%" 2>&1


echo. && echo.>>"%logFile%"
echo COMPILING DATABASE && echo.>>"%logFile%"

    echo database.sln>>"%logFile%" 2>&1
    echo ./database/database.sln /build Release
    devenv ./database/database.sln /build Release >>"%logFile%" 2>&1


echo. && echo.>>"%logFile%"
echo COMPILING AGENTPROC && echo.>>"%logFile%"

    echo agentProc.sln>>"%logFile%" 2>&1
    echo ./agentProc/agentProc.sln /build Release
    devenv ./agentProc/agentProc.sln /build Release >>"%logFile%" 2>&1


:: Make sure it compiled correctly before continuing
for /f "tokens=1-3 delims=," %%a in ('findstr /C:"failed" %logFile%') do (
    echo %%b | findstr /C:" 0 " 1>nul
    if errorlevel 1 (
      echo. && echo.
      echo Errors Detected in Compilation
      echo See log file for details
      echo Aborting operation
      goto endProgram
    )
)

echo. && echo.
echo Compilation Completed Successfully

echo. && echo.
@pause

@endlocal
