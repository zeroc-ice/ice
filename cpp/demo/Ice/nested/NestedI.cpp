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

#include <Ice/Ice.h>
#include <NestedI.h>

using namespace std;
using namespace Ice;

NestedI::NestedI(const NestedPrx& self) :
    _self(self)
{
}

void
NestedI::nestedCall(Int level, const NestedPrx& proxy, const Ice::Current& current)
{
    cout << level << endl;
    if(--level > 0)
    {
	proxy->nestedCall(level, _self, current.ctx);
    }
}
