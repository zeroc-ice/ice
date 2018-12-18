// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once
[["ice-prefix", "cpp:header-ext:h", "cpp:doxygen:include:IceDiscovery/IceDiscovery.h", "objc:header-dir:objc", "python:pkgdir:IceDiscovery",
  "js:module:ice"]]

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
