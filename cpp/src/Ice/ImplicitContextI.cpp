// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ImplicitContextI.h>
#include <IceUtil/StaticMutex.h>

using namespace std;
using namespace Ice;

namespace
{

class SharedImplicitContextWithoutLocking : public ImplicitContextI
{
public:
    
    virtual Context getContext() const;
    virtual void setContext(const Context&);

    virtual string get(const string&) const;
    virtual string getWithDefault(const string&, const string&) const;
    virtual void set(const string&, const string&);
    virtual void remove(const string&);

    virtual void write(const Context&, ::IceInternal::BasicStream*) const;
    virtual void combine(const Context&, Context&) const;

protected:

    Context _context;
};

class SharedImplicitContext : public SharedImplicitContextWithoutLocking
{
public:
    
    virtual Context getContext() const;
    virtual void setContext(const Context&);

    virtual string get(const string&) const;
    virtual string getWithDefault(const string&, const string&) const;
    virtual void set(const string&, const string&);
    virtual void remove(const string&);

    virtual void write(const Context&, ::IceInternal::BasicStream*) const;
    virtual void combine(const Context&, Context&) const;

private:

    IceUtil::Mutex _mutex;
};


class PerThreadImplicitContext : public ImplicitContextI
{
public:
    
    PerThreadImplicitContext();
    virtual ~PerThreadImplicitContext();

    virtual Context getContext() const;
    virtual void setContext(const Context&);

    virtual string get(const string&) const;
    virtual string getWithDefault(const string&, const string&) const;
    virtual void set(const string&, const string&);
    virtual void remove(const string&);

    virtual void write(const Context&, ::IceInternal::BasicStream*) const;
    virtual void combine(const Context&, Context&) const;
    
    static void threadDestructor(void*);
    

    struct Slot
    {
	Slot() :
	    context(0),
	    owner(-1) // just to avoid UMR; a random value would work as well
	{
	}

	Context* context;
	long owner;
    };


    //
    // Each thread maintains a SlotVector. Each PerThreadImplicitContext instance
    // is assigned a slot in this vector.
    //
    typedef std::vector<Slot> SlotVector;
    
    //
    // We remember which slot-indices are in use (to be able to reuse indices)
    //
    typedef std::vector<bool> IndexInUse;
    static IndexInUse* _indexInUse;
    static IceUtil::StaticMutex _mutex;

    static long _nextId;

#ifdef _WIN32
    static DWORD _key;
#else
    static pthread_key_t _key;
#endif    

private:

    Context* getThreadContext(bool) const;
    void clearThreadContext() const;

    size_t _index; // index in all SlotVector
    long _id; // corresponds to owner in the Slot
};

}


/*static*/ ImplicitContextI*
ImplicitContextI::create(const std::string& kind)
{
    if(kind == "None" || kind == "")
    {
	return 0;
    }
    else if(kind == "Shared")
    {
	return new SharedImplicitContext;
    }
    else if(kind == "SharedWithoutLocking")
    {
	return new SharedImplicitContextWithoutLocking;
    }
    else if(kind == "PerThread")
    {
	return new PerThreadImplicitContext;
    }
    else
    {
	throw Ice::InitializationException(
	    __FILE__, __LINE__,
	    "'" + kind + "' is not a valid value for Ice.ImplicitContext"); 
    }
}

#ifdef _WIN32
void
ImplicitContextI::cleanupThread()
{
    if(PerThreadImplicitContext::_nextId > 0)
    {
	PerThreadImplicitContext::threadDestructor(
	    TlsGetValue(PerThreadImplicitContext::_key));
    } 
}
#endif

//
// SharedImplicitContextWithoutLocking implementation
//

inline Context
SharedImplicitContextWithoutLocking::getContext() const
{
    return _context;
}

inline void
SharedImplicitContextWithoutLocking::setContext(const Context& newContext)
{
    _context = newContext;
}

inline string 
SharedImplicitContextWithoutLocking::get(const string& k) const
{
    Context::const_iterator p = _context.find(k);
    if(p == _context.end())
    {
	throw NotSetException(__FILE__, __LINE__, k);
    }
    return p->second;
}

inline string 
SharedImplicitContextWithoutLocking::getWithDefault(const string& k, const string& d) const
{
    Context::const_iterator p = _context.find(k);
    if(p == _context.end())
    {
	return d;
    }
    return p->second;
}

inline void 
SharedImplicitContextWithoutLocking::set(const string& k, const string& v)
{
    _context[k] = v;
}

inline void 
SharedImplicitContextWithoutLocking::remove(const string& k)
{
    if(_context.erase(k) == 0)
    {
	throw NotSetException(__FILE__, __LINE__, k);
    }
}

void 
SharedImplicitContextWithoutLocking::write(const Context& proxyCtx, ::IceInternal::BasicStream* s) const
{
    if(proxyCtx.size() == 0)
    {
	__write(s, _context, __U__Context());
    }
    else if(_context.size() == 0)
    {
	__write(s, proxyCtx, __U__Context());
    }
    else
    {
	Context combined = proxyCtx;
	combined.insert(_context.begin(), _context.end());
	__write(s, combined, __U__Context());
    }
}

