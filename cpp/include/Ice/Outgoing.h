// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_H
#define ICE_OUTGOING_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ReferenceF.h>
#include <Ice/BasicStream.h>
#include <Ice/Current.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

//
// An exception wrapper, which is used for local exceptions that
// require special retry considerations.
//
class ICE_API LocalExceptionWrapper
{
public:

    LocalExceptionWrapper(const Ice::LocalException&, bool);
    LocalExceptionWrapper(const LocalExceptionWrapper&);

    const Ice::LocalException* get() const;

    //
    // If true, always repeat the request. Don't take retry settings
    // or "at-most-once" guarantees into account.
    //
    // If false, only repeat the request if the retry settings allow
    // to do so, and if "at-most-once" does not need to be guaranteed.
    //
    bool retry() const;

private:

    const LocalExceptionWrapper& operator=(const LocalExceptionWrapper&);

    std::auto_ptr<Ice::LocalException> _ex;
    bool _retry;
};

class ICE_API Outgoing : private IceUtil::noncopyable
{
public:

    Outgoing(Ice::ConnectionI*, Reference*, const std::string&, Ice::OperationMode, const Ice::Context*, bool);

    bool invoke(); // Returns true if ok, false if user exception.
    void abort(const Ice::LocalException&);
    void finished(BasicStream&);
    void finished(const Ice::LocalException&);

    // Inlined for speed optimization.
    BasicStream* is() { return &_is; }
    BasicStream* os() { return &_os; }

private:

    //
    // Optimization. The connection and the reference may not be
    // deleted while a stack-allocated Outgoing still holds it.
    //
    Ice::ConnectionI* _connection;
    Reference* _reference;

    std::auto_ptr<Ice::LocalException> _exception;

    enum
    {
        StateUnsent,
        StateInProgress,
        StateOK,
        StateUserException,
        StateLocalException
    } _state;

    BasicStream _is;
    BasicStream _os;

    const bool _compress;

    //
    // NOTE: we use an attribute for the monitor instead of inheriting
    // from the monitor template.  Otherwise, the template would be
    // exported from the DLL on Windows and could cause linker errors
    // because of multiple definition of IceUtil::Monitor<IceUtil::Mutex>, 
    // see bug 1541.
    //
    IceUtil::Monitor<IceUtil::Mutex> _monitor;
};

}

#endif
