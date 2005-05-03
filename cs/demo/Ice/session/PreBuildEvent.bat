@echo off
C:\cygwin\home\michi\src\icecs\\bin\generatecs.exe C:\cygwin\home\michi\src\icecs\ C:\cygwin\home\michi\src\icecs\demo\Ice\session\ sessionS
if errorlevel 1 goto CSharpReportError
goto CSharpEnd
:CSharpReportError
echo Project error: A tool returned an error code from the build event
exit 1
:CSharpEnd