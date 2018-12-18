// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_H
#define ICEGRID_H

namespace IceGrid
{

/**
 * Abstract base class for a replica group filter.
 * \headerfile IceGrid/IceGrid.h
 */
class ReplicaGroupFilter : public IceUtil::Shared
{
public:

    /**
     * Called by the registry to filter a collection of adapters. The implementation must not block.
     * @param replicaGroupId The replica group identifier involved in this request.
     * @param adapterIds A sequence of object adapter identifiers denoting the object
     * adapters participating in the replica group whose nodes are active at the time of the request.
     * @param connection The incoming connection from the client to the registry.
     * @param context The incoming connection from the client to the registry.
     * @return The filtered list of adapter identifiers.
     */
    virtual Ice::StringSeq filter(const string& replicaGroupId, const Ice::StringSeq& adapterIds,
                                  const Ice::ConnectionPtr& connection, const Ice::Context& context) = 0;
};

};

#endif
