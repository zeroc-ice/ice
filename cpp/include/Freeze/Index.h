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
class ObjectStore;

class FREEZE_API Index : public IceUtil::Shared
{
public:
    
    virtual ~Index();

    const std::string& name() const;
    const std::string& facet() const;

protected:
    
    Index(const std::string&, const std::string& = "");
  
    virtual bool marshalKey(const Ice::ObjectPtr&, Freeze::Key&) const = 0;
    
    std::vector<Ice::Identity> untypedFindFirst(const Freeze::Key&, Ice::Int) const;
    
    std::vector<Ice::Identity> untypedFind(const Freeze::Key&) const;
    
    Ice::Int untypedCount(const Freeze::Key&) const;
    
    Ice::CommunicatorPtr _communicator;

private:

    friend class IndexI;
    friend class ObjectStore;
    
    std::string _name;
    std::string _facet;
    IndexI* _impl;
};

typedef IceUtil::Handle<Index> IndexPtr;
}

#endif

