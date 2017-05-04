// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Test
{

exception BadEncodingException {}

interface MyObject
{
    ["cpp:type:wstring"] string widen(string msg) throws BadEncodingException;
    string narrow(["cpp:type:wstring"] string wmsg);

    void shutdown();
}

}

