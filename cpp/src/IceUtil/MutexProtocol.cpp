// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/MutexProtocol.h>

IceUtil::MutexProtocol
IceUtil::getDefaultMutexProtocol()
{
#ifdef _WIN32
   return PrioNone;
#else
#  if defined(ICE_PRIO_INHERIT) && defined(_POSIX_THREAD_PRIO_INHERIT) && _POSIX_THREAD_PRIO_INHERIT > 0
    return PrioInherit;
#  else
    return PrioNone;
#  endif
#endif
}
