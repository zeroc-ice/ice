// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef QUEUE_ICE
#define QUEUE_ICE

module Demo
{

interface Queue
{
    ["ami", "amd"] string get();
    void add(string message);
};

};

#endif
