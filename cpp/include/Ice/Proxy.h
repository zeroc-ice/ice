// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_H
#define ICE_PROXY_H

#include <Ice/ProxyF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/EmitterF.h>
#include <Ice/ReferenceF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Stream;

ICE_API void write(Stream*, const ::Ice::Object_prx&);
ICE_API void read(Stream*, ::Ice::Object_prx&);

}

namespace Ice
{

class ICE_API Object_prxE
{
public:

    Object_prxE() { }
    Object_prxE(const Object_prxE&);
    explicit Object_prxE(const Object_prx&);
    operator Object_prx() const;
    ::__IceProxy::Ice::Object* operator->() const;
    operator bool() const;

protected:

    Object_prx prx_;        
};

};

namespace __IceProxy { namespace Ice
{

class ICE_API Object : public ::__Ice::Shared, JTCMutex
{
public:

    virtual void _throw();
    bool _isA(const std::string&);

    bool operator==(const Object&) const;
    bool operator!=(const Object&) const;

    ::Ice::Object_prx _twoway() const;
    ::Ice::Object_prx _oneway() const;
    ::Ice::Object_prx _datagram() const;
    ::Ice::Object_prx _timeout(int) const;

    ::__Ice::Reference_ptr __reference() const;
    void __copyTo(::__IceProxy::Ice::Object*) const;

protected:

    Object();
    virtual ~Object();

    ::__Ice::Handle< ::__IceDelegate::Ice::Object> __getDelegate();
    virtual ::__Ice::Handle< ::__IceDelegateM::Ice::Object> __createDelegateM();

private:

    void setup(const ::__Ice::Reference_ptr&);

    ::__Ice::Reference_ptr reference_;
    ::__Ice::Handle< ::__IceDelegate::Ice::Object> delegate_;

    friend class ::__Ice::ProxyFactory;
};

} }

namespace __IceDelegate { namespace Ice
{

class ICE_API Object : public ::__Ice::Shared
{
public:

    virtual bool _isA(const std::string&) = 0;

protected:

    Object();
    virtual ~Object();

private:

    virtual void setup(const ::__Ice::Reference_ptr&);
    friend class ::__IceProxy::Ice::Object;
};

} }

namespace __IceDelegateM { namespace Ice
{

class ICE_API Object : virtual public ::__IceDelegate::Ice::Object
{
public:

    virtual bool _isA(const std::string&);

protected:

    Object();
    virtual ~Object();

    const ::__Ice::Emitter_ptr& __emitter(); // const...& for performance
    const ::__Ice::Reference_ptr& __reference(); // const...& for performance

private:

    ::__Ice::Emitter_ptr emitter_;
    ::__Ice::Reference_ptr reference_;

    virtual void setup(const ::__Ice::Reference_ptr&);
    friend class ::__IceProxy::Ice::Object;
};

} }

#endif
