// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

#include <Ice/Properties.ice>

[["java:package:test.IceBox.admin"]]
module Test
{

interface TestFacet
{
    Ice::PropertyDict getChanges();
};

};

#endif
