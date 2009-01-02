// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <iostream>
#include <NrvoI.h>

using namespace std;

NrvoI::NrvoI() :
    _stringSeq(Demo::StringSeqSize, "hello")
{
}

Demo::StringSeq
NrvoI::op1(const Ice::Current&)
{
    cout << "running op1" << endl;
    MyStringSeq seq = MyStringSeq(Demo::StringSeqSize, "hello");
    return seq;
}

Demo::StringSeq
NrvoI::op2(const Ice::Current&)
{
    cout << "running op2" << endl;
    return _stringSeq;
}

Demo::StringSeq
NrvoI::op3(int size, const Ice::Current&)
{
    cout << "running op3" << endl;
    MyStringSeq seq;
    if(size < 10)
    {
        return MyStringSeq(size, "hello");
    }
    seq = MyStringSeq(10, "hello");
    return seq;
}


void
NrvoI::shutdown(const Ice::Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}
