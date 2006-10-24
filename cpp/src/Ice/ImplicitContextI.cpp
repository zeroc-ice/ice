// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    virtual void write(const Context&, Context&) const;

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
    virtual void write(const Context&, Context&) const;

private:

    IceUtil::Mutex _mutex;
};


class PerThreadImplicitContext : public ImplicitContextI
{
public:
    
    PerThreadImplicitContext();

    virtual Context getContext() const;
    virtual void setContext(const Context&);

    virtual string get(const string&) const;
    virtual string getWithDefault(const string&, const string&) const;
    virtual void set(const string&, const string&);
    virtual void remove(const string&);

    virtual void write(const Context&, ::IceInternal::BasicStream*) const;
    virtual void write(const Context&, Context&) const;
    
    static void threadDestructor(void*);
    
    typedef std::vector<Context*> ContextVector;
    

    static size_t _count;

#ifdef _WIN32
    static DWORD _key;
#else
    static pthread_key_t _key;
#endif    

private:

    Context* getThreadContext(bool) const;
    void clearThreadContext() const;

    size_t _index;
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
    if(PerThreadImplicitContext::_count > 0)
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
SharedImplicitContextWithoutLocking::write(const Context& proxyCtx, Context& ctx) const
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
SharedImplicitContext::write(const Context& proxyCtx, Context& ctx) const
{
    IceUtil::Mutex::Lock lock(_mutex);
    SharedImplicitContextWithoutLocking::write(proxyCtx, ctx);
}

//
// PerThreadImplicitContext implementation
//

size_t PerThreadImplicitContext::_count;

#ifdef _WIN32
DWORD PerThreadImplicitContext::_key;
#else
pthread_key_t PerThreadImplicitContext::_key;
#endif

PerThreadImplicitContext::PerThreadImplicitContext()
{
    IceUtil::StaticMutex::Lock lock(IceUtil::globalMutex);
    _index = _count++;
    if(_index == 0)
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
}

/*static*/ void
PerThreadImplicitContext::threadDestructor(void* v)
{
    delete static_cast<ContextVector*>(v);
}

Context*
PerThreadImplicitContext::getThreadContext(bool allocate) const
{
#ifdef _WIN32
    ContextVector* val = static_cast<ContextVector*>(TlsGetValue(_key));
#else
    ContextVector* val = static_cast<ContextVector*>(pthread_getspecific(_key));
#endif
    if(val == 0)
    {
	if(!allocate)
	{
	    return 0;
	}

	val = new std::vector<Context*>(_index + 1);
#ifdef _WIN32
	if(TlsSetValue(_key, val) != 0)
	{
	    throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
#else
	if(int err = pthread_setspecific(_key, val))
	{
	    throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
	}
#endif
    }
    else
    {
	if(val->size() <= _index)
	{
	    if(!allocate)
	    {
		return 0;
	    }
	    else
	    {
		val->resize(_index + 1);
		assert((*val)[_index] == 0);
	    }
	}
    }

    ContextVector& contextVector = *val;

    Context* result = contextVector[_index];
    if(result == 0 && allocate)
    {
	result = new Context;
	contextVector[_index] = result;
    }
    return result;
}

void
PerThreadImplicitContext::clearThreadContext() const
{
#ifdef _WIN32
    ContextVector* val = static_cast<ContextVector*>(TlsGetValue(_key));
#else
    ContextVector* val = static_cast<ContextVector*>(pthread_getspecific(_key));
#endif
    if(val != 0 && _index < val->size())
    {
	delete (*val)[_index];
	(*val)[_index] = 0;

	int i = val->size() - 1;
	while(i >= 0 && (*val)[i] == 0) 
	{
	    i--;
	}
	if(i < 0)
	{
	    delete val;
#ifdef _WIN32
	    if(!TlsSetValue(_key, 0))
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
	    val->resize(i + 1);
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
PerThreadImplicitContext::write(const Context& proxyCtx, Context& ctx) const
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
