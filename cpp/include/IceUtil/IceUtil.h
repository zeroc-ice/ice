//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_ICE_UTIL_H
#define ICE_UTIL_ICE_UTIL_H

#include <IceUtil/PushDisableWarnings.h>

//
// This file must include *all* headers of IceUtil, except
// for DisableWarnings.h and headers with only IceUtilInternal symbols
//

#include <IceUtil/Config.h>
#include <IceUtil/Cond.h>
#include <IceUtil/ConsoleUtil.h>
#if (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
#   include <IceUtil/CtrlCHandler.h>
#endif
#include <IceUtil/Exception.h>
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

#include <IceUtil/PopDisableWarnings.h>
#endif
