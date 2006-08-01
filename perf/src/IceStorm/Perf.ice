// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PERF_ICE
#define PERF_ICE

#include <Ice/BuiltinSequences.ice>

module Perf
{

enum AEnum
{
    A, 
    B,
    C
};

struct AStruct
{
    AEnum e;
    string s;
    double d;
};

sequence<AStruct> AStructSeq;

interface Intf
{
};

interface Ping
{
    void tickVoid(long time);

    void tick(long time, AEnum e, int i, AStruct s);
};

};

#endif
