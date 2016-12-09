// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ASYNC_RESULT_H
#define ICE_ASYNC_RESULT_H

#ifndef ICE_CPP11_MAPPING

#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <Ice/LocalObject.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionF.h>
#include <Ice/ProxyF.h>
#include <Ice/AsyncResultF.h>

namespace Ice
{

class ICE_API AsyncResult : private IceUtil::noncopyable, public Ice::LocalObject
{
public:

    virtual ~AsyncResult();

    virtual void cancel() = 0;

    virtual Int getHash() const = 0;

    virtual CommunicatorPtr getCommunicator() const = 0;
    virtual ConnectionPtr getConnection() const = 0;
    virtual ObjectPrxPtr getProxy() const = 0;

    virtual bool isCompleted() const = 0;
    virtual void waitForCompleted() = 0;

    virtual bool isSent() const = 0;
    virtual void waitForSent() = 0;

    virtual void throwLocalException() const = 0;

    virtual bool sentSynchronously() const = 0;

    virtual LocalObjectPtr getCookie() const = 0;

    virtual const std::string& getOperation() const = 0;

    virtual bool waitForResponse() = 0;
    virtual Ice::InputStream* startReadParams() = 0;
    virtual void endReadParams() = 0;
    virtual void readEmptyParams() = 0;
    virtual void readParamEncaps(const ::Ice::Byte*&, ::Ice::Int&) = 0;
    virtual void throwUserException() = 0;

    static void check(const AsyncResultPtr&, const ::IceProxy::Ice::Object*, const ::std::string&);
    static void check(const AsyncResultPtr&, const Connection*, const ::std::string&);
    static void check(const AsyncResultPtr&, const Communicator*, const ::std::string&);

    class Callback : public IceUtil::Shared
    {
    public:

        virtual void run() = 0;
    };
    typedef IceUtil::Handle<Callback> CallbackPtr;

    virtual void scheduleCallback(const CallbackPtr&) = 0;

protected:

    static void check(const AsyncResultPtr&, const ::std::string&);
};

}

#endif

#endif
