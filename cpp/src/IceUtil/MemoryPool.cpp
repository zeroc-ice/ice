// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/MemoryPool.h>

using namespace std;

bool IceUtil::SimpleMemoryPool::_usePool = true;
int IceUtil::SimpleMemoryPool::_traceLevel = 0;

IceUtil::SimpleMemoryPool::SimpleMemoryPool(const char* name, int num) :
    _name(name),
    _num(num),
    _pool(0)
{
}

void*
IceUtil::SimpleMemoryPool::operatorNew(size_t sz)
{
    if(!_usePool)
    {
	return ::operator new(sz);
    }

    sz = (sz + 7) & 0xffffff8U;

    if(_pool == 0 || *reinterpret_cast<void**>(_pool) == 0)
    {
	int total = 8 + sz;
	char* p = reinterpret_cast<char*>(malloc(_num * total));

	int i;
	for(i = 0; i < _num - 1; ++i)
	{
	    *reinterpret_cast<void**>(p + i * total) = p + (i + 1) * total;
	}
	*reinterpret_cast<void**>(p + i * total) = 0;
	
	if(_pool == 0)
	{
	    _pool = p;
	}
	else
	{
	    *reinterpret_cast<void**>(_pool) = p;
	}
    }
    
    void* result = reinterpret_cast<char*>(_pool) + 8;
    _pool = *reinterpret_cast<void**>(_pool);
    return result;
}

void
IceUtil::SimpleMemoryPool::operatorDelete(void* p)
{
    if(!_usePool)
    {
	::operator delete(p);
	return;
    }

    if(p == 0)
    {
	return;
    }
    
    *reinterpret_cast<void**>(reinterpret_cast<char*>(p) - 8) = _pool;
    _pool = reinterpret_cast<char*>(p) - 8;
}

void
IceUtil::SimpleMemoryPool::usePool(bool b)
{
    _usePool = b;
}

void
IceUtil::SimpleMemoryPool::traceLevel(int lvl)
{
    _traceLevel = lvl;
}

IceUtil::MemoryPool::MemoryPool(const char* name, int num) :
    SimpleMemoryPool(name, num)
{
}

void*
IceUtil::MemoryPool::operatorNew(size_t sz)
{
    if(!_usePool)
    {
	return ::operator new(sz);
    }

    IceUtil::Mutex::Lock lock(*this);
    return SimpleMemoryPool::operatorNew(sz);
}

void
IceUtil::MemoryPool::operatorDelete(void* p)
{
    if(!_usePool)
    {
	::operator delete(p);
	return;
    }

    IceUtil::Mutex::Lock lock(*this);
    SimpleMemoryPool::operatorDelete(p);
}
