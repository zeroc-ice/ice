// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

#include <Ice/Identity.ice>

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


exception InsufficientFundsException
{
};

class Account
{
    int getBalance();
    ["freeze:write"] void deposit(int amount) throws InsufficientFundsException;
    ["freeze:write"] void transfer(int amount, Account* toAccount) throws InsufficientFundsException;

    int balance;
};

sequence<Account*> AccountPrxSeq;
sequence<Ice::Identity> AccountIdSeq;

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

    ["freeze:write"] AccountPrxSeq getAccounts();
    ["freeze:read"] int getTotalBalance();

    void destroy();

    int value;
    AccountIdSeq accounts;
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
    RemoteEvictor* createEvictor(string name, bool transactional);
    void shutdown();
};

};

#endif
