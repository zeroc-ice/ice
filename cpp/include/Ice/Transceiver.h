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

namespace _Ice
{

class Buffer;

class TransceiverI : public Shared
{
public:
    
    void write(Buffer&);
    void read(Buffer&);
    
private:

    TransceiverI(const TransceiverI&);
    void operator=(const TransceiverI&);

    TransceiverI(int);
    virtual ~TransceiverI();
    void destroy();
    friend class ConnectorI; // May create and destroy TransceiverIs
    friend class AcceptorI; // May create and destroy TransceiverIs

    int fd_;
};

}

#endif