void 
SharedImplicitContextWithoutLocking::combine(const Context& proxyCtx, Context& ctx) const
{
    if(proxyCtx.size() == 0)
    {
	ctx = _context;
    }
    else if(_context.size() == 0)
    {
	ctx = proxyCtx;
    }
    else
    {
	ctx = proxyCtx;
	ctx.insert(_context.begin(), _context.end());
    }

}

//
// SharedImplicitContext implementation
//

Context
SharedImplicitContext::getContext() const
{
    IceUtil::Mutex::Lock lock(_mutex);
    return SharedImplicitContextWithoutLocking::getContext();
}

void
SharedImplicitContext::setContext(const Context& newContext)
{
    IceUtil::Mutex::Lock lock(_mutex);
    SharedImplicitContextWithoutLocking::setContext(newContext);
}

string 
SharedImplicitContext::get(const string& k) const
{
    IceUtil::Mutex::Lock lock(_mutex);
    return SharedImplicitContextWithoutLocking::get(k);
}

string 
SharedImplicitContext::getWithDefault(const string& k, const string& d) const
{
    IceUtil::Mutex::Lock lock(_mutex);
    return SharedImplicitContextWithoutLocking::getWithDefault(k, d);
}

void 
SharedImplicitContext::set(const string& k, const string& v)
{
    IceUtil::Mutex::Lock lock(_mutex);
    SharedImplicitContextWithoutLocking::set(k, v);
}

void 
SharedImplicitContext::remove(const string& k)
{
    IceUtil::Mutex::Lock lock(_mutex);
    SharedImplicitContextWithoutLocking::remove(k);
}

void 
SharedImplicitContext::write(const Context& proxyCtx, ::IceInternal::BasicStream* s) const
{
    IceUtil::Mutex::Lock lock(_mutex);
    if(proxyCtx.size() == 0)
    {
	__write(s, _context, __U__Context());
    }
    else if(_context.size() == 0)
    {
	lock.release();
	__write(s, proxyCtx, __U__Context());
    }
    else
    {
	Context combined = proxyCtx;
	combined.insert(_context.begin(), _context.end());
	lock.release();
	__write(s, combined, __U__Context());
    }
}

void 
SharedImplicitContext::combine(const Context& proxyCtx, Context& ctx) const
{
    IceUtil::Mutex::Lock lock(_mutex);
    SharedImplicitContextWithoutLocking::combine(proxyCtx, ctx);
}

//
// PerThreadImplicitContext implementation
//

long PerThreadImplicitContext::_nextId;
PerThreadImplicitContext::IndexInUse* PerThreadImplicitContext::_indexInUse;
IceUtil::StaticMutex PerThreadImplicitContext::_mutex = ICE_STATIC_MUTEX_INITIALIZER;

#ifdef _WIN32
DWORD PerThreadImplicitContext::_key;
#else
pthread_key_t PerThreadImplicitContext::_key;
#endif

