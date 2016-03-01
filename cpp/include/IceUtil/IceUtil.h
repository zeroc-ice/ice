// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_ICE_UTIL_H
#define ICE_UTIL_ICE_UTIL_H

#include <IceUtil/PushDisableWarnings.h>

//
// This file must include *all* other headers of IceUtil, except
// for DisableWarnings.h and headers with only IceUtilInternal symbols
//

#include <IceUtil/Config.h>
#include <IceUtil/AbstractMutex.h>
#include <IceUtil/Cache.h>
#include <IceUtil/Cond.h>
#if !defined(__APPLE__) && !defined(ICE_OS_WINRT)
#   include <IceUtil/CtrlCHandler.h>
#endif
#include <IceUtil/Exception.h>
#include <IceUtil/Functional.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Lock.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/MutexProtocol.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/RecMutex.h>
#include <IceUtil/ScopedArray.h>
#include <IceUtil/Shared.h>
#include <IceUtil/StringConverter.h>
#include <IceUtil/Thread.h>
#include <IceUtil/ThreadException.h>
#include <IceUtil/Time.h>
#include <IceUtil/Timer.h>
#include <IceUtil/UUID.h>
#include <IceUtil/UniquePtr.h>

#ifndef _WIN32
#   include <IceUtil/IconvStringConverter.h>
#endif

#include <IceUtil/PopDisableWarnings.h>
#endif
