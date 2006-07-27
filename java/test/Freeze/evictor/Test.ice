// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

exception AlreadyRegisteredException
{
};

exception NotRegisteredException
{
};

exception EvictorDeactivatedException
{
};

["freeze:write"] class Servant
{
    ["freeze:read", "cpp:const"] idempotent int getValue();
    ["amd", "freeze:read", "cpp:const"] idempotent int slowGetValue();

    idempotent void setValue(int value);

    ["ami", "amd"] void setValueAsync(int value);
    ["freeze:read", "cpp:const"] void releaseAsync();

    ["freeze:read", "cpp:const"] void addFacet(string name, string data) throws AlreadyRegisteredException;
    ["freeze:read", "cpp:const"] void removeFacet(string name) throws NotRegisteredException;

    ["freeze:read", "cpp:const"] int getTransientValue();
    idempotent void setTransientValue(int value);
    idempotent void keepInCache();
    void release() throws NotRegisteredException;

    void destroy();

    int value;
};

["freeze:write"] class Facet extends Servant
{
    ["freeze:read", "cpp:const"] idempotent string getData();
    idempotent void setData(string data);

    string data;
};

interface RemoteEvictor
{
    idempotent void setSize(int size);

    Servant* createServant(string id, int value) 
	throws AlreadyRegisteredException, EvictorDeactivatedException;

    idempotent Servant* getServant(string id);

    void saveNow();

    void deactivate();
    idempotent void destroyAllServants(string facet);
};

interface RemoteEvictorFactory
{
    RemoteEvictor* createEvictor(string name);
    void shutdown();
};

};

#endif
