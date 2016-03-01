// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROXY_HANDLE_H
#define ICE_PROXY_HANDLE_H

#include <IceUtil/Handle.h>
#include <Ice/Config.h>

#include <iosfwd>

namespace IceInternal
{

template<typename T> class ProxyHandle;
template<typename T> class Handle;

}

namespace IceProxy 
{ 
namespace Ice
{

class Object;

}
}

namespace Ice
{

typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

class ObjectAdapter;
typedef ::IceInternal::Handle< ::Ice::ObjectAdapter> ObjectAdapterPtr;

typedef ::std::map< ::std::string, ::std::string> Context;
}

namespace IceInternal
{

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx&, const ::Ice::Context*);

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx&, const std::string&, const ::Ice::Context*);

template<typename P> P 
uncheckedCastImpl(const ::Ice::ObjectPrx&);

template<typename P> P
uncheckedCastImpl(const ::Ice::ObjectPrx&, const std::string&);

//
// Upcast
//
template<typename T, typename Y> inline ProxyHandle<T> 
checkedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, T*, const ::Ice::Context*)
{
    return b;
}

template<typename T, typename Y> inline ProxyHandle<T> 
uncheckedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, T*)
{
    return b;
}

//
// Downcast
//
template<typename T, typename Y> inline ProxyHandle<T> 
checkedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, void*, const ::Ice::Context* ctx)
{
#ifdef __SUNPRO_CC
    //
    // Sun CC bug introduced in version 5.10
    //
    const ::Ice::ObjectPrx& o = b;
    return checkedCastImpl<ProxyHandle<T> >(o, ctx);
#else
    return checkedCastImpl<ProxyHandle<T> >(b, ctx);
#endif
}

template<typename T, typename Y> inline ProxyHandle<T> 
uncheckedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, void*)
{
#ifdef __SUNPRO_CC
    //
    // Sun CC bug introduced in version 5.10
    //
    const ::Ice::ObjectPrx& o = b;
    return uncheckedCastImpl<ProxyHandle<T> >(o);
#else
    return uncheckedCastImpl<ProxyHandle<T> >(b);
#endif
}

//
// Like IceInternal::Handle, but specifically for proxies, with
// support for checkedCast() and uncheckedCast() instead of
// dynamicCast().
//
template<typename T>
class ProxyHandle : public ::IceUtil::HandleBase<T>
{
public:
    
    ProxyHandle(T* p = 0)
    {
        this->_ptr = p;

        if(this->_ptr)
        {
            upCast(this->_ptr)->__incRef();
        }
    }
    
    template<typename Y>
    ProxyHandle(const ProxyHandle<Y>& r)
    {
        this->_ptr = r._ptr;

        if(this->_ptr)
        {
            upCast(this->_ptr)->__incRef();
        }
    }

    template<typename Y>
    ProxyHandle(const ::IceUtil::Handle<Y>& r)
    {
        this->_ptr = r._ptr;

        if(this->_ptr)
        {
            upCast(this->_ptr)->__incRef();
        }
    }

    ProxyHandle(const ProxyHandle& r)
    {
        this->_ptr = r._ptr;

        if(this->_ptr)
        {
            upCast(this->_ptr)->__incRef();
        }
    }
    
    ~ProxyHandle()
    {
        if(this->_ptr)
        {
            upCast(this->_ptr)->__decRef();
        }
    }
    
    ProxyHandle& operator=(T* p)
    {
        if(this->_ptr != p)
        {
            if(p)
            {
                upCast(p)->__incRef();
            }

            if(this->_ptr)
            {
                upCast(this->_ptr)->__decRef();
            }
            
            this->_ptr = p;
        }
        return *this;
    }
        
    template<typename Y>
    ProxyHandle& operator=(const ProxyHandle<Y>& r)
    {
        if(this->_ptr != r._ptr)
        {
            if(r._ptr)
            {
                upCast(r._ptr)->__incRef();
            }

            if(this->_ptr)
            {
                upCast(this->_ptr)->__decRef();
            }
            
            this->_ptr = r._ptr;
        }
        return *this;
    }

    template<typename Y>
    ProxyHandle& operator=(const ::IceUtil::Handle<Y>& r)
    {
        if(this->_ptr != r._ptr)
        {
            if(r._ptr)
            {
                upCast(r._ptr)->__incRef();
            }

            if(this->_ptr)
            {
                upCast(this->_ptr)->__decRef();
            }
            
            this->_ptr = r._ptr;
        }
        return *this;
    }

    ProxyHandle& operator=(const ProxyHandle& r)
    {
        if(this->_ptr != r._ptr)
        {
            if(r._ptr)
            {
                upCast(r._ptr)->__incRef();
            }

            if(this->_ptr)
            {
                upCast(this->_ptr)->__decRef();
            }
            
            this->_ptr = r._ptr;
        }
        return *this;
    }

    ::IceProxy::Ice::Object* __upCast() const
    {
        return upCast(this->_ptr);
    }
        
    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r)
    {
        Y* tag = 0;
        Ice::Context* ctx = 0;
        return ::IceInternal::checkedCastHelper<T>(r, tag, ctx);
    }

    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r, const std::string& f)
    {
        Ice::Context* ctx = 0;
#ifdef __SUNPRO_CC
        //
        // Sun CC bug introduced in version 5.10
        //
        const ::Ice::ObjectPrx& o = r;
        return ::IceInternal::checkedCastImpl<ProxyHandle>(o, f, ctx);
#else
        return ::IceInternal::checkedCastImpl<ProxyHandle>(r, f, ctx);
#endif
    }

    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r, const ::Ice::Context& ctx)
    {
        Y* tag = 0;
        return ::IceInternal::checkedCastHelper<T>(r, tag, &ctx);
    }

    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r, const std::string& f, const ::Ice::Context& ctx)
    {
#ifdef __SUNPRO_CC
        //
        // Sun CC bug introduced in version 5.10
        //
        const ::Ice::ObjectPrx& o = r;
        return ::IceInternal::checkedCastImpl<ProxyHandle>(o, f, &ctx);
#else
        return ::IceInternal::checkedCastImpl<ProxyHandle>(r, f, &ctx);
#endif
    }

    template<class Y>
    static ProxyHandle uncheckedCast(const ProxyHandle<Y>& r)
    {
        Y* tag = 0;
        return::IceInternal::uncheckedCastHelper<T>(r, tag);
    }

    template<class Y>
    static ProxyHandle uncheckedCast(const ProxyHandle<Y>& r, const std::string& f)
    {
#ifdef __SUNPRO_CC
        //
        // Sun CC bug introduced in version 5.10
        //
        const ::Ice::ObjectPrx& o = r;
        return ::IceInternal::uncheckedCastImpl<ProxyHandle<T> >(o, f);
#else
        return ::IceInternal::uncheckedCastImpl<ProxyHandle>(r, f);
#endif
    }

    static const std::string& ice_staticId()
    {
        return T::ice_staticId();
    }
};

template<class Y>
std::ostream& operator<<(std::ostream& os, ::IceInternal::ProxyHandle<Y> p)
{
    return os << (p ? p->ice_toString() : std::string(""));
}

}

#endif
