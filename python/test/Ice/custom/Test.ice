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
    sequence<byte> ByteString; /* By default, sequence<byte> is received as a string. */
    ["python:seq:list"] sequence<byte> ByteList;

    sequence<string> StringList; /* By default, a sequence is received as a list. */
    ["python:seq:tuple"] sequence<string> StringTuple;

    struct S
    {
        ByteString b1;
        ["python:seq:list"] ByteString b2;
        ["python:seq:default"] ByteList b3;
        ByteList b4;
        StringList s1;
        ["python:seq:tuple"] StringList s2;
        StringTuple s3;
        ["python:seq:default"] StringTuple s4;
    }

    class C
    {
        ByteString b1;
        ["python:seq:list"] ByteString b2;
        ["python:seq:default"] ByteList b3;
        ByteList b4;
        StringList s1;
        ["python:seq:tuple"] StringList s2;
        StringTuple s3;
        ["python:seq:default"] StringTuple s4;
    }

    interface Custom
    {
        ByteString opByteString1(ByteString b1, out ByteString b2);
        ["python:seq:tuple"] ByteString opByteString2(["python:seq:list"] ByteString b1,
                                                      out ["python:seq:list"] ByteString b2);

        ByteList opByteList1(ByteList b1, out ByteList b2);
        ["python:seq:default"] ByteList opByteList2(["python:seq:tuple"] ByteList b1,
                                                    out ["python:seq:tuple"] ByteList b2);

        StringList opStringList1(StringList s1, out StringList s2);
        ["python:seq:tuple"] StringList opStringList2(["python:seq:tuple"] StringList s1,
                                                      out ["python:seq:tuple"] StringList s2);

        StringTuple opStringTuple1(StringTuple s1, out StringTuple s2);
        ["python:seq:list"] StringTuple opStringTuple2(["python:seq:list"] StringTuple s1,
                                                        out ["python:seq:default"] StringTuple s2);

        void sendS(S val);
        void sendC(C val);

        void shutdown();
    }
}
