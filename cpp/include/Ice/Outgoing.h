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
#include <Ice/Stream.h>

namespace __Ice
{

class ICE_API Outgoing : public JTCMonitor
{
public:

    Outgoing(const Emitter&);
    ~Outgoing();

    void invoke();
    void finished(Stream&);

    Stream* is();
    Stream* os();

private:

    Outgoing(const Outgoing&);
    void operator=(const Outgoing&);

    Emitter emitter_;

    enum
    {
	StateInProgress,
	StateFinished
    } state_;

    Stream is_;
    Stream os_;
};

}

#endif
