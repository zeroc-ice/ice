// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <NrvoI.h>

using namespace std;

NrvoI::NrvoI() :
    _stringSeq(10, "hello")
{
}

//
// NRVO (Named Return Value Optimization):
// the return value is a stack-allocated variable,
// and there is only a single return path.
//
Demo::StringSeq
NrvoI::op1(const Ice::Current&)
{
    cout << "running op1" << endl;
    //
    // Not "return MyStringSeq(10, "hello")", since this lacks a name.
    //
    MyStringSeq seq = MyStringSeq(10, "hello");
    return seq;
}

//
// No optimization: the return value is a data member.
//
Demo::StringSeq
NrvoI::op2(const Ice::Current&)
{
    cout << "running op2" << endl;
    return _stringSeq;
}


//
// Operation with multiple return paths.
//
Demo::StringSeq
NrvoI::op3(int size, const Ice::Current&)
{
    cout << "running op3" << endl;
    if(size < 10)
    {
        //
        // RVO (Return Value Optimization): return value
        // is constructed in place.
        //
        return MyStringSeq(size, "hello");
    }

    //
    // No optimization: NRVO requires a single return path.
    //
    MyStringSeq seq = MyStringSeq(10, "hello");
    return seq;
}

void
NrvoI::shutdown(const Ice::Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}
