// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef THROUGHPUT_I_H
#define THROUGHPUT_I_H

#include <Ice/Ice.h>
#include <Throughput.h>

class ThroughputI : public ::Demo::Throughput
{
public:

    ThroughputI() :
	_seq(::Demo::seqSize, 0)
    {
    }

    virtual void
    sendByteSeq(const ::Demo::ByteSeq&, const Ice::Current&)
    {
    }

    virtual ::Demo::ByteSeq
    recvByteSeq(const Ice::Current&)
    {
	return _seq;
    }

    virtual ::Demo::ByteSeq
    echoByteSeq(const ::Demo::ByteSeq& seq, const Ice::Current&)
    {
	return seq;
    }

private:

    ::Demo::ByteSeq _seq;
};

#endif
