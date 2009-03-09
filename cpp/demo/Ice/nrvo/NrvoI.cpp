// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
// An example where NRVO work in the server side,
// this is because the operation has a single return
// path, and the returned object is not a data member
// of the servant.
//
Demo::StringSeq
NrvoI::op1(const Ice::Current&)
{
    cout << "running op1" << endl;
    MyStringSeq seq = MyStringSeq(10, "hello");
    return seq;
}

//
// An example where NRVO doesn't work because
// the operation returns returns a data member of
// the servant.
//
Demo::StringSeq
NrvoI::op2(const Ice::Current&)
{
    cout << "running op2" << endl;
    return _stringSeq;
}


//
// An example where NRVO doesn't work because
// the operation has multiple return paths.
//
Demo::StringSeq
NrvoI::op3(int size, const Ice::Current&)
{
    cout << "running op3" << endl;
    if(size < 10)
    {
        return MyStringSeq(size, "hello");
    }
    MyStringSeq seq = MyStringSeq(10, "hello");
    return seq;
}

void
NrvoI::shutdown(const Ice::Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}
