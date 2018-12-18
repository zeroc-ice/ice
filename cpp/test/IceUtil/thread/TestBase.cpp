// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
