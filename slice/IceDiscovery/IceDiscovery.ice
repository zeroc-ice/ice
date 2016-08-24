// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once
[["cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Ice/Identity.ice>
#include <Ice/JavaCompat.ice>

#ifndef JAVA_COMPAT
[["java:package:com.zeroc"]]
#endif

module IceDiscovery
{

interface LookupReply
{
    void foundObjectById(Ice::Identity id, Object* prx);

    void foundAdapterById(string id, Object* prx, bool isReplicaGroup);
};

interface Lookup
{
    idempotent void findObjectById(string domainId, Ice::Identity id, LookupReply* reply);

    idempotent void findAdapterById(string domainId, string id, LookupReply* reply);
};

};
