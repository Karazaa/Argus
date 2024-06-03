@echo off

if not exist "F:\GameDevelopment\UnrealEngine\Engine\Build\BatchFiles\GenerateProjectFiles.bat" goto Error_BatchFileInWrongLocation
call "F:\GameDevelopment\UnrealEngine\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -projectfiles -project="F:\GameDevelopment\Argus\Argus\Argus.uproject" -game -engine -dotnet %*
exit /B %ERRORLEVEL%

:Error_BatchFileInWrongLocation
echo GenerateProjectFiles ERROR: The batch file does not appear to be located in the root UE5 directory.  This script must be run from within that directory.
pause
exit /B 1