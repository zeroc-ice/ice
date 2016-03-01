// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ImplicitContextI.h>
#include <Ice/BasicStream.h>
#include <Ice/Object.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

using namespace std;
using namespace Ice;

namespace
{

class SharedImplicitContext : public ImplicitContextI
{
public:
    
    virtual Context getContext() const;
    virtual void setContext(const Context&);

    virtual bool containsKey(const string&) const;
    virtual string get(const string&) const;
    virtual string put(const string&, const string&);
    virtual string remove(const string&);

    virtual void write(const Context&, ::IceInternal::BasicStream*) const;
    virtual void combine(const Context&, Context&) const;

private:
    Context _context;
    IceUtil::Mutex _mutex;
};

#ifndef ICE_OS_WINRT
class PerThreadImplicitContext : public ImplicitContextI
{
public:
    
    PerThreadImplicitContext();
    virtual ~PerThreadImplicitContext();

    virtual Context getContext() const;
    virtual void setContext(const Context&);

    virtual bool containsKey(const string&) const;
    virtual string get(const string&) const;
    virtual string put(const string&, const string&);
    virtual string remove(const string&);

    virtual void write(const Context&, ::IceInternal::BasicStream*) const;
    virtual void combine(const Context&, Context&) const;

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
    static IceUtil::Mutex* _mutex;

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
#endif
}

extern "C" void iceImplicitContextThreadDestructor(void*);



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
    else if(kind == "PerThread")
    {
#ifndef ICE_OS_WINRT
        return new PerThreadImplicitContext;
#else
        throw InitializationException(__FILE__, __LINE__, 
                                      "'PerThread' Ice.ImplicitContext isn't supported for WinRT.");
        return 0; // Keep the compiler happy.
#endif
    }
    else
    {
        throw Ice::InitializationException(
            __FILE__, __LINE__,
            "'" + kind + "' is not a valid value for Ice.ImplicitContext"); 
        return 0; // Keep the compiler happy.
    }
}

#if defined(_WIN32) && !defined(ICE_OS_WINRT)
void
ImplicitContextI::cleanupThread()
{
    if(PerThreadImplicitContext::_nextId > 0)
    {
        iceImplicitContextThreadDestructor(TlsGetValue(PerThreadImplicitContext::_key));
    } 
}
#endif


//
// SharedImplicitContext implementation
//

Context
SharedImplicitContext::getContext() const
{
    IceUtil::Mutex::Lock lock(_mutex);
    return _context;
}

void
SharedImplicitContext::setContext(const Context& newContext)
{
    IceUtil::Mutex::Lock lock(_mutex);
    _context = newContext;
}

bool 
SharedImplicitContext::containsKey(const string& k) const
{
    IceUtil::Mutex::Lock lock(_mutex);
    Context::const_iterator p = _context.find(k);
    return p != _context.end();
}

string 
SharedImplicitContext::get(const string& k) const
{
    IceUtil::Mutex::Lock lock(_mutex);
    Context::const_iterator p = _context.find(k);
    if(p == _context.end())
    {
        return "";
    }
    return p->second;
}


string 
SharedImplicitContext::put(const string& k, const string& v)
{
    IceUtil::Mutex::Lock lock(_mutex);
    string& val = _context[k];
    
    string oldVal = val;
    val = v;
    return oldVal;
}

string
SharedImplicitContext::remove(const string& k)
{
    IceUtil::Mutex::Lock lock(_mutex);
    Context::iterator p = _context.find(k);
    if(p == _context.end())
    {
        return "";
    }
    else
    {
        string oldVal = p->second;
        _context.erase(p);
        return oldVal;
    }
}

void 
SharedImplicitContext::write(const Context& proxyCtx, ::IceInternal::BasicStream* s) const
{
    IceUtil::Mutex::Lock lock(_mutex);
    if(proxyCtx.size() == 0)
    {
        s->write(_context);
    }
    else if(_context.size() == 0)
    {
        lock.release();
        s->write(proxyCtx);
    }
    else
    {
        Context combined = proxyCtx;
        combined.insert(_context.begin(), _context.end());
        lock.release();
        s->write(combined);
    }
}

void 
SharedImplicitContext::combine(const Context& proxyCtx, Context& ctx) const
{
    IceUtil::Mutex::Lock lock(_mutex);
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
// PerThreadImplicitContext implementation
//
#ifndef ICE_OS_WINRT
long PerThreadImplicitContext::_nextId;
PerThreadImplicitContext::IndexInUse* PerThreadImplicitContext::_indexInUse;
IceUtil::Mutex* PerThreadImplicitContext::_mutex = 0;

namespace
{

class Init
{
public:

    Init()
    {
        PerThreadImplicitContext::_mutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete PerThreadImplicitContext::_mutex;
        PerThreadImplicitContext::_mutex = 0;
    }
};

Init init;

}

#   ifdef _WIN32
DWORD PerThreadImplicitContext::_key;
#   else
pthread_key_t PerThreadImplicitContext::_key;
#   endif

PerThreadImplicitContext::PerThreadImplicitContext()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_mutex);
    _id = _nextId++;
    if(_id == 0)
    {
        //
        // Initialize; note that we never dealloc this key (it would be
        // complex, and since it's a static variable, it's not really a leak)
        //
#   ifdef _WIN32
        _key = TlsAlloc();
        if(_key == TLS_OUT_OF_INDEXES)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
#   else
        int err = pthread_key_create(&_key, &iceImplicitContextThreadDestructor);
        if(err != 0)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
        }
