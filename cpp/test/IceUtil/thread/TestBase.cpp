// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <TestBase.h>

using namespace std;

void
TestBase::start()
{
    cout << "running " << _name << " test... " << flush;
    try
    {
	run();
    }
    catch(const IceUtil::Exception& e)
    {
	cout << e << " failed" << endl;
	throw TestFailed(_name);
    }
    cout << "ok" << endl;
}
