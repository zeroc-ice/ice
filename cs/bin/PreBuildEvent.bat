@echo off
C:\cygwin\home\michi\src\icicle\compileSlice.bat C:\cygwin\home\michi\src\icicle\ Ice
if errorlevel 1 goto CSharpReportError
goto CSharpEnd
:CSharpReportError
echo Project error: A tool returned an error code from the build event
exit 1
:CSharpEnd