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

#include <Yellow/QueryI.h>

using namespace std;
using namespace Ice;
using namespace Yellow;
using namespace Freeze;

Yellow::QueryI::QueryI(const DBPtr& db) :
    _dict(db)
{
}

ObjectPrx
Yellow::QueryI::lookup(const string& intf, const Current&)
{
    StringObjectProxySeqDict::const_iterator p = _dict.find(intf);
    if(p == _dict.end())
    {
	NoSuchOfferException e;
	e.intf = intf;
	throw e;
    }
    
    int r = rand() % p->second.size();
    return p->second[r];
}

ObjectProxySeq
Yellow::QueryI::lookupAll(const string& intf, const Current&)
{
    StringObjectProxySeqDict::const_iterator p = _dict.find(intf);
    if(p == _dict.end())
    {
	NoSuchOfferException e;
	e.intf = intf;
	throw e;
    }
    return p->second;
}
