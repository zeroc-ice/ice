// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 2.1.0
// Generated from file `Latency.ice'

#ifndef __Latency_h__
#define __Latency_h__

#include <Ice/LocalObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/Outgoing.h>
#include <Ice/Incoming.h>
#include <Ice/Direct.h>
#include <Ice/StreamF.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION != 20100
#       error Ice version mismatch!
#   endif
#endif

namespace IceProxy
{

namespace Demo
{

class Ping;
bool operator==(const Ping&, const Ping&);
bool operator!=(const Ping&, const Ping&);
bool operator<(const Ping&, const Ping&);

}

}

namespace Demo
{

class Ping;
bool operator==(const Ping&, const Ping&);
bool operator!=(const Ping&, const Ping&);
bool operator<(const Ping&, const Ping&);

}

namespace IceInternal
{

void incRef(::Demo::Ping*);
void decRef(::Demo::Ping*);

void incRef(::IceProxy::Demo::Ping*);
void decRef(::IceProxy::Demo::Ping*);

}

namespace Demo
{

typedef ::IceInternal::Handle< ::Demo::Ping> PingPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Demo::Ping> PingPrx;

void __write(::IceInternal::BasicStream*, const PingPrx&);
void __read(::IceInternal::BasicStream*, PingPrx&);
void __write(::IceInternal::BasicStream*, const PingPtr&);
void __patch__PingPtr(void*, ::Ice::ObjectPtr&);

}

namespace Demo
{

}

namespace IceProxy
{

namespace Demo
{

class Ping : virtual public ::IceProxy::Ice::Object
{
public:

    void shutdown();
    void shutdown(const ::Ice::Context&);
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

}

}

namespace IceDelegate
{

namespace Demo
{

class Ping : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual void shutdown(const ::Ice::Context&) = 0;
};

}

}

namespace IceDelegateM
{

namespace Demo
{

class Ping : virtual public ::IceDelegate::Demo::Ping,
	     virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual void shutdown(const ::Ice::Context&);
};

}

}

namespace IceDelegateD
{

namespace Demo
{

class Ping : virtual public ::IceDelegate::Demo::Ping,
	     virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual void shutdown(const ::Ice::Context&);
};

}

}

namespace Demo
{

class Ping : virtual public ::Ice::Object
{
public:

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual void shutdown(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___shutdown(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

void __patch__PingPtr(void*, ::Ice::ObjectPtr&);

}

#endif
