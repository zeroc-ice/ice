// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_H
#define ICEGRID_H

namespace IceGrid
{

class ReplicaGroupFilter : public IceUtil::Shared
{
public:

    virtual Ice::StringSeq filter(const string& replicaGroupId, const Ice::StringSeq& adapterIds, 
                                  const Ice::ConnectionPtr&, const Ice::Context&) = 0;
};
 
};

#endif
