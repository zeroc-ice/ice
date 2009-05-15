// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

[["java:package:test.Freeze.oldevictor"]]
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

class Servant
{
    idempotent int getValue();
    ["amd"] idempotent int slowGetValue();

    void setValue(int value);

    ["ami", "amd"] void setValueAsync(int value);
    idempotent void releaseAsync();

    idempotent void addFacet(string name, string data) throws AlreadyRegisteredException;
    idempotent void removeFacet(string name) throws NotRegisteredException;

    idempotent int getTransientValue();
    void setTransientValue(int value);
    void keepInCache();
    void release() throws NotRegisteredException;

    void destroy();

    int value;
};

class Facet extends Servant
{
    idempotent string getData();
    void setData(string data);

    string data;
};

interface RemoteEvictor
{
    void setSize(int size);

    Servant* createServant(string id, int value) 
        throws AlreadyRegisteredException, EvictorDeactivatedException;

    Servant* getServant(string id);

    void saveNow();

    void deactivate();
    void destroyAllServants(string facet);
};

interface RemoteEvictorFactory
{
    RemoteEvictor* createEvictor(string name);
    void shutdown();
};

};

#endif
