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

class Servant
{
    nonmutating int getValue();
    ["amd"] nonmutating int slowGetValue();

    void setValue(int value);

    ["ami", "amd"] void setValueAsync(int value);
    nonmutating void releaseAsync();

    nonmutating void addFacet(string name, string data) throws AlreadyRegisteredException;
    nonmutating void removeFacet(string name) throws NotRegisteredException;

    nonmutating int getTransientValue();
    void setTransientValue(int value);
    void keepInCache();
    void release() throws NotRegisteredException;

    void destroy();

    int value;
};

class Facet extends Servant
{
    nonmutating string getData();
    void setData(string data);

    string data;
};

interface RemoteEvictor
{
    void setSize(int size);

    Servant* createServant(int id, int value) 
	throws AlreadyRegisteredException, EvictorDeactivatedException;

    Servant* getServant(int id);

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
