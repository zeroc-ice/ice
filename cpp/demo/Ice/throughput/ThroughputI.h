// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef THROUGHPUT_I_H
#define THROUGHPUT_I_H

#include <Ice/Ice.h>
#include <Throughput.h>

class ThroughputI : public Throughput
{
public:

    ThroughputI() :
	_seq(seqSize, 0)
    {
    }

    virtual void
    sendByteSeq(const ByteSeq&, const Ice::Current&)
    {
    }

    virtual ByteSeq
    recvByteSeq(const Ice::Current&)
    {
	return _seq;
    }

    virtual ByteSeq
    echoByteSeq(const ByteSeq& seq, const Ice::Current&)
    {
	return seq;
    }

private:

    ByteSeq _seq;
};

#endif
