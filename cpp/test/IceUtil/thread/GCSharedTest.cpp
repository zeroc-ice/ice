// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <GCSharedTest.h>
#include <IceUtil/Thread.h>
#include <IceUtil/GCShared.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string GCSharedTestName("GCShared");

struct TestClass : public Thread, GCShared
{
    virtual void run() {}
    virtual void __gcReachable(GCObjectMultiSet&) const {}
    virtual void __gcClear() {}

    bool basesAreVirtual()
    {
	GCShared::_noDelete = true;
	Thread::_noDelete = false;
	//
	// If we have virtual bases, there will be only one instance of the IceUtil::Shared base class,
	// so GCShared::_noDelete will be false after the second assignment.
	//
	return !GCShared::_noDelete;
    }
};

GCSharedTest::GCSharedTest() :
    TestBase(GCSharedTestName)
{
}

void
GCSharedTest::run()
{
    TestClass tc;
    test(tc.basesAreVirtual());
}
