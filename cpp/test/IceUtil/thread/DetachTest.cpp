// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <stdio.h>

#include <DetachTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string createTestName("thread detach");

class DetachTestThread : public Thread
{
public:
    
    DetachTestThread()
    {
    }

    virtual void run()
    {
    }
};

typedef Handle<DetachTestThread> DetachTestThreadPtr;

DetachTest::DetachTest() :
    TestBase(createTestName)
{
}

void
DetachTest::run()
{
    //
    // Check that calling join() more than once raises ThreadSyscallException.
    //
    DetachTestThreadPtr t = new DetachTestThread();
    ThreadControl control = t->start();
    control.join();
    bool gotException = false;
    try {
	control.join();
    }
    catch(const ThreadSyscallException&)
    {
	gotException = true;
    }
    test(gotException);

    //
    // Check that calling detach() more than once raises ThreadSyscallException.
    //
    t = new DetachTestThread();
    control = t->start();
    control.detach();
    gotException = false;
    try {
	control.detach();
    }
    catch(const ThreadSyscallException&)
    {
	gotException = true;
    }
    test(gotException);

    //
    // Check that calling join() after detach() raises ThreadSyscallException.
    //
    t = new DetachTestThread();
    control = t->start();
    control.detach();
    gotException = false;
    try {
	control.join();
    }
    catch(const ThreadSyscallException&)
    {
	gotException = true;
    }
    test(gotException);
}
