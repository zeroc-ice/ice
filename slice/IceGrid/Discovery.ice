// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <IceGrid/Locator.ice>

module IceGrid
{

interface LookupReply
{
    void foundLocator(Locator* prx);
};

interface Lookup
{
    idempotent void findLocator(string instanceName, LookupReply* reply);

    Locator* getLocator();
};

};
