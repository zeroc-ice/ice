// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef THROUGHPUT_I_H
#define THROUGHPUT_I_H

#include <Throughput.h>

class ThroughputI : public Demo::Throughput
{
public:

    ThroughputI();

    virtual void echoByteSeq_async(const Demo::AMD_Throughput_echoByteSeqPtr&,
                                   const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);
    virtual Demo::StringSeq echoStringSeq(const Demo::StringSeq&, const Ice::Current&);
    virtual Demo::StringDoubleSeq echoStructSeq(const Demo::StringDoubleSeq&, const Ice::Current&);
    virtual Demo::FixedSeq echoFixedSeq(const Demo::FixedSeq&, const Ice::Current&);
    virtual void shutdown(const Ice::Current& c);

private:

    Demo::ByteSeq _byteSeq;
    Demo::StringSeq _stringSeq;
    Demo::StringDoubleSeq _structSeq;
    Demo::FixedSeq _fixedSeq;
};

#endif
