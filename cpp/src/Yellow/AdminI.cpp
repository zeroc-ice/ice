// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Yellow/AdminI.h>

using namespace std;
using namespace Ice;
using namespace Yellow;
using namespace Freeze;

Yellow::AdminI::AdminI(const DBPtr& db) :
    _dict(db)
{
}

void
Yellow::AdminI::add(const string& intf, const ObjectPrx& offer, const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    ObjectProxySeq seq;

    StringObjectProxySeqDict::iterator p = _dict.find(intf);
    if(p != _dict.end())
    {
	seq = p->second;
    }

    seq.push_back(offer);
    if(p == _dict.end())
    {
	_dict.insert(make_pair(intf, seq));
    }
    else
    {
	p.set(seq);
    }
}

void
Yellow::AdminI::remove(const string& intf, const ObjectPrx& offer, const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    ObjectProxySeq seq;

    StringObjectProxySeqDict::iterator p = _dict.find(intf);
    if(p == _dict.end())
    {
	throw NoSuchOfferException();
    }

    seq = p->second;
    Ice::Identity ident = offer->ice_getIdentity();
    ObjectProxySeq::iterator q;
    for(q = seq.begin(); q != seq.end(); ++q)
    {
	ObjectPrx proxy = *q;
	if(proxy->ice_getIdentity() == ident)
	{
	    break;
	}
    }
    if(q == seq.end())
    {
	throw NoSuchOfferException();
    }
    seq.erase(q);
    if(seq.size() == 0)
    {
	_dict.erase(p);
    }
    else
    {
	p.set(seq);
    }
}

