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

#ifndef FREEZE_INDEX_H
#define FREEZE_INDEX_H

#include <Ice/Ice.h>
#include <Freeze/DB.h>
#include <vector>

namespace Freeze
{

class IndexI;
class EvictorI;

class FREEZE_API Index : public IceUtil::Shared
{
public:
    
    virtual ~Index();

protected:
    
    Index(const std::string&);
  
    virtual bool
    marshalKey(const Ice::ObjectPtr&, Freeze::Key&) const = 0;
    
    std::vector<Ice::Identity>
    untypedFindFirst(const Freeze::Key&, Ice::Int) const;
    
    std::vector<Ice::Identity>
    untypedFind(const Freeze::Key&) const;
    
    Ice::Int
    untypedCount(const Freeze::Key&) const;

    friend class IndexI;

    Ice::CommunicatorPtr _communicator;

private:

    friend class EvictorI;
    IndexI* _impl;
};

typedef IceUtil::Handle<Index> IndexPtr;
}

#endif