#   endif
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
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_mutex);
    (*_indexInUse)[_index] = false;

    if(find(_indexInUse->begin(), _indexInUse->end(), true) == _indexInUse->end())
    {
        delete _indexInUse;
        _indexInUse = 0;
    }
}

Context*
PerThreadImplicitContext::getThreadContext(bool allocate) const
{
#   ifdef _WIN32
    SlotVector* sv = static_cast<SlotVector*>(TlsGetValue(_key));
#   else
    SlotVector* sv = static_cast<SlotVector*>(pthread_getspecific(_key));
#   endif
    if(sv == 0)
    {
        if(!allocate)
        {
            return 0;
        }

        sv = new SlotVector(_index + 1);
#   ifdef _WIN32

        if(TlsSetValue(_key, sv) == 0)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
#   else
        if(int err = pthread_setspecific(_key, sv))
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
        }
#   endif
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
#   ifdef _WIN32
    SlotVector* sv = static_cast<SlotVector*>(TlsGetValue(_key));
#   else
    SlotVector* sv = static_cast<SlotVector*>(pthread_getspecific(_key));
#   endif
    if(sv != 0 && _index < sv->size())
    {
        delete (*sv)[_index].context;
        (*sv)[_index].context = 0;

        //
        // Trim tailing empty contexts.
        //
        size_t i = sv->size();

        bool clear = true;
        while(i != 0) 
        {
            i--;
            if((*sv)[i].context != 0)
            {
                clear = false;
                break;
            }
        }

        //
        // If we did not find any contexts, delete the SlotVector.
        //
        if(clear)
        {
            delete sv;
#   ifdef _WIN32
            if(TlsSetValue(_key, 0) == 0)
            {
                IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
            }
#   else
            if(int err = pthread_setspecific(_key, 0))
            {
                throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
            }
#   endif
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

bool 
PerThreadImplicitContext::containsKey(const string& k) const
{
    const Context* ctx = getThreadContext(false);
    if(ctx == 0)
    {
        return false;
    }
    Context::const_iterator p = ctx->find(k);
    return p != ctx->end();
}

string 
PerThreadImplicitContext::get(const string& k) const
{
    const Context* ctx = getThreadContext(false);
    if(ctx == 0)
    {
        return "";
    }
    Context::const_iterator p = ctx->find(k);
    if(p == ctx->end())
    {
        return "";
    }
    return p->second;
}

string 
PerThreadImplicitContext::put(const string& k, const string& v)
{
    Context* ctx = getThreadContext(true);

    string& val = (*ctx)[k];
    
    string oldVal = val;
    val = v;
    return oldVal;
}

string
PerThreadImplicitContext::remove(const string& k)
{
     Context* ctx = getThreadContext(false);
     if(ctx == 0)
     {
         return "";
     }
     
     Context::iterator p = ctx->find(k);
     if(p == ctx->end())
     {
         return "";
     }
     else
     {
         string oldVal = p->second;
         ctx->erase(p);

         if(ctx->size() == 0)
         {
             clearThreadContext();
         }
         return oldVal;
    }
}

void 
PerThreadImplicitContext::write(const Context& proxyCtx, ::IceInternal::BasicStream* s) const
{
    const Context* threadCtx = getThreadContext(false);

    if(threadCtx == 0 || threadCtx->size() == 0)
    {
        s->write(proxyCtx);
    }
    else if(proxyCtx.size() == 0)
    {
        s->write(*threadCtx);
    }
    else
    {
        Context combined = proxyCtx;
        combined.insert(threadCtx->begin(), threadCtx->end());
        s->write(combined);
    }
}

void 
PerThreadImplicitContext::combine(const Context& proxyCtx, Context& ctx) const
{
    const Context* threadCtx = getThreadContext(false);

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

extern "C" void iceImplicitContextThreadDestructor(void* v)
{
    PerThreadImplicitContext::SlotVector* sv = static_cast<PerThreadImplicitContext::SlotVector*>(v);
    if(sv != 0)
    {
        //
        // Cleanup each slot
        //
        for(PerThreadImplicitContext::SlotVector::iterator p = sv->begin(); p != sv->end(); ++p)
        {
            delete p->context;
        }
        //
        // Then the vector
        //
        delete sv;
    }
}

#endif
