// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

class Servant
{
    nonmutating int getValue();

    void setValue(int value);

    void destroy();

    int value;
};

interface RemoteEvictor
{
    void setSize(int size);
    Servant* createServant(int value);
    nonmutating int getLastSavedValue();
    void clearLastSavedValue();
    void deactivate();
};

enum EvictorPersistenceMode
{
    SaveUponEviction,
    SaveAfterMutatingOperation
};

interface RemoteEvictorFactory
{
    RemoteEvictor* createEvictor(string name, EvictorPersistenceMode mode);
    void shutdown();
};

};

#endif
