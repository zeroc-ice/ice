//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
