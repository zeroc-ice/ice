// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef THROUGHPUT_I_H
#define THROUGHPUT_I_H

#include <IceE/IceE.h>
#include <Throughput.h>

class ThroughputI : public Demo::Throughput
{
public:

    ThroughputI() :
	_byteSeq(Demo::ByteSeqSize, 0),
	_stringSeq(Demo::StringSeqSize, "hello"),
	_structSeq(Demo::StringDoubleSeqSize),
	_fixedSeq(Demo::FixedSeqSize)
    {
	int i;
        for(i = 0; i < Demo::StringDoubleSeqSize; ++i)
	{
	    _structSeq[i].s = "hello";
	    _structSeq[i].d = 3.14;
	}
        for(i = 0; i < Demo::FixedSeqSize; ++i)
	{
	    _fixedSeq[i].i = 0;
	    _fixedSeq[i].j = 0;
	    _fixedSeq[i].d = 0;
	}
    }

    virtual void
    sendByteSeq(const Demo::ByteSeq&, const Ice::Current&)
    {
    }

    virtual Demo::ByteSeq
    recvByteSeq(const Ice::Current&)
    {
	return _byteSeq;
    }

    virtual Demo::ByteSeq
    echoByteSeq(const Demo::ByteSeq& seq, const Ice::Current&)
    {
	return seq;
    }

    virtual void
    sendStringSeq(const Demo::StringSeq&, const Ice::Current&)
    {
    }

    virtual Demo::StringSeq
    recvStringSeq(const Ice::Current&)
    {
	return _stringSeq;
    }

    virtual Demo::StringSeq
    echoStringSeq(const Demo::StringSeq& seq, const Ice::Current&)
    {
	return seq;
    }

    virtual void
    sendStructSeq(const Demo::StringDoubleSeq&, const Ice::Current&)
    {
    }

    virtual Demo::StringDoubleSeq
    recvStructSeq(const Ice::Current&)
    {
	return _structSeq;
    }

    virtual Demo::StringDoubleSeq
    echoStructSeq(const Demo::StringDoubleSeq& seq, const Ice::Current&)
    {
	return seq;
    }

    virtual void
    sendFixedSeq(const Demo::FixedSeq&, const Ice::Current&)
    {
    }

    virtual Demo::FixedSeq
    recvFixedSeq(const Ice::Current&)
    {
	return _fixedSeq;
    }

    virtual Demo::FixedSeq
    echoFixedSeq(const Demo::FixedSeq& seq, const Ice::Current&)
    {
	return seq;
    }

    virtual void
    shutdown(const Ice::Current& c)
    {
	c.adapter->getCommunicator()->shutdown();
    }

private:

    Demo::ByteSeq _byteSeq;
    Demo::StringSeq _stringSeq;
    Demo::StringDoubleSeq _structSeq;
    Demo::FixedSeq _fixedSeq;
};

#endif
