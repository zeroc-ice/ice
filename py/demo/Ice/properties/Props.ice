// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DEMO_ICE
#define DEMO_ICE

#include <Ice/Properties.ice>

module Demo
{

interface Props
{
    idempotent Ice::PropertyDict getChanges();
    void shutdown();
};

};

#endif
