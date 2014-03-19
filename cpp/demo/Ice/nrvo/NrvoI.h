// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef NRVO_I_H
#define NRVO_I_H

#include <Nrvo.h>
#include <MyStringSeq.h>

class NrvoI : public Demo::Nrvo
{
public:

    NrvoI();
    virtual Demo::StringSeq op1(const Ice::Current&);
    virtual Demo::StringSeq op2(const Ice::Current&);
    virtual Demo::StringSeq op3(int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    MyStringSeq _stringSeq;
};

#endif
