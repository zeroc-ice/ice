// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <NestedI.h>

using namespace std;
using namespace IceE;
using namespace Demo;

NestedI::NestedI(const NestedPrx& self) :
    _self(self)
{
}

void
NestedI::nestedCall(Int level, const NestedPrx& proxy, const IceE::Current& current)
{
    printf("%d\n", level);
    if(--level > 0)
    {
	proxy->nestedCall(level, _self, current.ctx);
    }
}
