// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef QUEUE_ICE
#define QUEUE_ICE

#include <Ice/BuiltinSequences.ice>

module Demo
{

exception RequestCanceledException
{
};

interface Queue
{
    ["ami", "amd"] string get(string id)
        throws RequestCanceledException;

    void add(string message);

    ["amd"] void cancel(Ice::StringSeq ids);
};

};

#endif
