// **********************************************************************
//
// Copyright (c) 2003
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

#include <TestBase.h>

using namespace std;

TestFailed::TestFailed(const std::string& n) :
    name(n)
{
}

TestBase::TestBase(const std::string& n) :
    _name(n)
{
}

string
TestBase::name() const
{
    return _name;
}

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
