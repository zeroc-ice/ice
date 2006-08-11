// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
	_longStringSeq(Demo::LongStringSeqSize, "As far as the laws of mathematics refer to reality, they are not certain; and as far as they are certain, they do not refer to reality."),
	_structSeq(Demo::StringDoubleSeqSize)
    {
        for(int i = 0; i < Demo::StringDoubleSeqSize; ++i)
	{
	    _structSeq[i].s = "hello";
	    _structSeq[i].d = 3.14;
	}
    }

#ifdef ICEE_USE_ARRAY_MAPPING
    virtual void
    sendByteSeq(const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&)
    {
    }

    virtual void
    sendByteSeqNZ(const Demo::ByteSeq&, const Ice::Current&)
    {
    }
#else
    virtual void
    sendByteSeq(const Demo::ByteSeq&, const Ice::Current&)
    {
    }
#endif

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
    recvLongStringSeq(const Ice::Current&)
    {
	return _longStringSeq;
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
    shutdown(const Ice::Current& c)
    {
	c.adapter->getCommunicator()->shutdown();
    }

private:

    Demo::ByteSeq _byteSeq;
    Demo::StringSeq _stringSeq;
    Demo::StringSeq _longStringSeq;
    Demo::StringDoubleSeq _structSeq;
};

#endif
