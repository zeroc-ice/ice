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
#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

void
TestI::deactivate(const Current& current)
{
    current.adapter->deactivate();
    IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
}

string
CookieI::message() const
{
    return "blahblah";
}
