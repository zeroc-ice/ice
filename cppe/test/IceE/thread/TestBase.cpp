// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>

#include <TestBase.h>
#include <TestCommon.h>

#include <stdio.h>

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
    tprintf("running %s test... ", _name.c_str());
    try
    {
	run();
    }
    catch(const IceUtil::Exception& e)
    {
        tprintf("%s failed\n", e.toString().c_str());
	throw TestFailed(_name);
    }
    tprintf("ok\n");
}
