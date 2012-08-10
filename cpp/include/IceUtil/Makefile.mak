# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..
INCLUDE_DIR	= IceUtil

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" != "yes"
install::
	@if not exist "$(install_includedir)\IceUtil" \
	    @echo "Creating $(install_includedir)\IceUtil..." && \
	    mkdir "$(install_includedir)\IceUtil"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\IceUtil"

!else

SDK_HEADERS	= $(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\AbstractMutex.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ArgVector.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Cache.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Cond.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Config.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\CountDownLatch.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\DisableWarnings.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Exception.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\FileUtil.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Functional.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Handle.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\IceUtil.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\InputUtil.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Iterator.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Lock.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Monitor.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Mutex.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\MutexProtocol.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\MutexPtrLock.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\MutexPtrTryLock.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Optional.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Options.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\OutputUtil.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Random.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\RecMutex.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ScopedArray.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Shared.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\StringUtil.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Thread.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ThreadException.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Time.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Timer.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\UUID.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Unicode.h
	
all::	$(SDK_HEADERS)

!endif
