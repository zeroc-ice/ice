// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Mutex.h>
#include <IceUtil/Exception.h>

#ifdef _WIN32

bool
IceUtil::Mutex::tryLock() const
{
    if(!TryEnterCriticalSection(&_mutex))
    {
        return false;
    }
    if(_mutex.RecursionCount > 1)
    {
        LeaveCriticalSection(&_mutex);
        throw ThreadLockedException(__FILE__, __LINE__);
    }
    return true;
}

#endif
