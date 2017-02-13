:: **********************************************************************
::
:: Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
::
:: This copy of Ice is licensed to you under the terms described in the
:: ICE_LICENSE file included in this distribution.
::
:: **********************************************************************

@echo off

set Configuration=
set Platform=
set Compiler=

if "%1" == "/?" goto usage

if "%1" == "Release" set Configuration=Release
if "%1" == "release" set Configuration=Release
if "%1" == "Debug" set Configuration=Debug
if "%1" == "debug" set Configuration=Debug
if "%1" == "" set Configuration=Debug

if "%Configuration%" == "" goto configuration_error

if "%2" == "Win32" set Platform=Win32
if "%2" == "win32" set Platform=Win32
if "%2" == "x64" set Platform=x64
if "%2" == "" set Platform=Win32

if "%Platform%" == "" goto platform_error

if "%3" == "v100" set Compiler=v100
if "%3" == "v110" set Compiler=v110
if "%3" == "v120" set Compiler=v120
if "%3" == "v140" set Compiler=v140
if "%3" == "mingw4.7.2" set Compiler=mingw4.7.2
if "%3" == "" set Compiler=v120

if "%Compiler%" == "" goto compiler_error

echo Setting Ice Third Party Packages Enviroment for %Configuration% %Platform% %Compiler%

if "%Compiler%" == "v100" goto v100_setup
if "%Compiler%" == "mingw4.7.2" goto v100_mingw4.7.2

set THIRDPARTY_HOME=%~dp0

::
:: Set Java Enviroment
::
set CLASSPATH=%THIRDPARTY_HOME%berkeley.db.java7\build\native\lib\db.jar;%CLASSPATH%
set PATH=%THIRDPARTY_HOME%berkeley.db.java7\build\native\bin\%Platform%\Release;%PATH%

::
:: Set C++ Enviroment
::
if "%Configuration%" == "Debug" set PATH=%THIRDPARTY_HOME%berkeley.db.%Compiler%\build\native\bin\%Platform%\Release;%PATH%

set PATH=%THIRDPARTY_HOME%bzip2.%Compiler%\build\native\bin\%Platform%\%Configuration%;%PATH%
set PATH=%THIRDPARTY_HOME%berkeley.db.%Compiler%\build\native\bin\%Platform%\%Configuration%;%PATH%
set PATH=%THIRDPARTY_HOME%expat.%Compiler%\build\native\bin\%Platform%\%Configuration%;%PATH%

goto :eof

::
:: For Visual Studio 2010 we just need to set bzip2 PATH
::
:v100_setup
set PATH=%THIRDPARTY_HOME%bzip2.%Compiler%\build\native\bin\%Platform%\%Configuration%;%PATH%
goto :eof

::
:: For Mingw we just need to set bzip2 PATH
::
:mingw4.7.2_setup
set PATH=%THIRDPARTY_HOME%bzip2.%Compiler%\build\native\bin\%Platform%;%PATH%
goto :eof

:platform_error
echo:
echo Error in script usage, unknown Platform %1%. The correct usage is:
goto usage

:configuration_error
echo:
echo Error in script usage, unknown Configuration %2%. The correct usage is:
goto usage

:compiler_error
echo:
echo Error in script usage, unknown Compiler %3%. The correct usage is:
goto usage

:usage
echo:
echo ThirdParty.bat [Configuration] [Platform] [Compiler]
echo:
echo  [Configuration] is: Release ^| Debug
echo:
echo  [Platform] is: Win32 ^| x64
echo:
echo  [Compiler] is: v100 ^| v110 ^| v120 ^| v140 ^| mingw4.7.2
echo:
echo For example:
echo:
echo     ThirdParty.bat debug Win32 v140
goto :eof