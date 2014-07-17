@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this sample source code is subject to the terms of the Microsoft
@REM license agreement under which you licensed this sample source code. If
@REM you did not accept the terms of the license agreement, you are not
@REM authorized to use this sample source code. For the terms of the license,
@REM please see the license agreement between you and Microsoft or, if applicable,
@REM see the LICENSE.RTF on your install media or the root of your tools installation.
@REM THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
@REM
if /i not "%1"=="preproc" goto :Not_Preproc
    goto :EOF
:Not_Preproc
if /i not "%1"=="pass1" goto :Not_Pass1

    REM ==============================================================================================
    REM
    REM Application Builder Debugging Support
    REM
    REM ==============================================================================================
    if not "%SYSGEN_APPDEBUG%"=="1" goto NoAPPDEBUG
        set SYSGEN_DCOM=1
        set SYSGEN_MINGDI=1
        set SYSGEN_MINGWES=1
        set SYSGEN_MINWMGR=1
        set SYSGEN_MSVCP=1
        set SYSGEN_NETWORKING=1
        set __SYSGEN_OLEAUT32=1
        set __SYSGEN_WWSAPI=1
        set SYSGEN_FSDBASE=1
        set SYSGEN_MININPUT=1
        set SYSGEN_TOOLHELP=1
    :NoAPPDEBUG

    REM ==============================================================================================
    REM
    REM Remote Tools Support
    REM
    REM ==============================================================================================
    if not "%SYSGEN_REMOTETOOLS%"=="1" goto NoREMOTETOOLS
        set SYSGEN_FSDBASE=1
        set SYSGEN_MINGDI=1
        set SYSGEN_MINGWES=1
        set SYSGEN_MININPUT=1
        set SYSGEN_MINWMGR=1
        set SYSGEN_NETWORKING=1
        set SYSGEN_OLE=1
        set SYSGEN_SHELL=1
    :NoREMOTETOOLS

    goto :EOF
:Not_Pass1
if /i not "%1"=="pass2" goto :Not_Pass2
    goto :EOF
:Not_Pass2
if /i not "%1"=="report" goto :Not_Report
    goto :EOF
:Not_Report
echo %0 Invalid parameter %1
