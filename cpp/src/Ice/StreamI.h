// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STREAM_I_H
#define ICE_STREAM_I_H

#include <Ice/Stream.h>
#include <Ice/BasicStream.h>

namespace Ice
{

class StreamI : public Stream
{
public:

    StreamI(const ::IceInternal::InstancePtr&);

private:

    ::IceInternal::BasicStream _stream;
};

}

#endif
