// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OUTGOING_H
#define ICE_OUTGOING_H

#include <Ice/EmitterF.h>
#include <Ice/ReferenceF.h>
#include <Ice/Stream.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

class ICE_API Outgoing : noncopyable, public JTCMonitorT< JTCMutex >
{
public:

    Outgoing(const EmitterPtr&, const ReferencePtr&);
    ~Outgoing();

    bool invoke();
    void finished(Stream&);
    void finished(const ::Ice::LocalException&);

    Stream* is();
    Stream* os();

private:

    EmitterPtr _emitter;
    ReferencePtr _reference;
    std::auto_ptr< ::Ice::LocalException> _exception;

    enum
    {
	StateInProgress,
	StateOK,
	StateException,
	StateLocalException,
    } _state;

    Stream _is;
    Stream _os;
};

}

#endif
