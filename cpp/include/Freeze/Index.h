// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
class ObjectStoreBase;

class FREEZE_API Index : public IceUtil::Shared
{
public:
    
    virtual ~Index();

    const std::string& name() const;
    const std::string& facet() const;

protected:
    
    Index(const std::string&, const std::string&);
  
    virtual bool marshalKey(const Ice::ObjectPtr&, Freeze::Key&) const = 0;
    
    std::vector<Ice::Identity> untypedFindFirst(const Freeze::Key&, Ice::Int) const;
    
    std::vector<Ice::Identity> untypedFind(const Freeze::Key&) const;
    
    Ice::Int untypedCount(const Freeze::Key&) const;
    
    Ice::CommunicatorPtr _communicator;
    Ice::EncodingVersion _encoding;

private:

    friend class IndexI;
    friend class ObjectStoreBase;
    
    std::string _name;
    std::string _facet;
    IndexI* _impl;
};

typedef IceUtil::Handle<Index> IndexPtr;
}

#endif

