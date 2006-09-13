// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef ICE_MEMORY_POOL_H
#define ICE_MEMORY_POOL_H

#include <IceUtil/Mutex.h>
#include <Ice/InstanceF.h>

namespace IceInternal
{

//
// Forward declarations.
//
struct PageInfo;
struct BlockInfo;

//
// Public interface to the memory pool
//
class MemoryPool : private IceUtil::noncopyable
{
public:
    MemoryPool(size_t);
    ~MemoryPool();

    Ice::Byte* alloc(size_t);
    Ice::Byte* realloc(Ice::Byte*, size_t);
    void free(Ice::Byte*);

private:
    IceUtil::Mutex _mutex;
    size_t _pageSize;
    size_t _maxPageSize;
    size_t _highWaterMark;
    size_t _currentSize;

    PageInfo* _pages;

    BlockInfo* initBlock(Ice::Byte*, PageInfo*, size_t, bool);
    PageInfo* initPage(size_t);
    PageInfo* createNewPage(size_t);
    void purgePages(bool);
    Ice::Byte* allocBlock(size_t);
    Ice::Byte* getBlock(PageInfo*, size_t);
    void freeBlock(Ice::Byte*);
    Ice::Byte* reallocBlock(Ice::Byte*, size_t);
};

}

#endif  // End of ICE_MEMORY_POOL_H
