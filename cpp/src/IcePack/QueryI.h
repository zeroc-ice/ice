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

#ifndef ICE_PACK_QUERY_I_H
#define ICE_PACK_QUERY_I_H

#include <IcePack/Query.h>

namespace IcePack
{

class QueryI : public Query, public IceUtil::Mutex
{
public:

    QueryI(const Ice::CommunicatorPtr&, const ObjectRegistryPtr&);
    virtual ~QueryI();

    virtual ::Ice::ObjectPrx findObjectById(const ::Ice::Identity&, const ::Ice::Current&) const;
    virtual ::Ice::ObjectPrx findObjectByType(const ::std::string&, const ::Ice::Current&) const;
    virtual ::Ice::ObjectProxySeq findAllObjectsWithType(const ::std::string&, const ::Ice::Current&) const;

private:

    Ice::CommunicatorPtr _communicator;
    ObjectRegistryPtr _objectRegistry;
};

}

#endif
