// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_H
#define ICE_TRANSCEIVER_H

#include <Ice/TransceiverF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Buffer;

class TransceiverI : public Shared
{
public:

    virtual int fd() = 0;
    virtual void close() = 0;
    virtual void shutdown() = 0;
    virtual void write(Buffer&, int) = 0;
    virtual void read(Buffer&, int) = 0;
    virtual std::string toString() const = 0;
    
protected:

    TransceiverI() { }
    virtual ~TransceiverI() { }

private:

    TransceiverI(const TransceiverI&);
    void operator=(const TransceiverI&);
};

}

#endif
