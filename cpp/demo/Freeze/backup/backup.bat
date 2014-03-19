@echo off
rem ********************************************************************
rem
rem Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
rem
rem This copy of Ice is licensed to you under the terms described in the
rem ICE_LICENSE file included in this distribution.
rem
rem ********************************************************************

if "%1"=="full" goto fullBackup
if "%1"=="incremental" goto incrementalBackup
echo Usage: %0 {full^|incremental}
exit /b 1

:fullBackup
echo Performing full backup
echo ====== Checkpoint ======
db_checkpoint -1 -h db

if exist hotbackup echo ====== Preserve (move) previous hotbackup directory ===== && (if exist hotbackup.1 rmdir /s /q hotbackup.1) && move hotbackup hotbackup.1

echo ====== Retrieve list of old logs ======
db_archive -h db > oldlogs.txt

echo ====== Run db_hotbackup ======
db_hotbackup -b hotbackup -v -D -h db
if errorlevel 1 exit /b %errorlevel%

echo ===== Remove old logs =====
for /f %%i in (oldlogs.txt) do del /q db\logs\%%i && echo db\logs\%%i deleted
del /q oldLogs.txt
exit /b 0

:incrementalBackup
echo Performing incremental backup
if exist hotbackup echo ====== Preserve (copy) previous hotbackup directory ===== && (if exist hotbackup.1 rmdir /s /q hotbackup.1) && xcopy /s /q hotbackup hotbackup.1\
echo ====== Run db_hotbackup -c -u (log archival) ======
db_hotbackup -c -u -b hotbackup -v -D -h db
if errorlevel 1 exit /b %errorlevel%
exit /b 0

