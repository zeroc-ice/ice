// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

void
ThroughputI::echoByteSeq_async(const Demo::AMD_Throughput_echoByteSeqPtr& cb, 
                         const std::pair<const Ice::Byte*, const Ice::Byte*>& seq, const Ice::Current&)
{
    cb->ice_response(seq);
}

Demo::StringSeq
ThroughputI::echoStringSeq(const Demo::StringSeq& seq, const Ice::Current&)
{
    return seq;
}

Demo::StringDoubleSeq
ThroughputI::echoStructSeq(const Demo::StringDoubleSeq& seq, const Ice::Current&)
{
    return seq;
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
