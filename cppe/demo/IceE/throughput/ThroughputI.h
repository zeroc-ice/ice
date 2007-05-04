// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef THROUGHPUT_I_H
#define THROUGHPUT_I_H

#include <Throughput.h>

class ThroughputI : public Demo::Throughput
{
public:

    ThroughputI(int);

    virtual void sendByteSeq(const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);
    virtual Demo::ByteSeq recvByteSeq(const Ice::Current&);
    virtual Demo::ByteSeq echoByteSeq(const Demo::ByteSeq&, const Ice::Current&);
    virtual void sendStringSeq(const Demo::StringSeq&, const Ice::Current&);
    virtual Demo::StringSeq recvStringSeq(const Ice::Current&);
    virtual Demo::StringSeq echoStringSeq(const Demo::StringSeq&, const Ice::Current&);
    virtual void sendStructSeq(const Demo::StringDoubleSeq&, const Ice::Current&);
    virtual Demo::StringDoubleSeq recvStructSeq(const Ice::Current&);
    virtual Demo::StringDoubleSeq echoStructSeq(const Demo::StringDoubleSeq&, const Ice::Current&);
    virtual void sendFixedSeq(const Demo::FixedSeq&, const Ice::Current&);
    virtual Demo::FixedSeq recvFixedSeq(const Ice::Current&);
    virtual Demo::FixedSeq echoFixedSeq(const Demo::FixedSeq&, const Ice::Current&);
    virtual void shutdown(const Ice::Current& c);

private:

    const Demo::ByteSeq _byteSeq;
    const Demo::StringSeq _stringSeq;
    /*const*/ Demo::StringDoubleSeq _structSeq;
    /*const*/ Demo::FixedSeq _fixedSeq;
};

#endif
