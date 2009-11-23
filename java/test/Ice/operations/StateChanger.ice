// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef STATECHANGER_ICE
#define STATECHANGER_ICE

[["java:package:test.Ice.operations"]]
module Test
{

interface StateChanger
{
    void hold(int milliSeconds);
    void activate(int milliSeconds);
};

};

#endif
