@echo off

set WORKING_DIR=c:\src\vc60

set ICE_HOME=%WORKING_DIR%\stage\Ice-2.1.0

set BERKELEY_HOME=%WORKING_DIR%\db-4.3.28.NC
set BZIP2_HOME=%WORKING_DIR%\bzip2-1.0.3
set EXPAT_HOME=%WORKING_DIR%\Expat-1.95.8
set OPENSSL_HOME=%WORKING_DIR%\openssl-0.9.8
set PHP_BIN_HOME=%WORKING_DIR%\php-5.0.3-win32
set PHP_SRC_HOME=%WORKING_DIR%\php-5.0.3
set STLPORT_HOME=%WORKING_DIR%\STLport-4.6.2

set ANT_HOME=C:\apache-ant-1.6.5
set INSTALLSHIELD_HOME=C:\Program Files\InstallShield\DevStudio 9\System
set PYTHON_HOME=C:\Python24

:: XXX You shouldn't need to edit below this line. XXX

set PATH=%ANT_HOME%\bin;%INSTALLSHIELD_HOME%;%PYTHON_HOME%
set LIB=
set INCLUDE=
call "%SystemDrive%\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat"
python ..\common\makewindist.py %*
