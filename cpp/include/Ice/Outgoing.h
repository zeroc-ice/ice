// **********************************************************************
//
// Copyright (c) 2002
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

namespace __Ice
{

class ICE_API Outgoing : noncopyable, public JTCMonitorT< JTCMutex >
{
public:

    Outgoing(const Emitter_ptr&, const Reference_ptr&);
    ~Outgoing();

    bool invoke();
    void finished(Stream&);
    void finished(const ::Ice::LocalException&);

    Stream* is();
    Stream* os();

private:

    Emitter_ptr emitter_;
    Reference_ptr reference_;
    std::auto_ptr< ::Ice::LocalException> exception_;

    enum
    {
	StateInProgress,
	StateOK,
	StateException,
	StateLocalException,
    } state_;

    Stream is_;
    Stream os_;
};

}

#endif
