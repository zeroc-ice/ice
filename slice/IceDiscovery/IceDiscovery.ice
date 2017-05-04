// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once
[["ice-prefix", "cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Ice/Identity.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

module IceDiscovery
{

interface LookupReply
{
    void foundObjectById(Ice::Identity id, Object* prx);

    void foundAdapterById(string id, Object* prx, bool isReplicaGroup);
}

interface Lookup
{
    idempotent void findObjectById(string domainId, Ice::Identity id, LookupReply* reply);

    idempotent void findAdapterById(string domainId, string id, LookupReply* reply);
}

}
