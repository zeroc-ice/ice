REM Script to compile Slice files

REM %1 Root directory of source tree
REM %2 Name of project

@echo off

cd %1
slice2cs --ice -Islice %1slice/%2/*.ice
cd %1slice/%2
move *.cs %1src/%2/Generated
if errorlevel 1 goto CSharpReportError
goto CSharpEnd
:CSharpReportError
echo Project error: A tool returned an error code from the build event
exit 1
:CSharpEnd
