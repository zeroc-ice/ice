// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
