@echo off

set ICE_HOME=D:\Src\vc71\stage\Ice-2.1.0

set BERKELEY_HOME=D:\Src\vc71\db-4.2.52.NC
set BZIP2_HOME=D:\Src\vc71\bzip2-1.0.2
set EXPAT_HOME=D:\Src\vc71\Expat-1.95.8
set OPENSSL_HOME=D:\Src\vc71\openssl-0.9.7e
set PHP_BIN_HOME=D:\Src\vc71\php-5.0.3-win32
set PHP_SRC_HOME=D:\Src\vc71\php-5.0.3
set STLPORT_HOME=

set ANT_HOME=C:\Opt\apache-ant-1.6.1
set INSTALLSHIELD_HOME=C:\Program Files\InstallShield\DevStudio 9\System
set PYTHON_HOME=C:\Python24

REM XXX You shouldn't need to edit below this line. XXX

set PATH=%ANT_HOME%\bin;%INSTALLSHIELD_HOME%;%PYTHON_HOME%
set LIB=
set INCLUDE=
call "%VS71COMNTOOLS%vsvars32.bat"
python ..\common\makewindist.py %1 %2 %3 %4 %5 %6 %7 %8 %9