PerThreadImplicitContext::PerThreadImplicitContext()
{
    IceUtil::StaticMutex::Lock lock(_mutex);
    _id = _nextId++;
    if(_id == 0)
    {
	//
	// Initialize; note that we never dealloc this key (it would be
	// complex, and since it's a static variable, it's not really a leak)
	//
#ifdef _WIN32
	_key = TlsAlloc();
	if(_key == TLS_OUT_OF_INDEXES)
	{
	    throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
#else
	int err = pthread_key_create(&_key, &threadDestructor);
	if(err != 0)
	{
	    throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
	}
#endif
    }
    
    //
    // Now grabs an index
    //
    if(_indexInUse == 0)
    {
	_indexInUse = new IndexInUse(1);
    }
    size_t i = 0;
    while(i < _indexInUse->size() && (*_indexInUse)[i])
    {
	i++;
    }

    if(i == _indexInUse->size())
    {
	_indexInUse->resize(i + 1);
    }
    (*_indexInUse)[i] = true;
    _index = i;
}

PerThreadImplicitContext::~PerThreadImplicitContext()
{
    IceUtil::StaticMutex::Lock lock(_mutex);
    (*_indexInUse)[_index] = false;

    if(find(_indexInUse->begin(), _indexInUse->end(), true) == _indexInUse->end())
    {
	delete _indexInUse;
	_indexInUse = 0;
    }
}

/*static*/ void
PerThreadImplicitContext::threadDestructor(void* v)
{
    SlotVector* sv = static_cast<SlotVector*>(v);
    if(sv != 0)
    {
	//
	// Cleanup each slot
	//
	for(SlotVector::iterator p = sv->begin(); p != sv->end(); ++p)
	{
	    delete p->context;
	}
	//
	// Then the vector
	//
	delete sv;
    }
}

Context*
PerThreadImplicitContext::getThreadContext(bool allocate) const
{
#ifdef _WIN32
    SlotVector* sv = static_cast<SlotVector*>(TlsGetValue(_key));
#else
    SlotVector* sv = static_cast<SlotVector*>(pthread_getspecific(_key));
#endif
    if(sv == 0)
    {
	if(!allocate)
	{
	    return 0;
	}

	sv = new SlotVector(_index + 1);
#ifdef _WIN32

	if(TlsSetValue(_key, sv) == 0)
	{
	    throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
#else
	if(int err = pthread_setspecific(_key, sv))
	{
	    throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
	}
#endif
    }
    else
    {
	if(sv->size() <= _index)
	{
	    if(!allocate)
	    {
		return 0;
	    }
	    else
	    {
		sv->resize(_index + 1);
	    }
	}
    }

    Slot& slot = (*sv)[_index];
    if(slot.context != 0)
    {
	if(slot.owner != _id)
	{
	    //
	    // Reuse the slot from another (dead) communicator
	    //
	    slot.context->clear();
	    slot.owner = _id;
	}
	//
	// else keep this slot.context
	//
    }
    else
    {
	if(allocate)
	{
	    slot.context = new Context;
	    slot.owner = _id;
	}
	//
	// else keep null slot.context
	//
    }
    return slot.context;
}

void
PerThreadImplicitContext::clearThreadContext() const
{
#ifdef _WIN32
    SlotVector* sv = static_cast<SlotVector*>(TlsGetValue(_key));
#else
    SlotVector* sv = static_cast<SlotVector*>(pthread_getspecific(_key));
#endif
    if(sv != 0 && _index < sv->size())
    {
	delete (*sv)[_index].context;
	(*sv)[_index].context = 0;

	int i = sv->size() - 1;
	while(i >= 0 && (*sv)[i].context == 0) 
	{
	    i--;
	}
	if(i < 0)
	{
	    delete sv;
#ifdef _WIN32
	    if(TlsSetValue(_key, 0) == 0)
	    {
		IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	    }
#else
	    if(int err = pthread_setspecific(_key, 0))
	    {
		throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
	    }
#endif
	}
	else
	{
	    sv->resize(i + 1);
	}
    }
}


Context
PerThreadImplicitContext::getContext() const
{
    Context* ctx = getThreadContext(false);
    if(ctx == 0)
    {
	return Context();
    }
    else
    {
	return *ctx;
    }
}

void
PerThreadImplicitContext::setContext(const Context& newContext)
{
    if(newContext.size() == 0)
    {
	clearThreadContext();
    }
    else
    {
	Context* ctx = getThreadContext(true);
	assert(ctx != 0);
	*ctx = newContext;
    }
}

string 
PerThreadImplicitContext::get(const string& k) const
{
    Context* ctx = getThreadContext(false);
    if(ctx == 0)
    {
	throw NotSetException(__FILE__, __LINE__, k);
    }
    Context::const_iterator p = ctx->find(k);
    if(p == ctx->end())
    {
	throw NotSetException(__FILE__, __LINE__, k);
    }
    return p->second;
}

string 
PerThreadImplicitContext::getWithDefault(const string& k, const string& d) const
{
    Context* ctx = getThreadContext(false);
    if(ctx == 0)
    {
	return d;
    }
    Context::const_iterator p = ctx->find(k);
    if(p == ctx->end())
    {
	return d;
    }
    return p->second;
}

void 
PerThreadImplicitContext::set(const string& k, const string& v)
{
     Context* ctx = getThreadContext(true);
     (*ctx)[k] = v;
}

void 
PerThreadImplicitContext::remove(const string& k)
{
     Context* ctx = getThreadContext(false);
     if(ctx == 0 || ctx->erase(k) == 0)
     {
	 throw NotSetException(__FILE__, __LINE__, k);
     }
     
     if(ctx->size() == 0)
     {
	 clearThreadContext();
     }
}

void 
PerThreadImplicitContext::write(const Context& proxyCtx, ::IceInternal::BasicStream* s) const
{
    Context* threadCtx = getThreadContext(false);

    if(threadCtx == 0 || threadCtx->size() == 0)
    {
	__write(s, proxyCtx, __U__Context());
    }
    else if(proxyCtx.size() == 0)
    {
	__write(s, *threadCtx, __U__Context());
    }
    else
    {
	Context combined = proxyCtx;
	combined.insert(threadCtx->begin(), threadCtx->end());
	__write(s, combined, __U__Context());
    }
}

void 
PerThreadImplicitContext::combine(const Context& proxyCtx, Context& ctx) const
{
    Context* threadCtx = getThreadContext(false);

    if(threadCtx == 0 || threadCtx->size() == 0)
    {
	ctx = proxyCtx;
    }
    else if(proxyCtx.size() == 0)
    {
	ctx = *threadCtx;
    }
    else
    {
	ctx = proxyCtx;
	ctx.insert(threadCtx->begin(), threadCtx->end());
    }
}
