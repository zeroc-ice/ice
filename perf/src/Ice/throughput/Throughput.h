// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 2.1.0
// Generated from file `Throughput.ice'

#ifndef __Throughput_h__
#define __Throughput_h__

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

class Throughput;
bool operator==(const Throughput&, const Throughput&);
bool operator!=(const Throughput&, const Throughput&);
bool operator<(const Throughput&, const Throughput&);

}

}

namespace Demo
{

class Throughput;
bool operator==(const Throughput&, const Throughput&);
bool operator!=(const Throughput&, const Throughput&);
bool operator<(const Throughput&, const Throughput&);

}

namespace IceInternal
{

void incRef(::Demo::Throughput*);
void decRef(::Demo::Throughput*);

void incRef(::IceProxy::Demo::Throughput*);
void decRef(::IceProxy::Demo::Throughput*);

}

namespace Demo
{

typedef ::IceInternal::Handle< ::Demo::Throughput> ThroughputPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Demo::Throughput> ThroughputPrx;

void __write(::IceInternal::BasicStream*, const ThroughputPrx&);
void __read(::IceInternal::BasicStream*, ThroughputPrx&);
void __write(::IceInternal::BasicStream*, const ThroughputPtr&);
void __patch__ThroughputPtr(void*, ::Ice::ObjectPtr&);

}

namespace Demo
{

typedef ::std::vector< ::Ice::Byte> ByteSeq;

const ::Ice::Int ByteSeqSize = 500000;

typedef ::std::vector< ::std::string> StringSeq;

const ::Ice::Int StringSeqSize = 50000;

const ::Ice::Int LongStringSeqSize = 5000;

struct StringDouble
{
    ::std::string s;

    ::Ice::Double d;

    bool operator==(const StringDouble&) const;
    bool operator!=(const StringDouble&) const;
    bool operator<(const StringDouble&) const;

    void __write(::IceInternal::BasicStream*) const;
    void __read(::IceInternal::BasicStream*);
};

typedef ::std::vector< ::Demo::StringDouble> StringDoubleSeq;

class __U__StringDoubleSeq { };
void __write(::IceInternal::BasicStream*, const StringDoubleSeq&, __U__StringDoubleSeq);
void __read(::IceInternal::BasicStream*, StringDoubleSeq&, __U__StringDoubleSeq);

const ::Ice::Int StringDoubleSeqSize = 50000;

}

namespace IceProxy
{

namespace Demo
{

class Throughput : virtual public ::IceProxy::Ice::Object
{
public:

    void sendByteSeq(const ::Demo::ByteSeq&);
    void sendByteSeq(const ::Demo::ByteSeq&, const ::Ice::Context&);

    ::Demo::ByteSeq recvByteSeq();
    ::Demo::ByteSeq recvByteSeq(const ::Ice::Context&);

    ::Demo::ByteSeq echoByteSeq(const ::Demo::ByteSeq&);
    ::Demo::ByteSeq echoByteSeq(const ::Demo::ByteSeq&, const ::Ice::Context&);

    void sendStringSeq(const ::Demo::StringSeq&);
    void sendStringSeq(const ::Demo::StringSeq&, const ::Ice::Context&);

    ::Demo::StringSeq recvStringSeq();
    ::Demo::StringSeq recvStringSeq(const ::Ice::Context&);

    ::Demo::StringSeq echoStringSeq(const ::Demo::StringSeq&);
    ::Demo::StringSeq echoStringSeq(const ::Demo::StringSeq&, const ::Ice::Context&);

    void sendStructSeq(const ::Demo::StringDoubleSeq&);
    void sendStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Context&);

    ::Demo::StringDoubleSeq recvStructSeq();
    ::Demo::StringDoubleSeq recvStructSeq(const ::Ice::Context&);

    ::Demo::StringDoubleSeq echoStructSeq(const ::Demo::StringDoubleSeq&);
    ::Demo::StringDoubleSeq echoStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Context&);

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

class Throughput : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual void sendByteSeq(const ::Demo::ByteSeq&, const ::Ice::Context&) = 0;

