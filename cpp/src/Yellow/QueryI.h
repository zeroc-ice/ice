// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef QUERY_I_H
#define QUERY_I_H

#include <Yellow/Yellow.h>
#include <Yellow/StringObjectProxySeqDict.h>

namespace Yellow
{

class QueryI : public Query
{
public:

    QueryI(const Freeze::DBPtr&);

    virtual ::Ice::ObjectPrx lookup(const ::std::string&, const Ice::Current&);
    virtual ::Ice::ObjectProxySeq lookupAll(const ::std::string&, const Ice::Current&);

private:

    StringObjectProxySeqDict _dict;
};

}

#endif
