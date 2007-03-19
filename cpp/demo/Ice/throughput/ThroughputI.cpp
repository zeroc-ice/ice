// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ThroughputI.h>

ThroughputI::ThroughputI() :
    _byteSeq(Demo::ByteSeqSize),
    _stringSeq(Demo::StringSeqSize, "hello"),
    _structSeq(Demo::StringDoubleSeqSize),
    _fixedSeq(Demo::FixedSeqSize),
    _warmup(true)
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

void
ThroughputI::endWarmup(const Ice::Current&)
{
    _warmup = false;
}

void
ThroughputI::sendByteSeq(const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&)
{
}

Demo::ByteSeq
ThroughputI::recvByteSeq(const Ice::Current&)
{
    if(_warmup)
    {
        return Demo::ByteSeq();
    }
    else
    {
        return _byteSeq;
    }
}

Demo::ByteSeq
ThroughputI::echoByteSeq(const Demo::ByteSeq& seq, const Ice::Current&)
{
    return seq;
}

void
ThroughputI::sendStringSeq(const Demo::StringSeq&, const Ice::Current&)
{
}

Demo::StringSeq
ThroughputI::recvStringSeq(const Ice::Current&)
{
    if(_warmup)
    {
        return Demo::StringSeq();
    }
    else
    {
        return _stringSeq;
    }
}

Demo::StringSeq
ThroughputI::echoStringSeq(const Demo::StringSeq& seq, const Ice::Current&)
{
    return seq;
}

void
ThroughputI::sendStructSeq(const Demo::StringDoubleSeq&, const Ice::Current&)
{
}

Demo::StringDoubleSeq
ThroughputI::recvStructSeq(const Ice::Current&)
{
    if(_warmup)
    {
        return Demo::StringDoubleSeq();
    }
    else
    {
        return _structSeq;
    }
}

Demo::StringDoubleSeq
ThroughputI::echoStructSeq(const Demo::StringDoubleSeq& seq, const Ice::Current&)
{
    return seq;
}

void
ThroughputI::sendFixedSeq(const Demo::FixedSeq&, const Ice::Current&)
{
}

Demo::FixedSeq
ThroughputI::recvFixedSeq(const Ice::Current&)
{
    if(_warmup)
    {
        return Demo::FixedSeq();
    }
    else
    {
        return _fixedSeq;
    }
}

Demo::FixedSeq
ThroughputI::echoFixedSeq(const Demo::FixedSeq& seq, const Ice::Current&)
{
    return seq;
}

void
ThroughputI::shutdown(const Ice::Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}
