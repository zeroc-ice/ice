// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OUTGOING_H
#define ICEE_OUTGOING_H

#include <IceE/ConnectionF.h>
#include <IceE/ReferenceF.h>
#include <IceE/BasicStream.h>
#include <IceE/OperationMode.h>

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

    enum State
    {
	StateUnsent,
	StateInProgress,
	StateOK,
	StateUserException,
	StateLocalException
    };

    Outgoing(Ice::Connection*, Reference*, const std::string&, Ice::OperationMode, const Ice::Context*);
    ~Outgoing() {}

    bool invoke(); // Returns true if ok, false if user exception.
    void abort(const Ice::LocalException&);
    void finished(BasicStream&);
    void finished(const Ice::LocalException&);

    // Inlined for speed optimization.
    BasicStream* stream() { return &_stream; }
    State state() { return _state; }
    
protected:

    //
    // Optimization. The connection and the reference may not be
    // deleted while a stack-allocated Outgoing still holds it.
    //
    Ice::Connection* _connection;
    Reference* _reference;

    std::auto_ptr<Ice::LocalException> _exception;

    State _state;
    BasicStream _stream;
};

}

#endif
