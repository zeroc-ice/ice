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