    virtual ::Demo::ByteSeq recvByteSeq(const ::Ice::Context&) = 0;

    virtual ::Demo::ByteSeq echoByteSeq(const ::Demo::ByteSeq&, const ::Ice::Context&) = 0;

    virtual void sendStringSeq(const ::Demo::StringSeq&, const ::Ice::Context&) = 0;

    virtual ::Demo::StringSeq recvStringSeq(const ::Ice::Context&) = 0;

    virtual ::Demo::StringSeq echoStringSeq(const ::Demo::StringSeq&, const ::Ice::Context&) = 0;

    virtual void sendStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Context&) = 0;

    virtual ::Demo::StringDoubleSeq recvStructSeq(const ::Ice::Context&) = 0;

    virtual ::Demo::StringDoubleSeq echoStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Context&) = 0;

    virtual void shutdown(const ::Ice::Context&) = 0;
};

}

}

namespace IceDelegateM
{

namespace Demo
{

class Throughput : virtual public ::IceDelegate::Demo::Throughput,
		   virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual void sendByteSeq(const ::Demo::ByteSeq&, const ::Ice::Context&);

    virtual ::Demo::ByteSeq recvByteSeq(const ::Ice::Context&);

    virtual ::Demo::ByteSeq echoByteSeq(const ::Demo::ByteSeq&, const ::Ice::Context&);

    virtual void sendStringSeq(const ::Demo::StringSeq&, const ::Ice::Context&);

    virtual ::Demo::StringSeq recvStringSeq(const ::Ice::Context&);

    virtual ::Demo::StringSeq echoStringSeq(const ::Demo::StringSeq&, const ::Ice::Context&);

    virtual void sendStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Context&);

    virtual ::Demo::StringDoubleSeq recvStructSeq(const ::Ice::Context&);

    virtual ::Demo::StringDoubleSeq echoStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Context&);

    virtual void shutdown(const ::Ice::Context&);
};

}

}

namespace IceDelegateD
{

namespace Demo
{

class Throughput : virtual public ::IceDelegate::Demo::Throughput,
		   virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual void sendByteSeq(const ::Demo::ByteSeq&, const ::Ice::Context&);

    virtual ::Demo::ByteSeq recvByteSeq(const ::Ice::Context&);

    virtual ::Demo::ByteSeq echoByteSeq(const ::Demo::ByteSeq&, const ::Ice::Context&);

    virtual void sendStringSeq(const ::Demo::StringSeq&, const ::Ice::Context&);

    virtual ::Demo::StringSeq recvStringSeq(const ::Ice::Context&);

    virtual ::Demo::StringSeq echoStringSeq(const ::Demo::StringSeq&, const ::Ice::Context&);

    virtual void sendStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Context&);

    virtual ::Demo::StringDoubleSeq recvStructSeq(const ::Ice::Context&);

    virtual ::Demo::StringDoubleSeq echoStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Context&);

    virtual void shutdown(const ::Ice::Context&);
};

}

}

namespace Demo
{

class Throughput : virtual public ::Ice::Object
{
public:

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual void sendByteSeq(const ::Demo::ByteSeq&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___sendByteSeq(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Demo::ByteSeq recvByteSeq(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___recvByteSeq(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Demo::ByteSeq echoByteSeq(const ::Demo::ByteSeq&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___echoByteSeq(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void sendStringSeq(const ::Demo::StringSeq&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___sendStringSeq(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Demo::StringSeq recvStringSeq(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___recvStringSeq(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Demo::StringSeq echoStringSeq(const ::Demo::StringSeq&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___echoStringSeq(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void sendStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___sendStructSeq(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Demo::StringDoubleSeq recvStructSeq(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___recvStructSeq(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Demo::StringDoubleSeq echoStructSeq(const ::Demo::StringDoubleSeq&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___echoStructSeq(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void shutdown(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___shutdown(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

void __patch__ThroughputPtr(void*, ::Ice::ObjectPtr&);

}

#endif
