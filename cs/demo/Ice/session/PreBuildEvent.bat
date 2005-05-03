@echo off
C:\src\icecs\demo\\..\bin\generatecs.exe C:\src\icecs\demo\ C:\src\icecs\demo\Ice\session\ sessionSD
if errorlevel 1 goto CSharpReportError
goto CSharpEnd
:CSharpReportError
echo Project error: A tool returned an error code from the build event
exit 1
:CSharpEnd