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
