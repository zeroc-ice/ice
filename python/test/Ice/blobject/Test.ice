// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Test
{

exception UE
{
}

interface Hello
{
    void sayHello(int delay);
    int add(int s1, int s2);
    void raiseUE()
        throws UE;
    void shutdown();
}

}
