// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
NestedI::nested(Int level, const NestedPrx& proxy, const Ice::Current& current)
{
    cout << level << endl;
    if(--level > 0)
    {
	proxy->nested(level, _self, current.ctx);
    }
}
