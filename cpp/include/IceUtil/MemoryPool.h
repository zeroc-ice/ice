// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef UTIL_MEMORY_POOL_H
#define UTIL_MEMORY_POOL_H

#include <IceUtil/Mutex.h>

namespace IceUtil
{

class ICE_UTIL_API SimpleMemoryPool
{
public:

    SimpleMemoryPool(const char*, int);

    void* operatorNew(size_t);
    void operatorDelete(void*);

    static void usePool(bool);
    static void traceLevel(int);

protected:

    static bool _usePool;

private:

    const char* _name;
    const int _num;

    void* _pool;

    static int _traceLevel;
};

class ICE_UTIL_API MemoryPool : public SimpleMemoryPool, IceUtil::Mutex
{
public:

    MemoryPool(const char*, int);

    void* operatorNew(size_t);
    void operatorDelete(void*);
};

}

#endif
