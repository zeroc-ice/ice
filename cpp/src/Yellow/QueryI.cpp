// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
